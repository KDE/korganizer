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

#include <qtooltip.h>
#include <qframe.h>
#include <qpixmap.h>
#include <qlayout.h>
#include <qwidgetstack.h>

#include <kiconloader.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <libkcal/calendarresources.h>
#include <libkcal/resourcecalendar.h>
#include <kresources/resourceselectdialog.h>

#include <libkdepim/categoryselectdialog.h>
#include <libkcal/calendarlocal.h>

#include "koprefs.h"

#include "koeventeditor.h"
#include "koeventeditor.moc"

KOEventEditor::KOEventEditor( Calendar *calendar, QWidget *parent ) :
  KOIncidenceEditor( i18n("Edit Event"), calendar, parent )
{
  mEvent = 0;
}

KOEventEditor::~KOEventEditor()
{
  emit dialogClose( mEvent );
}

void KOEventEditor::init()
{
  setupGeneral();
  setupAttendeesTab();
  setupRecurrence();

  // Propagate date time settings to recurrence tab
  connect(mGeneral,SIGNAL(dateTimesChanged(QDateTime,QDateTime)),
          mRecurrence,SLOT(setDateTimes(QDateTime,QDateTime)));
  connect(mGeneral,SIGNAL(dateTimeStrChanged(const QString &)),
          mRecurrence,SLOT(setDateTimeStr(const QString &)));

  // Category dialog
  connect(mGeneral,SIGNAL(openCategoryDialog()),mCategoryDialog,SLOT(show()));
  connect(mCategoryDialog,SIGNAL(categoriesSelected(const QString &)),
          mGeneral,SLOT(setCategories(const QString &)));
}

void KOEventEditor::reload()
{
  if ( mEvent ) readEvent( mEvent );
}

void KOEventEditor::setupGeneral()
{
  mGeneral = new KOEditorGeneralEvent( this );

  if( KOPrefs::instance()->mCompactDialogs ) {
    QFrame *topFrame = addPage(i18n("General"));

    QBoxLayout *topLayout = new QVBoxLayout(topFrame);
    topLayout->setSpacing(spacingHint());

    mGeneral->initHeader(topFrame,topLayout);
    mGeneral->initTime(topFrame,topLayout);
//    QBoxLayout *alarmLineLayout = new QHBoxLayout(topLayout);
    mGeneral->initAlarm(topFrame,topLayout);
    mGeneral->enableAlarm( false );
    mGeneral->initCategories( topFrame, topLayout );

    topLayout->addStretch( 1 );

    QFrame *topFrame2 = addPage(i18n("Details"));

    QBoxLayout *topLayout2 = new QVBoxLayout(topFrame2);
    topLayout2->setSpacing(spacingHint());

    mGeneral->initClass(topFrame2,topLayout2);
    mGeneral->initSecrecy( topFrame2, topLayout2 );
    mGeneral->initDescription(topFrame2,topLayout2);
  } else {
    QFrame *topFrame = addPage(i18n("General"));

    QBoxLayout *topLayout = new QVBoxLayout(topFrame);
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
  QFrame *topFrame = addPage( i18n("Recurrence") );

  QBoxLayout *topLayout = new QVBoxLayout( topFrame );

  mRecurrence = new KOEditorRecurrence( topFrame );
  topLayout->addWidget( mRecurrence );
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
}

void KOEventEditor::loadDefaults()
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

void KOEventEditor::deleteEvent()
{
  kdDebug() << "Delete event" << endl;

  if (mEvent) {
    if (KOPrefs::instance()->mConfirm) {
      switch (msgItemDelete()) {
        case KMessageBox::Continue: // OK
          emit eventToBeDeleted(mEvent);
          emit dialogClose(mEvent);
          mCalendar->deleteEvent(mEvent);
          emit eventDeleted();
          reject();
          break;
      }
    }
    else {
      emit eventToBeDeleted(mEvent);
      emit dialogClose(mEvent);
      mCalendar->deleteEvent(mEvent);
      emit eventDeleted();
      reject();
    }
  } else {
    reject();
  }
}

void KOEventEditor::setDefaults(QDateTime from, QDateTime to, bool allDay)
{
  mGeneral->setDefaults(from,to,allDay);
  mDetails->setDefaults();
  mRecurrence->setDefaults(from,to,allDay);
}

void KOEventEditor::readEvent( Event *event, bool tmpl )
{
  mGeneral->readEvent( event, tmpl );
  mDetails->readEvent( event );
  mRecurrence->readEvent( event );

  // categories
  mCategoryDialog->setSelected( event->categories() );
}

void KOEventEditor::writeEvent(Event *event)
{
  mGeneral->writeEvent( event );
  mDetails->writeEvent( event );

  if ( event->organizer() == KOPrefs::instance()->email() ) {
    Event *ev = new Event( *event );
    ev->registerObserver(0);
    mDetails->cancelAttendeeEvent( ev );
    if ( ev->attendeeCount() > 0 ) {
      emit deleteAttendee( ev );
    }
    delete(ev);
  }

  mRecurrence->writeEvent(event);
}

bool KOEventEditor::validateInput()
{
  if (!mGeneral->validateInput()) return false;
  if (!mDetails->validateInput()) return false;
  if (!mRecurrence->validateInput()) return false;
  return true;
}

int KOEventEditor::msgItemDelete()
{
  return KMessageBox::warningContinueCancel(this,
      i18n("This item will be permanently deleted."),
      i18n("KOrganizer Confirmation"),i18n("Delete"));
}

void KOEventEditor::slotLoadTemplate()
{
  CalendarLocal cal;
  Event *event = new Event;
  QString templateName = loadTemplate( &cal, event->type(),
                                       KOPrefs::instance()->mEventTemplates );
  delete event;
  if ( templateName.isEmpty() ) {
    return;
  }

  QPtrList<Event> events = cal.events();
  event = events.first();
  if ( !event ) {
    KMessageBox::error( this,
        i18n("Template does not contain a valid Event.")
        .arg( templateName ) );
  } else {
    kdDebug() << "KOEventEditor::slotLoadTemplate(): readTemplate" << endl;
    readEvent( event, true );
  }
}

void KOEventEditor::slotSaveTemplate()
{
  createSaveTemplateDialog( SaveTemplateDialog::EventType );
}

void KOEventEditor::saveTemplate( const QString &templateName )
{
  Event *event = new Event;
  writeEvent( event );
  saveAsTemplate( event, templateName );
}
