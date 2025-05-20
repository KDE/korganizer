/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2000, 2001 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "koeventpopupmenu.h"
#include "korganizer_debug.h"

#include <Akonadi/CalendarUtils>
#include <Akonadi/ItemCreateJob>

#include <CalendarSupport/CalPrinter>
#include <CalendarSupport/KCalPrefs>
#include <CalendarSupport/Utils>

#include <KCalendarCore/CalFormat>

#include <IncidenceEditor/IncidenceDialog>
#include <IncidenceEditor/IncidenceDialogFactory>

#include <QCoreApplication>

KOEventPopupMenu::KOEventPopupMenu(QWidget *parent)
    : QMenu(parent)
{
    init(MenuStyle::NormalView);
}

KOEventPopupMenu::KOEventPopupMenu(MenuStyle menuStyle, QWidget *parent)
    : QMenu(parent)
{
    init(menuStyle);
}

void KOEventPopupMenu::init(MenuStyle menuStyle)
{
    // These actions are always shown, no matter what
    addAction(QIcon::fromTheme(QStringLiteral("document-preview")), i18nc("@action:inmenu", "&Show"), this, &KOEventPopupMenu::popupShow);

    addAction(QIcon::fromTheme(QStringLiteral("document-edit")), i18nc("@action:inmenu", "&Edit…"), this, &KOEventPopupMenu::popupEdit);

    addAction(QIcon::fromTheme(QStringLiteral("edit-delete")), i18nc("@action:inmenu delete this incidence", "&Delete"), this, &KOEventPopupMenu::popupDelete);

    addSeparator();

    addAction(QIcon::fromTheme(QStringLiteral("document-print")), i18nc("@action:inmenu", "&Print…"), this, &KOEventPopupMenu::slotPrint);

    addAction(QIcon::fromTheme(QStringLiteral("document-print-preview")), i18nc("@action:inmenu", "Print Previe&w…"), this, &KOEventPopupMenu::printPreview);

    // Add more menu actions according to Menu style
    switch (menuStyle) {
    case MenuStyle::NormalView:
        appendEditOnlyItems();
        appendEventOnlyItems();
        appendTodoOnlyItems();
        appendReminderOnlyItems();
        appendRecurrenceOnlyItems();
        appendShareOnlyItems();
    default:
        break;
    }
}

void KOEventPopupMenu::appendEditOnlyItems()
{
    mEditOnlyItems.append(addSeparator());

    mEditOnlyItems.append(
        addAction(QIcon::fromTheme(QStringLiteral("edit-cut")), i18nc("@action:inmenu cut this incidence", "C&ut"), this, &KOEventPopupMenu::popupCut));
    mEditOnlyItems.append(
        addAction(QIcon::fromTheme(QStringLiteral("edit-copy")), i18nc("@action:inmenu copy this incidence", "&Copy"), this, &KOEventPopupMenu::popupCopy));
    mEditOnlyItems.append(addAction(QIcon::fromTheme(QStringLiteral("edit-paste")), i18nc("@action:inmenu", "&Paste"), this, &KOEventPopupMenu::popupPaste));
}

void KOEventPopupMenu::appendEventOnlyItems()
{
    mEventOnlyItems.append(addSeparator());

    mEventOnlyItems.append(addAction(QIcon::fromTheme(QStringLiteral("appointment-new")),
                                     i18nc("@action:inmenu", "New Event"),
                                     this,
                                     qOverload<>(&KOEventPopupMenu::createEvent)));

    mEventOnlyItems.append(
        addAction(QIcon::fromTheme(QStringLiteral("task-new")), i18nc("@action:inmenu", "New To-do"), this, qOverload<>(&KOEventPopupMenu::createNewTodo)));

    mEventOnlyItems.append(
        addAction(QIcon::fromTheme(QStringLiteral("task-new")), i18nc("@action:inmenu", "Create To-do from Event"), this, &KOEventPopupMenu::createTodo));
    mEventOnlyItems.last()->setObjectName(QLatin1StringView("createtodo")); // id used by unit test
}

void KOEventPopupMenu::appendTodoOnlyItems()
{
    mTodoOnlyItems.append(addSeparator());

    mTodoOnlyItems.append(
        addAction(QIcon::fromTheme(QStringLiteral("task-new")), i18nc("@action:inmenu", "New To-do"), this, qOverload<>(&KOEventPopupMenu::createTodo)));

    mTodoOnlyItems.append(addAction(QIcon::fromTheme(QStringLiteral("appointment-new")),
                                    i18nc("@action:inmenu", "New Event"),
                                    this,
                                    qOverload<>(&KOEventPopupMenu::createNewEvent)));

    mTodoOnlyItems.append(addAction(QIcon::fromTheme(QStringLiteral("appointment-new")),
                                    i18nc("@action:inmenu", "Create Event from To-do"),
                                    this,
                                    qOverload<>(&KOEventPopupMenu::createEvent)));
    mTodoOnlyItems.last()->setObjectName(QLatin1StringView("createevent")); // id used by unit test

    mTodoOnlyItems.append(addAction(QIcon::fromTheme(QStringLiteral("task-complete")),
                                    i18nc("@action:inmenu", "Togg&le To-do Completed"),
                                    this,
                                    &KOEventPopupMenu::toggleTodoCompleted));
}

void KOEventPopupMenu::appendReminderOnlyItems()
{
    mReminderOnlyItems.append(addSeparator());

    mToggleReminder =
        addAction(QIcon::fromTheme(QStringLiteral("appointment-reminder")), i18nc("@action:inmenu", "&Toggle Reminder"), this, &KOEventPopupMenu::toggleAlarm);
    mReminderOnlyItems.append(mToggleReminder);
}

void KOEventPopupMenu::appendRecurrenceOnlyItems()
{
    mRecurrenceOnlyItems.append(addSeparator());
    mDissociateOccurrences = addAction(i18nc("@action:inmenu", "&Dissociate From Recurrence…"), this, &KOEventPopupMenu::dissociateOccurrences);
    mRecurrenceOnlyItems.append(mDissociateOccurrences);
}

void KOEventPopupMenu::appendShareOnlyItems()
{
    mShareOnlyItems.append(addSeparator());
    mShareOnlyItems.append(
        addAction(QIcon::fromTheme(QStringLiteral("mail-forward")), i18nc("@action:inmenu", "Send as iCalendar…"), this, &KOEventPopupMenu::forward));
}

void KOEventPopupMenu::showIncidencePopup(const Akonadi::CollectionCalendar::Ptr &calendar, const Akonadi::Item &item, const QDate &qd)
{
    mCurrentCalendar = calendar;
    mCurrentIncidence = item;
    mCurrentDate = qd;

    if (!CalendarSupport::hasIncidence(mCurrentIncidence) /*&& qd.isValid()*/) {
        qCDebug(KORGANIZER_LOG) << "No event selected";
        return;
    }

    if (!mCurrentCalendar) {
        // TODO fix it
        qCDebug(KORGANIZER_LOG) << "Calendar is unset";
        return;
    }

    KCalendarCore::Incidence::Ptr const incidence = Akonadi::CalendarUtils::incidence(mCurrentIncidence);
    Q_ASSERT(incidence);

    // Determine if this Incidence's calendar is writeable.
    // Else all actions that might modify the Incidence are disabled.
    const bool hasChangeRights = mCurrentCalendar->hasRight(Akonadi::Collection::CanChangeItem);

    QList<QAction *>::Iterator it;
    QList<QAction *>::Iterator end;

    // Enable/Disabled menu edit items
    end = mEditOnlyItems.end();
    for (it = mEditOnlyItems.begin(); it != end; ++it) {
        (*it)->setEnabled(hasChangeRights);
    }

    // Enable/Disable menu items valid for Events only
    end = mEventOnlyItems.end();
    for (it = mEventOnlyItems.begin(); it != end; ++it) {
        (*it)->setVisible(incidence->type() == KCalendarCore::Incidence::TypeEvent);
        (*it)->setEnabled(true);
    }

    // Enable/Disable menu items valid for Todos only
    end = mTodoOnlyItems.end();
    for (it = mTodoOnlyItems.begin(); it != end; ++it) {
        (*it)->setVisible(incidence->type() == KCalendarCore::Incidence::TypeTodo);
        (*it)->setEnabled(true);
    }
    if (mToggleReminder) {
        mToggleReminder->setText(incidence->hasEnabledAlarms() ? i18nc("@action:inmenu", "&Toggle Reminder Off")
                                                               : i18nc("@action:inmenu", "&Toggle Reminder On"));
    }

    // Enable/Disable menu items valid for reminder Incidences only
    end = mReminderOnlyItems.end();
    for (it = mReminderOnlyItems.begin(); it != end; ++it) {
        (*it)->setVisible(incidence->type() != KCalendarCore::Incidence::TypeJournal);
        (*it)->setEnabled(hasChangeRights);
    }

    // Enable/Disable menu items valid for recurrent Incidences only
    end = mRecurrenceOnlyItems.end();
    for (it = mRecurrenceOnlyItems.begin(); it != end; ++it) {
        (*it)->setVisible(incidence->recurs());
        (*it)->setEnabled(hasChangeRights);
    }
    if (incidence->recurs()) {
        const QDateTime thisDateTime(qd, {}, QTimeZone::LocalTime);
        const bool isLastOccurrence = !incidence->recurrence()->getNextDateTime(thisDateTime).isValid();
        const bool isFirstOccurrence = !incidence->recurrence()->getPreviousDateTime(thisDateTime).isValid();
        if (mDissociateOccurrences) {
            mDissociateOccurrences->setEnabled(!(isFirstOccurrence && isLastOccurrence) && hasChangeRights);
        }
    }

    // Enable/Disable menu items valid for sharing Incidences only
    end = mShareOnlyItems.end();
    for (it = mShareOnlyItems.begin(); it != end; ++it) {
        (*it)->setVisible(true);
        (*it)->setEnabled(true);
    }

    // Show the menu now
    popup(QCursor::pos());
}

void KOEventPopupMenu::popupShow()
{
    if (CalendarSupport::hasIncidence(mCurrentIncidence)) {
        Q_EMIT showOccurrenceSignal(mCurrentIncidence, mCurrentDate);
    }
}

void KOEventPopupMenu::popupEdit()
{
    if (CalendarSupport::hasIncidence(mCurrentIncidence)) {
        Q_EMIT editIncidenceSignal(mCurrentIncidence);
    }
}

void KOEventPopupMenu::slotPrint()
{
    print(false);
}

void KOEventPopupMenu::print(bool preview)
{
    CalendarSupport::CalPrinter printer(this, mCurrentCalendar, true);
    connect(this, &KOEventPopupMenu::configChanged, &printer, &CalendarSupport::CalPrinter::updateConfig);

    KCalendarCore::Incidence::List selectedIncidences;
    Q_ASSERT(mCurrentIncidence.hasPayload<KCalendarCore::Incidence::Ptr>());
    selectedIncidences.append(mCurrentIncidence.payload<KCalendarCore::Incidence::Ptr>());

    printer.print(CalendarSupport::CalPrinterBase::Incidence, mCurrentDate, mCurrentDate, selectedIncidences, preview);
}

void KOEventPopupMenu::printPreview()
{
    print(true);
}

void KOEventPopupMenu::popupDelete()
{
    if (CalendarSupport::hasIncidence(mCurrentIncidence)) {
        Q_EMIT deleteIncidenceSignal(mCurrentIncidence);
    }
}

void KOEventPopupMenu::popupCut()
{
    if (CalendarSupport::hasIncidence(mCurrentIncidence)) {
        Q_EMIT cutIncidenceSignal(mCurrentIncidence);
    }
}

void KOEventPopupMenu::popupCopy()
{
    if (CalendarSupport::hasIncidence(mCurrentIncidence)) {
        Q_EMIT copyIncidenceSignal(mCurrentIncidence);
    }
}

void KOEventPopupMenu::popupPaste()
{
    Q_EMIT pasteIncidenceSignal();
}

void KOEventPopupMenu::toggleAlarm()
{
    if (CalendarSupport::hasIncidence(mCurrentIncidence)) {
        Q_EMIT toggleAlarmSignal(mCurrentIncidence);
    }
}

void KOEventPopupMenu::dissociateOccurrences()
{
    if (CalendarSupport::hasIncidence(mCurrentIncidence)) {
        Q_EMIT dissociateOccurrencesSignal(mCurrentIncidence, mCurrentDate);
    }
}

void KOEventPopupMenu::forward()
{
    if (CalendarSupport::hasIncidence(mCurrentIncidence)) {
        KCalendarCore::Incidence::Ptr const incidence = Akonadi::CalendarUtils::incidence(mCurrentIncidence);
        if (incidence) {
            auto *handler = new Akonadi::ITIPHandler(this);
            connect(handler, &Akonadi::ITIPHandler::sentAsICalendar, this, [handler]() {
                handler->deleteLater();
            });
            handler->setCalendar(mCurrentCalendar);
            handler->sendAsICalendar(incidence, this);
        }
    }
}

void KOEventPopupMenu::createNewEvent()
{
    // Must be a Incidence
    if (!CalendarSupport::hasIncidence(mCurrentIncidence)) {
        return;
    }

    KCalendarCore::Event::Ptr const newEvent(new KCalendarCore::Event());
    KCalendarCore::Incidence::Ptr const currentIncidence(Akonadi::CalendarUtils::incidence(mCurrentIncidence));
    newEvent->setUid(KCalendarCore::CalFormat::createUniqueId());
    newEvent->setDtStart(currentIncidence->dtStart());
    newEvent->setAllDay(currentIncidence->allDay());
    newEvent->setDtEnd(QDateTime());
    if (CalendarSupport::KCalPrefs::instance()->defaultEventReminders()) {
        const KCalendarCore::Alarm::Ptr alarm = newEvent->newAlarm();
        CalendarSupport::createAlarmReminder(alarm, newEvent->type());
    }
    Akonadi::Item newEventItem;
    newEventItem.setMimeType(KCalendarCore::Event::eventMimeType());
    newEventItem.setPayload<KCalendarCore::Event::Ptr>(newEvent);
    IncidenceEditorNG::IncidenceDialog *dlg = IncidenceEditorNG::IncidenceDialogFactory::create(true, KCalendarCore::IncidenceBase::TypeEvent, nullptr, this);
    dlg->setObjectName(QLatin1StringView("incidencedialog"));
    dlg->load(newEventItem);
    dlg->open();
}

void KOEventPopupMenu::createEvent(const Akonadi::Item &item)
{
    mCurrentIncidence = item;
    createEvent();
}

void KOEventPopupMenu::createEvent()
{
    // Must be a Incidence
    if (!CalendarSupport::hasIncidence(mCurrentIncidence)) {
        return;
    }

    // Create New Event over an existing event in the same time-slot
    if (CalendarSupport::hasEvent(mCurrentIncidence)) {
        const KCalendarCore::Event::Ptr newEvent(new KCalendarCore::Event());
        const KCalendarCore::Event::Ptr currentEvent(Akonadi::CalendarUtils::event(mCurrentIncidence));
        newEvent->setUid(KCalendarCore::CalFormat::createUniqueId());
        QDateTime startDt = currentEvent->dtStart();
        startDt.setDate(mCurrentDate);
        newEvent->setDtStart(startDt);
        newEvent->setAllDay(currentEvent->allDay());
        newEvent->setDtEnd(currentEvent->dtEnd());
        if (!currentEvent->allDay()) {
            int const addSecs = (CalendarSupport::KCalPrefs::instance()->mDefaultDuration.time().hour() * 3600)
                + (CalendarSupport::KCalPrefs::instance()->mDefaultDuration.time().minute() * 60);
            newEvent->setDtEnd(startDt.addSecs(addSecs));
        }
        if (CalendarSupport::KCalPrefs::instance()->defaultEventReminders()) {
            const KCalendarCore::Alarm::Ptr alarm = newEvent->newAlarm();
            CalendarSupport::createAlarmReminder(alarm, newEvent->type());
        }
        Akonadi::Item newEventItem;
        newEventItem.setMimeType(KCalendarCore::Event::eventMimeType());
        newEventItem.setPayload<KCalendarCore::Event::Ptr>(newEvent);
        IncidenceEditorNG::IncidenceDialog *dlg =
            IncidenceEditorNG::IncidenceDialogFactory::create(true, KCalendarCore::IncidenceBase::TypeEvent, nullptr, this);
        dlg->setObjectName(QLatin1StringView("incidencedialog"));
        dlg->load(newEventItem);
        dlg->open();
        return;
    }

    // Create New Event from existing todo
    if (CalendarSupport::hasTodo(mCurrentIncidence)) {
        KCalendarCore::Todo::Ptr const todo(Akonadi::CalendarUtils::todo(mCurrentIncidence));
        KCalendarCore::Event::Ptr const event(new KCalendarCore::Event(*todo));
        event->setUid(KCalendarCore::CalFormat::createUniqueId());
        event->setDtStart(todo->dtStart());
        event->setAllDay(todo->allDay());
        event->setDtEnd(todo->dtDue());
        Akonadi::Item newEventItem;
        newEventItem.setMimeType(KCalendarCore::Event::eventMimeType());
        newEventItem.setPayload<KCalendarCore::Event::Ptr>(event);

        IncidenceEditorNG::IncidenceDialog *dlg =
            IncidenceEditorNG::IncidenceDialogFactory::create(true, KCalendarCore::IncidenceBase::TypeEvent, nullptr, this);
        dlg->setObjectName(QLatin1StringView("incidencedialog"));
        dlg->load(newEventItem);
        dlg->open();
    }
}

void KOEventPopupMenu::createNewTodo()
{
    // Must be a Incidence
    if (!CalendarSupport::hasIncidence(mCurrentIncidence)) {
        return;
    }

    KCalendarCore::Todo::Ptr const newTodo(new KCalendarCore::Todo());
    KCalendarCore::Incidence::Ptr const currentIncidence(Akonadi::CalendarUtils::incidence(mCurrentIncidence));
    newTodo->setUid(KCalendarCore::CalFormat::createUniqueId());
    newTodo->setDtStart(currentIncidence->dtStart());
    newTodo->setAllDay(currentIncidence->allDay());
    newTodo->setDtDue(QDateTime());
    if (CalendarSupport::KCalPrefs::instance()->defaultTodoReminders()) {
        const KCalendarCore::Alarm::Ptr alarm = newTodo->newAlarm();
        CalendarSupport::createAlarmReminder(alarm, newTodo->type());
    }
    Akonadi::Item newTodoItem;
    newTodoItem.setMimeType(KCalendarCore::Todo::todoMimeType());
    newTodoItem.setPayload<KCalendarCore::Todo::Ptr>(newTodo);
    IncidenceEditorNG::IncidenceDialog *dlg = IncidenceEditorNG::IncidenceDialogFactory::create(true, KCalendarCore::IncidenceBase::TypeTodo, nullptr, this);
    dlg->setObjectName(QLatin1StringView("incidencedialog"));
    dlg->load(newTodoItem);
    dlg->open();
}

void KOEventPopupMenu::createTodo()
{
    // Must be a Incidence
    if (!CalendarSupport::hasIncidence(mCurrentIncidence)) {
        return;
    }

    // Create New Todo over an existing todo in the same time-slot
    if (CalendarSupport::hasTodo(mCurrentIncidence)) {
        KCalendarCore::Todo::Ptr const newTodo(new KCalendarCore::Todo());
        KCalendarCore::Todo::Ptr const currentTodo(Akonadi::CalendarUtils::todo(mCurrentIncidence));
        newTodo->setUid(KCalendarCore::CalFormat::createUniqueId());
        newTodo->setDtStart(currentTodo->dtStart());
        newTodo->setAllDay(currentTodo->allDay());
        newTodo->setDtDue(currentTodo->dtDue());
        if (CalendarSupport::KCalPrefs::instance()->defaultTodoReminders()) {
            const KCalendarCore::Alarm::Ptr alarm = newTodo->newAlarm();
            CalendarSupport::createAlarmReminder(alarm, newTodo->type());
        }
        Akonadi::Item newTodoItem;
        newTodoItem.setMimeType(KCalendarCore::Todo::todoMimeType());
        newTodoItem.setPayload<KCalendarCore::Todo::Ptr>(newTodo);
        IncidenceEditorNG::IncidenceDialog *dlg =
            IncidenceEditorNG::IncidenceDialogFactory::create(true, KCalendarCore::IncidenceBase::TypeTodo, nullptr, this);
        dlg->setObjectName(QLatin1StringView("incidencedialog"));
        dlg->load(newTodoItem);
        dlg->open();
    }

    // Create New Todo from existing event
    if (CalendarSupport::hasEvent(mCurrentIncidence)) {
        KCalendarCore::Event::Ptr const event(Akonadi::CalendarUtils::event(mCurrentIncidence));
        KCalendarCore::Todo::Ptr const todo(new KCalendarCore::Todo(*event));
        todo->setUid(KCalendarCore::CalFormat::createUniqueId());
        todo->setDtStart(event->dtStart());
        todo->setAllDay(event->allDay());
        todo->setDtDue(event->dtEnd());
        Akonadi::Item newTodoItem;
        newTodoItem.setMimeType(KCalendarCore::Todo::todoMimeType());
        newTodoItem.setPayload<KCalendarCore::Todo::Ptr>(todo);

        IncidenceEditorNG::IncidenceDialog *dlg =
            IncidenceEditorNG::IncidenceDialogFactory::create(true, KCalendarCore::IncidenceBase::TypeTodo, nullptr, this);
        dlg->setObjectName(QLatin1StringView("incidencedialog"));
        dlg->load(newTodoItem);
        dlg->open();
    }
}

void KOEventPopupMenu::toggleTodoCompleted()
{
    if (CalendarSupport::hasTodo(mCurrentIncidence)) {
        Q_EMIT toggleOccurrenceCompletedSignal(mCurrentIncidence, mCurrentDate);
    }
}

#include "moc_koeventpopupmenu.cpp"
