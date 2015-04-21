/*
  This file is part of KOrganizer.

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

#include "koeventview.h"
#include "kocore.h"
#include "koeventpopupmenu.h"

#include <Akonadi/Calendar/ETMCalendar>
#include <calendarsupport/kcalprefs.h>
#include <calendarsupport/utils.h>

#include <libkdepim/widgets/pimmessagebox.h>

#include <KXMLGUIFactory>
#include "korganizer_debug.h"

#include <QApplication>
#include <QMenu>

//---------------------------------------------------------------------------

KOEventView::KOEventView(QWidget *parent)
    : KOrg::BaseView(parent)
{
    mReturnPressed = false;
    mTypeAhead = false;
    mTypeAheadReceiver = Q_NULLPTR;

    //AKONADI_PORT review: the FocusLineEdit in the editor emits focusReceivedSignal(),
    //which triggered finishTypeAhead. But the global focus widget in QApplication is
    //changed later, thus subsequent keyevents still went to this view, triggering
    //another editor, for each keypress
    //Thus listen to the global focusChanged() signal (seen with Qt 4.6-stable-patched 20091112)
    //  -Frank
    connect(qobject_cast<QApplication *>(QApplication::instance()), SIGNAL(focusChanged(QWidget*,QWidget*)),
            this, SLOT(focusChanged(QWidget*,QWidget*)));
}

//---------------------------------------------------------------------------

KOEventView::~KOEventView()
{
}

//---------------------------------------------------------------------------

KOEventPopupMenu *KOEventView::eventPopup()
{
    KOEventPopupMenu *eventPopup = new KOEventPopupMenu(calendar().data(), this);

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
        qCritical() << "no xmlGuiClient.";
        return Q_NULLPTR;
    }
    if (!client->factory()) {
        qCritical() << "no factory";
        return Q_NULLPTR; // can happen if called too early
    }

    return static_cast<QMenu *>
           (client->factory()->container(QStringLiteral("rmb_selection_popup"), client));
}
//---------------------------------------------------------------------------

void KOEventView::popupShow()
{
    emit showIncidenceSignal(mCurrentIncidence);
}

//---------------------------------------------------------------------------

void KOEventView::popupEdit()
{
    emit editIncidenceSignal(mCurrentIncidence);
}

//---------------------------------------------------------------------------

void KOEventView::popupDelete()
{
    emit deleteIncidenceSignal(mCurrentIncidence);
}

//---------------------------------------------------------------------------

void KOEventView::popupCut()
{
    emit cutIncidenceSignal(mCurrentIncidence);
}

//---------------------------------------------------------------------------

void KOEventView::popupCopy()
{
    emit copyIncidenceSignal(mCurrentIncidence);
}

//---------------------------------------------------------------------------

void KOEventView::showNewEventPopup()
{
    QMenu *popup = newEventPopup();
    if (!popup) {
        qCritical() << "popup creation failed";
        return;
    }

    popup->popup(QCursor::pos());
}

//---------------------------------------------------------------------------

void KOEventView::defaultAction(const Akonadi::Item &aitem)
{
    qCDebug(KORGANIZER_LOG);
    const KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence(aitem);
    if (!incidence) {
        qCDebug(KORGANIZER_LOG) << "Ouch, null incidence";
        return;
    }

    if (!calendar()->hasRight(aitem, Akonadi::Collection::CanChangeItem)) {
        emit showIncidenceSignal(aitem);
    } else {
        emit editIncidenceSignal(aitem);
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
        foreach (QEvent *e, mTypeAheadEvents) {
            QApplication::sendEvent(mTypeAheadReceiver, e);
        }
    }
    qDeleteAll(mTypeAheadEvents);
    mTypeAheadEvents.clear();
    mTypeAhead = false;
}

bool KOEventView::usesCompletedTodoPixmap(const Akonadi::Item &aitem, const QDate &date)
{
    const KCalCore::Todo::Ptr todo = CalendarSupport::todo(aitem);
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
            time = todo->dtDue().toTimeSpec(CalendarSupport::KCalPrefs::instance()->timeSpec()).time();
        }

        KDateTime itemDateTime(date, time, CalendarSupport::KCalPrefs::instance()->timeSpec());

        return itemDateTime < todo->dtDue(false);

    } else {
        return false;
    }
}

