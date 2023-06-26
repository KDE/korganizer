/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 1998 Preston Brown <pbrown@kde.org>
  SPDX-FileCopyrightText: 2000, 2001 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "searchdialog.h"

#include "calendarview.h"
#include "koeventpopupmenu.h"
#include "korganizer_debug.h"
#include "ui_searchdialog_base.h"

#include <EventViews/ListView>
#include <PimCommon/PimUtil>

#include <KConfigGroup>
#include <KMessageBox>
#include <KSharedConfig>
#include <KWindowConfig>
#include <QWindow>

#include <QDialogButtonBox>
#include <QPushButton>

namespace
{
static const char mySearchDialogConfigGroupName[] = "SearchDialog";
}

SearchDialog::SearchDialog(CalendarView *calendarview)
    : QDialog(calendarview)
    , m_ui(new Ui::SearchDialog)
    , m_calendarview(calendarview)
{
    setWindowTitle(i18nc("@title:window", "Find in Calendars"));
    setModal(false);

    auto mainLayout = new QVBoxLayout(this);

    auto mainWidget = new QWidget(this);
    m_ui->setupUi(mainWidget);

    // Set nice initial start and end dates for the search
    const QDate currDate = QDate::currentDate();
    m_ui->startDate->setDate(currDate);
    m_ui->endDate->setDate(currDate.addYears(1));

    connect(m_ui->searchEdit, &QLineEdit::textChanged, this, &SearchDialog::searchPatternChanged);

    // Results list view
    auto layout = new QVBoxLayout;
    layout->setContentsMargins({});
    m_listView = new EventViews::ListView(this);
    layout->addWidget(m_listView);
    m_ui->listViewFrame->setLayout(layout);

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Close | QDialogButtonBox::Help, this);
    mainLayout->addWidget(mainWidget);
    m_user1Button = new QPushButton;
    buttonBox->addButton(m_user1Button, QDialogButtonBox::ActionRole);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &SearchDialog::reject);
    connect(buttonBox, &QDialogButtonBox::helpRequested, this, &SearchDialog::slotHelpRequested);
    mainLayout->addWidget(buttonBox);
    m_user1Button->setDefault(true);
    KGuiItem::assign(m_user1Button, KGuiItem(i18nc("@action:button search in calendar", "&Search")));
    m_user1Button->setIcon(QIcon::fromTheme(QStringLiteral("search")));
    m_user1Button->setToolTip(i18nc("@info:tooltip", "Start the search"));
    m_user1Button->setWhatsThis(i18nc("@info:whatsthis", "Press this button to start the search."));

    connect(m_user1Button, &QPushButton::clicked, this, &SearchDialog::doSearch);

    // Propagate edit and delete event signals from event list view
    connect(m_listView, &EventViews::ListView::showIncidenceSignal, this, &SearchDialog::showIncidenceSignal);
    connect(m_listView, &EventViews::ListView::editIncidenceSignal, this, &SearchDialog::editIncidenceSignal);
    connect(m_listView, &EventViews::ListView::deleteIncidenceSignal, this, &SearchDialog::deleteIncidenceSignal);

    m_popupMenu = new KOEventPopupMenu(KOEventPopupMenu::MiniList, this);
    connect(m_listView, &EventViews::ListView::showIncidencePopupSignal, m_popupMenu, &KOEventPopupMenu::showIncidencePopup);

    connect(m_popupMenu, &KOEventPopupMenu::showIncidenceSignal, this, &SearchDialog::showIncidenceSignal);
    connect(m_popupMenu, &KOEventPopupMenu::editIncidenceSignal, this, &SearchDialog::editIncidenceSignal);
    connect(m_popupMenu, &KOEventPopupMenu::deleteIncidenceSignal, this, &SearchDialog::deleteIncidenceSignal);
    // TODO: add these
    //   connect(m_popupMenu, &KOEventPopupMenu::toggleAlarmSignal, this,
    //           &SearchDialog::toggleAlarmSignal);
    //   connect(m_popupMenu, &KOEventPopupMenu::toggleTodoCompletedSignal, this,
    //           &SearchDialog::toggleTodoCompletedSignal);

    for (const auto &calendar : m_calendarview->calendars()) {
        m_listView->addCalendar(calendar);
    }

    readConfig();
}

SearchDialog::~SearchDialog()
{
    writeConfig();
    delete m_popupMenu;
    delete m_ui;
}

void SearchDialog::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    activateWindow();
    m_ui->searchEdit->setFocus();
}

void SearchDialog::searchPatternChanged(const QString &pattern)
{
    m_user1Button->setEnabled(!pattern.isEmpty());
}

void SearchDialog::doSearch()
{
    QRegularExpression re;
    re.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
    re.setPattern(QRegularExpression::wildcardToRegularExpression(m_ui->searchEdit->text()));
    if (!re.isValid()) {
        KMessageBox::error(this,
                           i18nc("@info",
                                 "Invalid search expression, cannot perform the search. "
                                 "Please enter a search expression using the wildcard characters "
                                 "'*' and '?' where needed."));
        return;
    }

    search(re);
    m_listView->showIncidences(m_matchedEvents, QDate());
    updateMatchesText();
    if (m_matchedEvents.isEmpty()) {
        m_ui->numItems->setText(QString());
        KMessageBox::information(this,
                                 i18nc("@info", "No items were found that match your search pattern."),
                                 i18nc("@title:window", "Search Results"),
                                 QStringLiteral("NoSearchResults"));
    }
}

void SearchDialog::popupMenu(const QPoint &point)
{
    m_listView->popupMenu(point);
}

void SearchDialog::updateMatchesText()
{
    if (m_matchedEvents.isEmpty()) {
        m_ui->numItems->setText(QString());
    } else {
        m_ui->numItems->setText(i18ncp("@label", "%1 match", "%1 matches", m_matchedEvents.count()));
    }
}

void SearchDialog::updateView()
{
    QRegularExpression re;
    re.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
    re.setPattern(m_ui->searchEdit->text());
    m_listView->clear();
    if (re.isValid()) {
        search(re);
    } else {
        m_matchedEvents.clear();
    }
    m_listView->showIncidences(m_matchedEvents, QDate());
    updateMatchesText();
}

void SearchDialog::search(const QRegularExpression &regularExpression)
{
    const QDate startDt = m_ui->startDate->date();
    const QDate endDt = m_ui->endDate->date();

    KCalendarCore::Event::List events;
    if (m_ui->eventsCheck->isChecked()) {
        for (const auto &calendar : m_calendarview->calendars()) {
            events += calendar->events(startDt, endDt, QTimeZone::systemTimeZone(), m_ui->inclusiveCheck->isChecked());
        }
    }

    KCalendarCore::Todo::List todos;

    if (m_ui->todosCheck->isChecked()) {
        if (m_ui->includeUndatedTodos->isChecked()) {
            for (const auto &calendar : m_calendarview->calendars()) {
                for (const KCalendarCore::Todo::Ptr &todo : calendar->todos()) {
                    Q_ASSERT(todo);
                    if ((!todo->hasStartDate() && !todo->hasDueDate()) // undated
                        || (todo->hasStartDate() && (todo->dtStart().toLocalTime().date() >= startDt)
                            && (todo->dtStart().toLocalTime().date() <= endDt)) // start dt in range
                        || (todo->hasDueDate() && (todo->dtDue().toLocalTime().date() >= startDt)
                            && (todo->dtDue().toLocalTime().date() <= endDt)) // due dt in range
                        || (todo->hasCompletedDate() && (todo->completed().toLocalTime().date() >= startDt)
                            && (todo->completed().toLocalTime().date() <= endDt))) { // completed dt in range
                        todos.append(todo);
                    }
                }
            }
        } else {
            QDate dt = startDt;
            while (dt <= endDt) {
                for (const auto &calendar : m_calendarview->calendars()) {
                    todos += calendar->todos(dt);
                }
                dt = dt.addDays(1);
            }
        }
    }

    KCalendarCore::Journal::List journals;
    if (m_ui->journalsCheck->isChecked()) {
        QDate dt = startDt;
        while (dt <= endDt) {
            for (const auto &calendar : m_calendarview->calendars()) {
                journals += calendar->journals(dt);
            }
            dt = dt.addDays(1);
        }
    }

    m_matchedEvents.clear();
    const KCalendarCore::Incidence::List incidences = Akonadi::ETMCalendar::mergeIncidenceList(events, todos, journals);
    for (const KCalendarCore::Incidence::Ptr &ev : incidences) {
        Q_ASSERT(ev);
        auto collectionId = ev->customProperty("VOLATILE", "COLLECTION-ID").toLongLong();
        Akonadi::Item item;
        for (const auto &calendar : m_calendarview->calendars()) {
            if (calendar->collection().id() == collectionId) {
                item = calendar->item(ev);
            }
        }
        if (!item.isValid()) {
            qCWarning(KORGANIZER_LOG) << "Failed to translate incidence " << ev->uid() << " to Akonadi Item";
            continue;
        }

        if (m_ui->summaryCheck->isChecked()) {
            if (regularExpression.match(ev->summary()).hasMatch()) {
                m_matchedEvents.append(item);
                continue;
            }
        }
        if (m_ui->descriptionCheck->isChecked()) {
            if (regularExpression.match(ev->description()).hasMatch()) {
                m_matchedEvents.append(item);
                continue;
            }
        }
        if (m_ui->categoryCheck->isChecked()) {
            if (regularExpression.match(ev->categoriesStr()).hasMatch()) {
                m_matchedEvents.append(item);
                continue;
            }
        }
        if (m_ui->locationCheck->isChecked()) {
            if (regularExpression.match(ev->location()).hasMatch()) {
                m_matchedEvents.append(item);
                continue;
            }
        }
        if (m_ui->attendeeCheck->isChecked()) {
            const KCalendarCore::Attendee::List lstAttendees = ev->attendees();
            for (const KCalendarCore::Attendee &attendee : lstAttendees) {
                if (regularExpression.match(attendee.fullName()).hasMatch()) {
                    m_matchedEvents.append(item);
                    break;
                }
            }
        }
    }
}

void SearchDialog::readConfig()
{
    create(); // ensure a window is created
    windowHandle()->resize(QSize(775, 600));
    KConfigGroup group(KSharedConfig::openStateConfig(), mySearchDialogConfigGroupName);
    KWindowConfig::restoreWindowSize(windowHandle(), group);
    resize(windowHandle()->size()); // workaround for QTBUG-40584

    m_listView->readSettings(group);
}

void SearchDialog::writeConfig()
{
    KConfigGroup group(KSharedConfig::openStateConfig(), mySearchDialogConfigGroupName);
    KWindowConfig::saveWindowSize(windowHandle(), group);
    m_listView->writeSettings(group);
    group.sync();
}

void SearchDialog::slotHelpRequested()
{
    PimCommon::Util::invokeHelp(QStringLiteral("korganizer/search-view.html"));
}
