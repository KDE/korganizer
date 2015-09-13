/*
  This file is part of KOrganizer.

  Copyright (c) 1998 Preston Brown <pbrown@kde.org>
  Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include "searchdialog.h"

#include "ui_searchdialog_base.h"
#include "calendarview.h"
#include "koglobals.h"

#include <CalendarSupport/KCalPrefs>
#include <CalendarSupport/Utils>

#include <eventviews/list/listview.h>

#include <KMessageBox>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>
#include <KGuiItem>
#include <QVBoxLayout>

using namespace KOrg;

SearchDialog::SearchDialog(CalendarView *calendarview)
    : QDialog(calendarview),
      m_ui(new Ui::SearchDialog),
      m_calendarview(calendarview)
{
    setWindowTitle(i18n("Search Calendar"));
    setModal(false);

    QWidget *mainWidget = new QWidget(this);
    m_ui->setupUi(mainWidget);

    // Set nice initial start and end dates for the search
    const QDate currDate = QDate::currentDate();
    m_ui->startDate->setDate(currDate);
    m_ui->endDate->setDate(currDate.addYears(1));

    connect(m_ui->searchEdit, &QLineEdit::textChanged, this, &SearchDialog::searchTextChanged);

    // Results list view
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    listView = new EventViews::ListView(m_calendarview->calendar(), this);
    layout->addWidget(listView);
    m_ui->listViewFrame->setLayout(layout);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);
    mUser1Button = new QPushButton;
    buttonBox->addButton(mUser1Button, QDialogButtonBox::ActionRole);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &SearchDialog::reject);
    mainLayout->addWidget(buttonBox);
    mUser1Button->setDefault(true);
    KGuiItem::assign(mUser1Button, KGuiItem(i18nc("search in calendar", "&Search")));
    mUser1Button->setToolTip(i18n("Start searching"));

    connect(mUser1Button, &QPushButton::clicked, this, &SearchDialog::doSearch);

    // Propagate edit and delete event signals from event list view
    connect(listView, &EventViews::ListView::showIncidenceSignal, this, &SearchDialog::showIncidenceSignal);
    connect(listView, &EventViews::ListView::editIncidenceSignal, this, &SearchDialog::editIncidenceSignal);
    connect(listView, &EventViews::ListView::deleteIncidenceSignal, this, &SearchDialog::deleteIncidenceSignal);

    readConfig();
}

SearchDialog::~SearchDialog()
{
    writeConfig();
    delete m_ui;
}

void SearchDialog::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    m_ui->searchEdit->setFocus();
}

void SearchDialog::searchTextChanged(const QString &_text)
{
    mUser1Button->setEnabled(!_text.isEmpty());
}

void SearchDialog::doSearch()
{
    QRegExp re;
    re.setPatternSyntax(QRegExp::Wildcard);   // most people understand these better.
    re.setCaseSensitivity(Qt::CaseInsensitive);
    re.setPattern(m_ui->searchEdit->text());
    if (!re.isValid()) {
        KMessageBox::sorry(
            this,
            i18n("Invalid search expression, cannot perform the search. "
                 "Please enter a search expression using the wildcard characters "
                 "'*' and '?' where needed."));
        return;
    }

    search(re);
    listView->showIncidences(mMatchedEvents, QDate());
    if (mMatchedEvents.isEmpty()) {
        m_ui->numItems->setText(QString());
        KMessageBox::information(
            this,
            i18n("No items were found that match your search pattern."),
            i18n("Search Results"),
            QStringLiteral("NoSearchResults"));
    } else {
        m_ui->numItems->setText(i18np("%1 item", "%1 items", mMatchedEvents.count()));
    }
}

void SearchDialog::updateView()
{
    QRegExp re;
    re.setPatternSyntax(QRegExp::Wildcard);   // most people understand these better.
    re.setCaseSensitivity(Qt::CaseInsensitive);
    re.setPattern(m_ui->searchEdit->text());
    if (re.isValid()) {
        search(re);
    } else {
        mMatchedEvents.clear();
    }
    listView->showIncidences(mMatchedEvents, QDate());
}

void SearchDialog::search(const QRegExp &re)
{
    const QDate startDt = m_ui->startDate->date();
    const QDate endDt = m_ui->endDate->date();

    KCalCore::Event::List events;
    KDateTime::Spec timeSpec = CalendarSupport::KCalPrefs::instance()->timeSpec();
    if (m_ui->eventsCheck->isChecked()) {
        events =
            m_calendarview->calendar()->events(
                startDt, endDt, timeSpec, m_ui->inclusiveCheck->isChecked());
    }

    KCalCore::Todo::List todos;

    if (m_ui->todosCheck->isChecked()) {
        if (m_ui->includeUndatedTodos->isChecked()) {
            KDateTime::Spec spec = CalendarSupport::KCalPrefs::instance()->timeSpec();
            KCalCore::Todo::List alltodos = m_calendarview->calendar()->todos();
            Q_FOREACH (const KCalCore::Todo::Ptr &todo, alltodos) {
                Q_ASSERT(todo);
                if ((!todo->hasStartDate() && !todo->hasDueDate()) ||    // undated
                        (todo->hasStartDate() &&
                         (todo->dtStart().toTimeSpec(spec).date() >= startDt) &&
                         (todo->dtStart().toTimeSpec(spec).date() <= endDt)) ||      //start dt in range
                        (todo->hasDueDate() &&
                         (todo->dtDue().toTimeSpec(spec).date() >= startDt) &&
                         (todo->dtDue().toTimeSpec(spec).date() <= endDt)) ||      //due dt in range
                        (todo->hasCompletedDate() &&
                         (todo->completed().toTimeSpec(spec).date() >= startDt) &&
                         (todo->completed().toTimeSpec(spec).date() <= endDt))) {      //completed dt in range
                    todos.append(todo);
                }
            }
        } else {
            QDate dt = startDt;
            while (dt <= endDt) {
                todos += m_calendarview->calendar()->todos(dt);
                dt = dt.addDays(1);
            }
        }
    }

    KCalCore::Journal::List journals;
    if (m_ui->journalsCheck->isChecked()) {
        QDate dt = startDt;
        while (dt <= endDt) {
            journals += m_calendarview->calendar()->journals(dt);
            dt = dt.addDays(1);
        }
    }

    mMatchedEvents.clear();
    KCalCore::Incidence::List incidences =
        Akonadi::ETMCalendar::mergeIncidenceList(events, todos, journals);
    Q_FOREACH (const KCalCore::Incidence::Ptr &ev, incidences) {
        Q_ASSERT(ev);
        Akonadi::Item item = m_calendarview->calendar()->item(ev->uid());
        if (m_ui->summaryCheck->isChecked()) {
            if (re.indexIn(ev->summary()) != -1) {
                mMatchedEvents.append(item);
                continue;
            }
        }
        if (m_ui->descriptionCheck->isChecked()) {
            if (re.indexIn(ev->description()) != -1) {
                mMatchedEvents.append(item);
                continue;
            }
        }
        if (m_ui->categoryCheck->isChecked()) {
            if (re.indexIn(ev->categoriesStr()) != -1) {
                mMatchedEvents.append(item);
                continue;
            }
        }
        if (m_ui->locationCheck->isChecked()) {
            if (re.indexIn(ev->location()) != -1) {
                mMatchedEvents.append(item);
                continue;
            }
        }
        if (m_ui->attendeeCheck->isChecked()) {
            Q_FOREACH (const KCalCore::Attendee::Ptr &attendee, ev->attendees()) {
                if (re.indexIn(attendee->fullName()) != -1) {
                    mMatchedEvents.append(item);
                    break;
                }
            }
        }
    }
}

void SearchDialog::readConfig()
{
    KConfigGroup group(KOGlobals::self()->config(), QStringLiteral("SearchDialog"));
    const QSize size = group.readEntry("Size", QSize(775, 600));
    if (size.isValid()) {
        resize(size);
    }
}

void SearchDialog::writeConfig()
{
    KConfigGroup group(KOGlobals::self()->config(), QStringLiteral("SearchDialog"));
    group.writeEntry("Size", size());
    group.sync();
}

