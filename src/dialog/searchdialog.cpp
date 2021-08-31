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
#include "ui_searchdialog_base.h"

#include <EventViews/ListView>
#include <PimCommon/PimUtil>

#include <KMessageBox>
#include <KSharedConfig>

#include <QDialogButtonBox>
#include <QPushButton>
#include <QShortcut>

SearchDialog::SearchDialog(CalendarView *calendarview)
    : QDialog(calendarview)
    , m_ui(new Ui::SearchDialog)
    , m_calendarview(calendarview)
{
    setWindowTitle(i18nc("@title:window", "Search Calendar"));
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
    m_listView = new EventViews::ListView(m_calendarview->calendar(), this);
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

    m_popupMenu = new KOEventPopupMenu(m_calendarview->calendar(), KOEventPopupMenu::MiniList, this);
    connect(m_listView, &EventViews::ListView::showIncidencePopupSignal, m_popupMenu, &KOEventPopupMenu::showIncidencePopup);

    connect(m_popupMenu, &KOEventPopupMenu::showIncidenceSignal, this, &SearchDialog::showIncidenceSignal);
    connect(m_popupMenu, &KOEventPopupMenu::editIncidenceSignal, this, &SearchDialog::editIncidenceSignal);
    connect(m_popupMenu, &KOEventPopupMenu::deleteIncidenceSignal, this, &SearchDialog::deleteIncidenceSignal);
    // TODO: add these
    //   connect(m_popupMenu, &KOEventPopupMenu::toggleAlarmSignal, this,
    //           &SearchDialog::toggleAlarmSignal);
    //   connect(m_popupMenu, &KOEventPopupMenu::toggleTodoCompletedSignal, this,
    //           &SearchDialog::toggleTodoCompletedSignal);

    const auto delKey = new QShortcut(QKeySequence(Qt::Key_Delete), this);
    connect(delKey, &QShortcut::activated, this, &SearchDialog::slotDeleteSelection);
    const auto retKey = new QShortcut(QKeySequence(Qt::Key_Return), this);
    connect(retKey, &QShortcut::activated, this, &SearchDialog::slotEditSelection);

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
    m_ui->searchEdit->setFocus();
}

void SearchDialog::searchPatternChanged(const QString &pattern)
{
    m_user1Button->setEnabled(!pattern.isEmpty());
}

void SearchDialog::doSearch()
{
    QRegExp re;
    re.setPatternSyntax(QRegExp::Wildcard); // most people understand these better.
    re.setCaseSensitivity(Qt::CaseInsensitive);
    re.setPattern(m_ui->searchEdit->text());
    if (!re.isValid()) {
        KMessageBox::sorry(this,
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
    QRegExp re;
    re.setPatternSyntax(QRegExp::Wildcard); // most people understand these better.
    re.setCaseSensitivity(Qt::CaseInsensitive);
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

void SearchDialog::search(const QRegExp &re)
{
    const QDate startDt = m_ui->startDate->date();
    const QDate endDt = m_ui->endDate->date();

    KCalendarCore::Event::List events;
    if (m_ui->eventsCheck->isChecked()) {
        events = m_calendarview->calendar()->events(startDt, endDt, QTimeZone::systemTimeZone(), m_ui->inclusiveCheck->isChecked());
    }

    KCalendarCore::Todo::List todos;

    if (m_ui->todosCheck->isChecked()) {
        if (m_ui->includeUndatedTodos->isChecked()) {
            const KCalendarCore::Todo::List alltodos = m_calendarview->calendar()->todos();
            for (const KCalendarCore::Todo::Ptr &todo : alltodos) {
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
        } else {
            QDate dt = startDt;
            while (dt <= endDt) {
                todos += m_calendarview->calendar()->todos(dt);
                dt = dt.addDays(1);
            }
        }
    }

    KCalendarCore::Journal::List journals;
    if (m_ui->journalsCheck->isChecked()) {
        QDate dt = startDt;
        while (dt <= endDt) {
            journals += m_calendarview->calendar()->journals(dt);
            dt = dt.addDays(1);
        }
    }

    m_matchedEvents.clear();
    const KCalendarCore::Incidence::List incidences = Akonadi::ETMCalendar::mergeIncidenceList(events, todos, journals);
    for (const KCalendarCore::Incidence::Ptr &ev : incidences) {
        Q_ASSERT(ev);
        Akonadi::Item item = m_calendarview->calendar()->item(ev->uid());
        if (m_ui->summaryCheck->isChecked()) {
            if (re.indexIn(ev->summary()) != -1) {
                m_matchedEvents.append(item);
                continue;
            }
        }
        if (m_ui->descriptionCheck->isChecked()) {
            if (re.indexIn(ev->description()) != -1) {
                m_matchedEvents.append(item);
                continue;
            }
        }
        if (m_ui->categoryCheck->isChecked()) {
            if (re.indexIn(ev->categoriesStr()) != -1) {
                m_matchedEvents.append(item);
                continue;
            }
        }
        if (m_ui->locationCheck->isChecked()) {
            if (re.indexIn(ev->location()) != -1) {
                m_matchedEvents.append(item);
                continue;
            }
        }
        if (m_ui->attendeeCheck->isChecked()) {
            const KCalendarCore::Attendee::List lstAttendees = ev->attendees();
            for (const KCalendarCore::Attendee &attendee : lstAttendees) {
                if (re.indexIn(attendee.fullName()) != -1) {
                    m_matchedEvents.append(item);
                    break;
                }
            }
        }
    }
}

void SearchDialog::readConfig()
{
    KConfigGroup group = KSharedConfig::openConfig()->group(QStringLiteral("SearchDialog"));
    const QSize size = group.readEntry("Size", QSize(775, 600));
    if (size.isValid()) {
        resize(size);
    }
    m_listView->readSettings(group);
}

void SearchDialog::writeConfig()
{
    KConfigGroup group = KSharedConfig::openConfig()->group(QStringLiteral("SearchDialog"));
    group.writeEntry("Size", size());
    m_listView->writeSettings(group);
    group.sync();
}

void SearchDialog::slotDeleteSelection()
{
    const auto selected = m_listView->selectedIncidences();
    if (selected.count() > 0) {
        Q_EMIT deleteIncidenceSignal(selected.at(0));
    }
}

void SearchDialog::slotEditSelection()
{
    const auto selected = m_listView->selectedIncidences();
    if (selected.count() > 0) {
        Q_EMIT editIncidenceSignal(selected.at(0));
    }
}

void SearchDialog::slotHelpRequested()
{
    PimCommon::Util::invokeHelp(QStringLiteral("korganizer/search-view.html"));
}
