/*
    This file is part of KOrganizer.
    Copyright (c) 2001,2002 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

// $Id$

#include <qtooltip.h>
#include <qframe.h>
#include <qpixmap.h>
#include <qlayout.h>
#include <qwidgetstack.h>

#include <kdatepik.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <klocale.h>

#include "koprefs.h"
#include "categoryselectdialog.h"

#include "koeventeditor.h"
#include "koeventeditor.moc"

KOEventEditor::KOEventEditor(Calendar *calendar) :
  KOIncidenceEditor(i18n("Edit Event"),calendar)
{
  mEvent = 0;
}

KOEventEditor::~KOEventEditor()
{
}

void KOEventEditor::init()
{
  setupGeneral();
  setupAttendeesTab();
#ifndef KORG_NORECURRENCE
  setupRecurrence();

  // Propagate date time settings to recurrence tab
  connect(mGeneral,SIGNAL(dateTimesChanged(QDateTime,QDateTime)),
          mRecurrence,SLOT(setDateTimes(QDateTime,QDateTime)));
  connect(mGeneral,SIGNAL(dateTimeStrChanged(const QString &)),
          mRecurrence,SLOT(setDateTimeStr(const QString &)));

  // Enable/Disable recurrence tab
  connect(mGeneral,SIGNAL(recursChanged(bool)),
          SLOT(enableRecurrence(bool)));
#endif

  // Category dialog
  connect(mGeneral,SIGNAL(openCategoryDialog()),mCategoryDialog,SLOT(show()));
  connect(mCategoryDialog,SIGNAL(categoriesSelected(const QString &)),
          mGeneral,SLOT(setCategories(const QString &)));
}

void KOEventEditor::setupGeneral()
{
  mGeneral = new KOEditorGeneralEvent( this );

  if( KOPrefs::instance()->mCompactDialogs ) {
    QFrame *topFrame = addPage(i18n("General"));

    QBoxLayout *topLayout = new QVBoxLayout(topFrame);
    topLayout->setMargin(marginHint());
    topLayout->setSpacing(spacingHint());

    mGeneral->initHeader(topFrame,topLayout);
    mGeneral->initTime(topFrame,topLayout);
//    QBoxLayout *alarmLineLayout = new QHBoxLayout(topLayout);
    mGeneral->initAlarm(topFrame,topLayout);
    mGeneral->initCategories( topFrame, topLayout );

    topLayout->addStretch( 1 );

    QFrame *topFrame2 = addPage(i18n("Details"));

    QBoxLayout *topLayout2 = new QVBoxLayout(topFrame2);
    topLayout2->setMargin(marginHint());
    topLayout2->setSpacing(spacingHint());

    mGeneral->initClass(topFrame2,topLayout2);
    mGeneral->initSecrecy( topFrame2, topLayout2 );
    mGeneral->initDescription(topFrame2,topLayout2);
  } else {
    QFrame *topFrame = addPage(i18n("General"));

    QBoxLayout *topLayout = new QVBoxLayout(topFrame);
    topLayout->setMargin(marginHint());
    topLayout->setSpacing(spacingHint());

    mGeneral->initHeader(topFrame,topLayout);
    mGeneral->initTime(topFrame,topLayout);
    QBoxLayout *alarmLineLayout = new QHBoxLayout(topLayout);
    mGeneral->initAlarm(topFrame,alarmLineLayout);
    mGeneral->initClass(topFrame,alarmLineLayout);
    mGeneral->initDescription(topFrame,topLayout);
    QBoxLayout *detailsLayout = new QHBoxLayout(topLayout);
    mGeneral->initCategories( topFrame, detailsLayout );
    mGeneral->initSecrecy( topFrame, detailsLayout );
  }

  mGeneral->finishSetup();
}

void KOEventEditor::setupRecurrence()
{
  QFrame *topFrame = addPage(i18n("Recurrence"));

  QBoxLayout *topLayout = new QVBoxLayout(topFrame);  
  topLayout->setMargin(marginHint());

  mRecurrenceStack = new QWidgetStack(topFrame);
  topLayout->addWidget(mRecurrenceStack);

  mRecurrence = new KOEditorRecurrence(spacingHint(),mRecurrenceStack);
  mRecurrenceStack->addWidget(mRecurrence,0);
  
  mRecurrenceDisabled = new QLabel(
      i18n("This event does not recur.\nEnable Recurrence in General Tab."),
      mRecurrenceStack);
  mRecurrenceDisabled->setAlignment(AlignCenter);
  mRecurrenceStack->addWidget(mRecurrenceDisabled,1);
}

void KOEventEditor::enableRecurrence(bool enable)
{
  if (enable) mRecurrenceStack->raiseWidget(mRecurrence);
  else mRecurrenceStack->raiseWidget(mRecurrenceDisabled);
  
  mRecurrence->setEnabled(enable);
}

void KOEventEditor::editEvent(Event *event)
{
  init();

  mEvent = event;
  readEvent(mEvent);
}

void KOEventEditor::newEvent( QDateTime from, QDateTime to, bool allDay )
{
  init();

  mEvent = 0;
  setDefaults(from,to,allDay);

  enableButton(User1,false);
}

void KOEventEditor::slotDefault()
{
  int fmt = KOPrefs::instance()->mStartTime;
  
  QDateTime from(QDate::currentDate(), QTime(fmt,0,0));
  QDateTime to(QDate::currentDate(),
               QTime(fmt+KOPrefs::instance()->mDefaultDuration,0,0));

  setDefaults(from,to,false);
}

bool KOEventEditor::processInput()
{
  if (!validateInput()) return false;

  Event *event = 0;

  if (mEvent) event = mEvent;
  else {
    event = new Event;
    event->setOrganizer(KOPrefs::instance()->email());
  }
  
  writeEvent(event);
  
  if (mEvent) {
    event->setRevision(event->revision()+1);
    emit eventChanged(event);
  } else {
    mCalendar->addEvent(event);
    mEvent = event;
    emit eventAdded(event);
  }
  
  return true;
}

void KOEventEditor::slotUser1()
{
  kdDebug() << "Delete event" << endl;
  if (mEvent) {
    emit eventToBeDeleted(mEvent);
    mCalendar->deleteEvent(mEvent);
    emit eventDeleted();
    reject();
  } else {
    reject();
  }
}

void KOEventEditor::setDefaults(QDateTime from, QDateTime to, bool allDay)
{
  mGeneral->setDefaults(from,to,allDay);
  mDetails->setDefaults();
#ifndef KORG_NORECURRENCE
  mRecurrence->setDefaults(from,to,allDay);

  enableRecurrence(false);
#endif
}

void KOEventEditor::readEvent(Event *event)
{
  mGeneral->readEvent(event);
  mDetails->readEvent(event);
#ifndef KORG_NORECURRENCE
  mRecurrence->readEvent(event);

  enableRecurrence(event->recurrence()->doesRecur());
#endif

  // categories
  mCategoryDialog->setSelected(event->categories());
}

void KOEventEditor::writeEvent(Event *event)
{
  mGeneral->writeEvent(event);
  mDetails->writeEvent(event);
	if (event->organizer()==KOPrefs::instance()->email()) {
	  Event *ev = new Event(*event);
	  mDetails->cancelAttendeeEvent(ev);
    if (ev->attendeeCount()>0) {
	    emit deleteAttendee(ev);
		}
	}

#ifndef KORG_NORECURRENCE
  mRecurrence->writeEvent(event);
#endif
}

bool KOEventEditor::validateInput()
{
  if (!mGeneral->validateInput()) return false;
  if (!mDetails->validateInput()) return false;
#ifndef KORG_NORECURRENCE
  if (!mRecurrence->validateInput()) return false;
#endif
  return true;
}

