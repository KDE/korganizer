/*
  This file is part of Kontact.

  Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>
  Copyright (c) 2005-2006,2008-2009 Allen Winter <winter@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "todosummarywidget.h"
#include "todoplugin.h"
#include "korganizerinterface.h"
#include <CalendarSupport/Utils>
#include <CalendarSupport/CalendarSingleton>

#include <AkonadiCore/Collection>
#include <AkonadiCore/ItemFetchScope>
#include <Akonadi/Calendar/IncidenceChanger>

#include <KCalUtils/IncidenceFormatter>

#include <KontactInterface/Core>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <QMenu>
#include <KUrlLabel>

#include <QGridLayout>
#include <QLabel>
#include <QTextDocument>  // for Qt::mightBeRichText
#include <QVBoxLayout>
#include <QStyle>

using namespace KCalUtils;

TodoSummaryWidget::TodoSummaryWidget(TodoPlugin *plugin, QWidget *parent)
    : KontactInterface::Summary(parent)
    , mPlugin(plugin)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(3);
    mainLayout->setContentsMargins(3, 3, 3, 3);

    QWidget *header = createHeader(this, QStringLiteral("korg-todo"), i18n("Pending To-dos"));
    mainLayout->addWidget(header);

    mLayout = new QGridLayout();
    mainLayout->addItem(mLayout);
    mLayout->setSpacing(3);
    mLayout->setRowStretch(6, 1);
    mCalendar = CalendarSupport::calendarSingleton();

    mChanger = new Akonadi::IncidenceChanger(parent);

    connect(
        mCalendar.data(), &Akonadi::ETMCalendar::calendarChanged, this,
        &TodoSummaryWidget::updateView);
    connect(
        mPlugin->core(), &KontactInterface::Core::dayChanged, this, &TodoSummaryWidget::updateView);

    updateView();
}

TodoSummaryWidget::~TodoSummaryWidget()
{
}

void TodoSummaryWidget::updateView()
{
    qDeleteAll(mLabels);
    mLabels.clear();

    KConfig config(QStringLiteral("kcmtodosummaryrc"));
    KConfigGroup group = config.group("Days");
    int mDaysToGo = group.readEntry("DaysToShow", 7);

    group = config.group("Hide");
    mHideInProgress = group.readEntry("InProgress", false);
    mHideOverdue = group.readEntry("Overdue", false);
    mHideCompleted = group.readEntry("Completed", true);
    mHideOpenEnded = group.readEntry("OpenEnded", true);
    mHideNotStarted = group.readEntry("NotStarted", false);

    group = config.group("Groupware");
    mShowMineOnly = group.readEntry("ShowMineOnly", false);

    // for each todo,
    //   if it passes the filter, append to a list
    //   else continue
    // sort todolist by summary
    // sort todolist by priority
    // sort todolist by due-date
    // print todolist

    // the filter is created by the configuration summary options, but includes
    //    days to go before to-do is due
    //    which types of to-dos to hide

    KCalendarCore::Todo::List prList;

    const QDate currDate = QDate::currentDate();
    const KCalendarCore::Todo::List todos = mCalendar->todos();
    for (const KCalendarCore::Todo::Ptr &todo : todos) {
        if (todo->hasDueDate()) {
            const int daysTo = currDate.daysTo(todo->dtDue().date());
            if (daysTo >= mDaysToGo) {
                continue;
            }
        }

        if (mHideOverdue && todo->isOverdue()) {
            continue;
        }
        if (mHideInProgress && todo->isInProgress(false)) {
            continue;
        }
        if (mHideCompleted && todo->isCompleted()) {
            continue;
        }
        if (mHideOpenEnded && todo->isOpenEnded()) {
            continue;
        }
        if (mHideNotStarted && todo->isNotStarted(false)) {
            continue;
        }

        prList.append(todo);
    }
    if (!prList.isEmpty()) {
        prList = Akonadi::ETMCalendar::sortTodos(prList,
                                                 KCalendarCore::TodoSortSummary,
                                                 KCalendarCore::SortDirectionAscending);
        prList = Akonadi::ETMCalendar::sortTodos(prList,
                                                 KCalendarCore::TodoSortPriority,
                                                 KCalendarCore::SortDirectionAscending);
        prList = Akonadi::ETMCalendar::sortTodos(prList,
                                                 KCalendarCore::TodoSortDueDate,
                                                 KCalendarCore::SortDirectionAscending);
    }

    // The to-do print consists of the following fields:
    //  icon:due date:days-to-go:priority:summary:status
    // where,
    //   the icon is the typical to-do icon
    //   the due date it the to-do due date
    //   the days-to-go/past is the #days until/since the to-do is due
    //     this field is left blank if the to-do is open-ended
    //   the priority is the to-do priority
    //   the summary is the to-do summary
    //   the status is comma-separated list of:
    //     overdue
    //     in-progress (started, or >0% completed)
    //     complete (100% completed)
    //     open-ended
    //     not-started (no start date and 0% completed)

    int counter = 0;
    QLabel *label = nullptr;

    if (!prList.isEmpty()) {
        QPixmap pm = QIcon::fromTheme(QStringLiteral("view-calendar-tasks")).pixmap(style()->pixelMetric(QStyle::PM_SmallIconSize));

        QString str;

        for (const KCalendarCore::Todo::Ptr &todo : qAsConst(prList)) {
            bool makeBold = false;
            int daysTo = -1;

            // Optionally, show only my To-dos
            /*      if ( mShowMineOnly &&
                         !KCalendarCore::CalHelper::isMyCalendarIncidence( mCalendarAdaptor, todo.get() ) ) {
                    continue;
                  }
            TODO: calhelper is deprecated, remove this?
            */

            // Icon label
            label = new QLabel(this);
            label->setPixmap(pm);
            label->setMaximumWidth(label->minimumSizeHint().width());
            mLayout->addWidget(label, counter, 0);
            mLabels.append(label);

            // Due date label
            str.clear();
            if (todo->hasDueDate() && todo->dtDue().date().isValid()) {
                daysTo = currDate.daysTo(todo->dtDue().date());

                if (daysTo == 0) {
                    makeBold = true;
                    str = i18nc("the to-do is due today", "Today");
                } else if (daysTo == 1) {
                    str = i18nc("the to-do is due tomorrow", "Tomorrow");
                } else {
                    const auto locale = QLocale::system();
                    for (int i = 3; i < 8; ++i) {
                        if (daysTo < i * 24 * 60 * 60) {
                            str = i18nc("1. weekday, 2. time", "%1 %2",
                                        locale.dayName(todo->dtDue().date().dayOfWeek(),
                                                       QLocale::LongFormat),
                                        locale.toString(todo->dtDue().time(),
                                                        QLocale::ShortFormat));
                            break;
                        }
                    }
                    if (str.isEmpty()) {
                        str = locale.toString(todo->dtDue(), QLocale::ShortFormat);
                    }
                }
            }

            label = new QLabel(str, this);
            label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            mLayout->addWidget(label, counter, 1);
            mLabels.append(label);
            if (makeBold) {
                QFont font = label->font();
                font.setBold(true);
                label->setFont(font);
            }

            // Days togo/ago label
            str.clear();
            if (todo->hasDueDate() && todo->dtDue().date().isValid()) {
                if (daysTo > 0) {
                    str = i18np("in 1 day", "in %1 days", daysTo);
                } else if (daysTo < 0) {
                    str = i18np("1 day ago", "%1 days ago", -daysTo);
                } else {
                    str = i18nc("the to-do is due", "due");
                }
            }
            label = new QLabel(str, this);
            label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            mLayout->addWidget(label, counter, 2);
            mLabels.append(label);

            // Priority label
            str = QLatin1Char('[') + QString::number(todo->priority()) + QLatin1Char(']');
            label = new QLabel(str, this);
            label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            mLayout->addWidget(label, counter, 3);
            mLabels.append(label);

            // Summary label
            str = todo->summary();
            if (!todo->relatedTo().isEmpty()) {   // show parent only, not entire ancestry
                KCalendarCore::Incidence::Ptr inc = mCalendar->incidence(todo->relatedTo());
                if (inc) {
                    str = inc->summary() + QLatin1Char(':') + str;
                }
            }
            if (!Qt::mightBeRichText(str)) {
                str = str.toHtmlEscaped();
            }

            KUrlLabel *urlLabel = new KUrlLabel(this);
            urlLabel->setText(str);
            urlLabel->setUrl(todo->uid());
            urlLabel->installEventFilter(this);
            urlLabel->setTextFormat(Qt::RichText);
            urlLabel->setWordWrap(true);
            mLayout->addWidget(urlLabel, counter, 4);
            mLabels.append(urlLabel);
            connect(urlLabel, &KUrlLabel::leftClickedUrl, this, [this, urlLabel] {
                viewTodo(urlLabel->url());
            });
            connect(urlLabel, &KUrlLabel::rightClickedUrl, this, [this, urlLabel] {
                popupMenu(urlLabel->url());
            });
            // State text label
            str = stateStr(todo);
            label = new QLabel(str, this);
            label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            mLayout->addWidget(label, counter, 5);
            mLabels.append(label);

            counter++;
        }
    } //foreach

    if (counter == 0) {
        QLabel *noTodos = new QLabel(
            i18np("No pending to-dos due within the next day",
                  "No pending to-dos due within the next %1 days",
                  mDaysToGo), this);
        noTodos->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        mLayout->addWidget(noTodos, 0, 0);
        mLabels.append(noTodos);
    }

    for (QLabel *label : qAsConst(mLabels)) {
        label->show();
    }
}

void TodoSummaryWidget::viewTodo(const QString &uid)
{
    const Akonadi::Item::Id id = mCalendar->item(uid).id();

    if (id != -1) {
        mPlugin->core()->selectPlugin(QStringLiteral("kontact_todoplugin"));  //ensure loaded
        OrgKdeKorganizerKorganizerInterface korganizer(
            QStringLiteral("org.kde.korganizer"), QStringLiteral(
                "/Korganizer"), QDBusConnection::sessionBus());

        korganizer.editIncidence(QString::number(id));
    }
}

void TodoSummaryWidget::removeTodo(const Akonadi::Item &item)
{
    mChanger->deleteIncidence(item);
}

void TodoSummaryWidget::completeTodo(Akonadi::Item::Id id)
{
    Akonadi::Item todoItem = mCalendar->item(id);

    if (todoItem.isValid()) {
        KCalendarCore::Todo::Ptr todo = CalendarSupport::todo(todoItem);
        if (!todo->isReadOnly()) {
            KCalendarCore::Todo::Ptr oldTodo(todo->clone());
            todo->setCompleted(QDateTime::currentDateTime());
            mChanger->modifyIncidence(todoItem, oldTodo);
            updateView();
        }
    }
}

void TodoSummaryWidget::popupMenu(const QString &uid)
{
    KCalendarCore::Todo::Ptr todo = mCalendar->todo(uid);
    if (!todo) {
        return;
    }
    Akonadi::Item item = mCalendar->item(uid);
    QMenu popup(this);
    QAction *editIt = popup.addAction(i18n("&Edit To-do..."));
    QAction *delIt = popup.addAction(i18n("&Delete To-do"));
    delIt->setIcon(QIcon::fromTheme(QStringLiteral("edit-delete")));

    QAction *doneIt = nullptr;
    delIt->setEnabled(mCalendar->hasRight(item, Akonadi::Collection::CanDeleteItem));

    if (!todo->isCompleted()) {
        doneIt = popup.addAction(i18n("&Mark To-do Completed"));
        doneIt->setIcon(QIcon::fromTheme(QStringLiteral("task-complete")));
        doneIt->setEnabled(mCalendar->hasRight(item, Akonadi::Collection::CanChangeItem));
    }
    // TODO: add icons to the menu actions

    const QAction *selectedAction = popup.exec(QCursor::pos());
    if (selectedAction == editIt) {
        viewTodo(uid);
    } else if (selectedAction == delIt) {
        removeTodo(item);
    } else if (doneIt && selectedAction == doneIt) {
        completeTodo(item.id());
    }
}

bool TodoSummaryWidget::eventFilter(QObject *obj, QEvent *e)
{
    if (obj->inherits("KUrlLabel")) {
        KUrlLabel *label = static_cast<KUrlLabel *>(obj);
        if (e->type() == QEvent::Enter) {
            Q_EMIT message(i18n("Edit To-do: \"%1\"", label->text()));
        }
        if (e->type() == QEvent::Leave) {
            Q_EMIT message(QString());
        }
    }
    return KontactInterface::Summary::eventFilter(obj, e);
}

QStringList TodoSummaryWidget::configModules() const
{
    return QStringList() << QStringLiteral("kcmtodosummary.desktop");
}

bool TodoSummaryWidget::startsToday(const KCalendarCore::Todo::Ptr &todo)
{
    return todo->hasStartDate()
           && todo->dtStart().date() == QDate::currentDate();
}

const QString TodoSummaryWidget::stateStr(const KCalendarCore::Todo::Ptr &todo)
{
    QString str1, str2;

    if (todo->isOpenEnded()) {
        str1 = i18n("open-ended");
    } else if (todo->isOverdue()) {
        str1 = QLatin1String("<font color=\"red\">")
               +i18nc("the to-do is overdue", "overdue")
               +QLatin1String("</font>");
    } else if (startsToday(todo)) {
        str1 = i18nc("the to-do starts today", "starts today");
    }

    if (todo->isNotStarted(false)) {
        str2 += i18nc("the to-do has not been started yet", "not-started");
    } else if (todo->isCompleted()) {
        str2 += i18nc("the to-do is completed", "completed");
    } else if (todo->isInProgress(false)) {
        str2 += i18nc("the to-do is in-progress", "in-progress ");
        str2 += QLatin1String(" (") + QString::number(todo->percentComplete())
                + QLatin1String("%)");
    }

    if (!str1.isEmpty() && !str2.isEmpty()) {
        str1 += i18nc("Separator for status like this: overdue, completed", ",");
    }

    return str1 + str2;
}
