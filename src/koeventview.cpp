/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2000, 2001 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "koeventview.h"
#include "kocore.h"
#include "koeventpopupmenu.h"

#include <Akonadi/Calendar/ETMCalendar>
#include <CalendarSupport/KCalPrefs>
#include <CalendarSupport/Utils>

#include "korganizer_debug.h"
#include <KXMLGUIFactory>

#include <QApplication>
#include <QMenu>

//---------------------------------------------------------------------------

KOEventView::KOEventView(QWidget *parent)
    : KOrg::BaseView(parent)
{
    // AKONADI_PORT review: the FocusLineEdit in the editor emits focusReceivedSignal(),
    // which triggered finishTypeAhead. But the global focus widget in QApplication is
    // changed later, thus subsequent keyevents still went to this view, triggering
    // another editor, for each keypress
    // Thus listen to the global focusChanged() signal (seen with Qt 4.6-stable-patched 20091112)
    //  -Frank
    connect(qobject_cast<QApplication *>(QApplication::instance()), &QApplication::focusChanged, this, &KOEventView::focusChanged);
}

//---------------------------------------------------------------------------

KOEventView::~KOEventView()
{
}

//---------------------------------------------------------------------------

KOEventPopupMenu *KOEventView::eventPopup()
{
    auto eventPopup = new KOEventPopupMenu(calendar(), this);

    connect(eventPopup, &KOEventPopupMenu::editIncidenceSignal, this, &KOEventView::editIncidenceSignal);
    connect(eventPopup, &KOEventPopupMenu::showIncidenceSignal, this, &KOEventView::showIncidenceSignal);
    connect(eventPopup, &KOEventPopupMenu::deleteIncidenceSignal, this, &KOEventView::deleteIncidenceSignal);
    connect(eventPopup, &KOEventPopupMenu::cutIncidenceSignal, this, &KOEventView::cutIncidenceSignal);
    connect(eventPopup, &KOEventPopupMenu::copyIncidenceSignal, this, &KOEventView::copyIncidenceSignal);
    connect(eventPopup, &KOEventPopupMenu::pasteIncidenceSignal, this, &KOEventView::pasteIncidenceSignal);
    connect(eventPopup, &KOEventPopupMenu::toggleAlarmSignal, this, &KOEventView::toggleAlarmSignal);
    connect(eventPopup, &KOEventPopupMenu::toggleTodoCompletedSignal, this, &KOEventView::toggleTodoCompletedSignal);
    connect(eventPopup, &KOEventPopupMenu::copyIncidenceToResourceSignal, this, &KOEventView::copyIncidenceToResourceSignal);
    connect(eventPopup, &KOEventPopupMenu::moveIncidenceToResourceSignal, this, &KOEventView::moveIncidenceToResourceSignal);
    connect(eventPopup, &KOEventPopupMenu::dissociateOccurrencesSignal, this, &KOEventView::dissociateOccurrencesSignal);

    return eventPopup;
}

QMenu *KOEventView::newEventPopup()
{
    KXMLGUIClient *client = KOCore::self()->xmlguiClient(this);
    if (!client) {
        qCCritical(KORGANIZER_LOG) << "no xmlGuiClient.";
        return nullptr;
    }
    if (!client->factory()) {
        qCCritical(KORGANIZER_LOG) << "no factory";
        return nullptr; // can happen if called too early
    }

    return static_cast<QMenu *>(client->factory()->container(QStringLiteral("rmb_selection_popup"), client));
}

//---------------------------------------------------------------------------

void KOEventView::popupShow()
{
    Q_EMIT showIncidenceSignal(mCurrentIncidence);
}

//---------------------------------------------------------------------------

void KOEventView::popupEdit()
{
    Q_EMIT editIncidenceSignal(mCurrentIncidence);
}

//---------------------------------------------------------------------------

void KOEventView::popupDelete()
{
    Q_EMIT deleteIncidenceSignal(mCurrentIncidence);
}

//---------------------------------------------------------------------------

void KOEventView::popupCut()
{
    Q_EMIT cutIncidenceSignal(mCurrentIncidence);
}

//---------------------------------------------------------------------------

void KOEventView::popupCopy()
{
    Q_EMIT copyIncidenceSignal(mCurrentIncidence);
}

//---------------------------------------------------------------------------

void KOEventView::showNewEventPopup()
{
    QMenu *popup = newEventPopup();
    if (!popup) {
        qCCritical(KORGANIZER_LOG) << "popup creation failed";
        return;
    }

    popup->popup(QCursor::pos());
}

//---------------------------------------------------------------------------

void KOEventView::defaultAction(const Akonadi::Item &aitem)
{
    const KCalendarCore::Incidence::Ptr incidence = CalendarSupport::incidence(aitem);
    if (!incidence) {
        qCDebug(KORGANIZER_LOG) << "Ouch, null incidence";
        return;
    }

    if (!calendar()->hasRight(aitem, Akonadi::Collection::CanChangeItem)) {
        Q_EMIT showIncidenceSignal(aitem);
    } else {
        Q_EMIT editIncidenceSignal(aitem);
    }
}

void KOEventView::setTypeAheadReceiver(QObject *o)
{
    mTypeAheadReceiver = o;
}

void KOEventView::focusChanged(QWidget *, QWidget *now)
{
    if (mTypeAhead && now && now == mTypeAheadReceiver) {
        finishTypeAhead();
    }
}

void KOEventView::finishTypeAhead()
{
    if (mTypeAheadReceiver) {
        for (QEvent *e : std::as_const(mTypeAheadEvents)) {
            QApplication::sendEvent(mTypeAheadReceiver, e);
        }
    }
    qDeleteAll(mTypeAheadEvents);
    mTypeAheadEvents.clear();
    mTypeAhead = false;
}

bool KOEventView::usesCompletedTodoPixmap(const Akonadi::Item &aitem, const QDate &date)
{
    const KCalendarCore::Todo::Ptr todo = CalendarSupport::todo(aitem);
    if (!todo) {
        return false;
    }

    if (todo->isCompleted()) {
        return true;
    } else if (todo->recurs()) {
        QTime time;
        if (todo->allDay()) {
            time = QTime(0, 0);
        } else {
            time = todo->dtDue().toLocalTime().time();
        }

        QDateTime itemDateTime(date, time, Qt::LocalTime);

        return itemDateTime < todo->dtDue(false).toLocalTime();
    } else {
        return false;
    }
}
