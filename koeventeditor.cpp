/*
    This file is part of KOrganizer.

    Copyright (c) 2001, 2002, 2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/


#include <QFrame>
#include <QPixmap>
#include <QLayout>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QBoxLayout>

#include <kiconloader.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kcal/calendarresources.h>
#include <kcal/resourcecalendar.h>

#include <kcal/calendarlocal.h>

#include "koprefs.h"
#include "koeditorgeneralevent.h"
#include "koeditoralarms.h"
#include "koeditorrecurrence.h"
#include "koeditordetails.h"
#include "koeditorattachments.h"
#include "koeditorfreebusy.h"
#include "kogroupware.h"
#include "kodialogmanager.h"
#include "incidencechanger.h"

#include "koeventeditor.h"

KOEventEditor::KOEventEditor( Calendar *calendar, QWidget *parent )
  : KOIncidenceEditor( QString(), calendar, parent ),
    mEvent( 0 ), mGeneral( 0 ), mRecurrence( 0 ), mFreeBusy( 0 )
{
}

KOEventEditor::~KOEventEditor()
{
  emit dialogClose( mEvent );
}

void KOEventEditor::init()
{
  setupGeneral();
//  setupAlarmsTab();
  setupRecurrence();
  setupAttendeesTab();
  setupFreeBusy();
  setupAttachmentsTab();
  setupDesignerTabs( "event" );

  mDetails->setFreeBusyWidget( mFreeBusy );

  // Propagate date time settings to recurrence tab
  connect( mGeneral, SIGNAL( dateTimesChanged( const QDateTime &, const QDateTime & ) ),
           mRecurrence, SLOT( setDateTimes( const QDateTime &, const QDateTime &) ) );
  connect( mGeneral, SIGNAL( dateTimeStrChanged( const QString & ) ),
           mRecurrence, SLOT( setDateTimeStr( const QString & ) ) );
  connect( mFreeBusy, SIGNAL( dateTimesChanged( const QDateTime &, const QDateTime & ) ),
           mRecurrence, SLOT( setDateTimes( const QDateTime &, const QDateTime & ) ) );

  // Propagate date time settings to gantt tab and back
  connect( mGeneral, SIGNAL( dateTimesChanged( const QDateTime &, const QDateTime & ) ),
           mFreeBusy, SLOT( slotUpdateGanttView( const QDateTime &, const QDateTime & ) ) );
  connect( mFreeBusy, SIGNAL( dateTimesChanged( const QDateTime &, const QDateTime & ) ),
           mGeneral, SLOT( setDateTimes( const QDateTime &, const QDateTime & ) ) );

  connect( mGeneral, SIGNAL( focusReceivedSignal() ),
           SIGNAL( focusReceivedSignal() ) );

  connect( mGeneral, SIGNAL( openCategoryDialog() ),
           SIGNAL( editCategories() ) );
}

void KOEventEditor::reload()
{
  kDebug(5850) << "KOEventEditor::reload()" << endl;

  if ( mEvent ) readEvent( mEvent );
}

void KOEventEditor::setupGeneral()
{
  mGeneral = new KOEditorGeneralEvent( this );

  if( KOPrefs::instance()->mCompactDialogs ) {
    QFrame *topFrame = new QFrame();
    addPage(topFrame, i18n( "General"));
    topFrame->setWhatsThis(
                     i18n("The General tab allows you to set the most common "
                          "options for the event.") );

    QBoxLayout *topLayout = new QVBoxLayout(topFrame);
    topLayout->setSpacing(spacingHint());

    mGeneral->initHeader(topFrame,topLayout);
    mGeneral->initTime(topFrame,topLayout);
    mGeneral->initAlarm(topFrame,topLayout);
    mGeneral->enableAlarm( false );
    mGeneral->initCategories( topFrame, topLayout );

    topLayout->addStretch( 1 );

    QFrame *topFrame2 = new QFrame();
    addPage(topFrame2, i18n("Details"));

    QBoxLayout *topLayout2 = new QVBoxLayout(topFrame2);
    topLayout2->setSpacing(spacingHint());

    mGeneral->initClass(topFrame2,topLayout2);
    mGeneral->initSecrecy( topFrame2, topLayout2 );
    mGeneral->initDescription(topFrame2,topLayout2);
  } else {
    QFrame *topFrame = new QFrame();
    addPage(topFrame, i18n("&General"));
    topFrame->setWhatsThis(
                     i18n("The General tab allows you to set the most common "
                          "options for the event.") );

    QBoxLayout *topLayout = new QVBoxLayout(topFrame);
    topLayout->setSpacing(spacingHint());

    mGeneral->initHeader(topFrame,topLayout);
    mGeneral->initTime(topFrame,topLayout);

    QBoxLayout *alarmLineLayout = new QHBoxLayout();
    alarmLineLayout->setSpacing( spacingHint() );
    topLayout->addItem(alarmLineLayout);
    mGeneral->initAlarm(topFrame,alarmLineLayout);
    alarmLineLayout->addStretch( 1 );
    mGeneral->initClass(topFrame,alarmLineLayout);
    mGeneral->initDescription(topFrame,topLayout);

    QBoxLayout *detailsLayout = new QHBoxLayout();
    detailsLayout->setSpacing( spacingHint() );
    topLayout->addItem(detailsLayout);
    mGeneral->initCategories( topFrame, detailsLayout );
    mGeneral->initSecrecy( topFrame, detailsLayout );
  }

  mGeneral->finishSetup();
}

void KOEventEditor::modified (int /*modification*/)
{
  // Play dump, just reload the event. This dialog has become so complicated
  // that there is no point in trying to be smart here...
  reload();
}

void KOEventEditor::setupRecurrence()
{
  QFrame *topFrame = new QFrame();
  addPage(topFrame, i18n("Rec&urrence") );

  topFrame->setWhatsThis(
        i18n("The Recurrence tab allows you to set options on "
       "how often this event recurs.") );

  QBoxLayout *topLayout = new QVBoxLayout( topFrame );

  mRecurrence = new KOEditorRecurrence( topFrame );
  topLayout->addWidget( mRecurrence );
}

void KOEventEditor::setupFreeBusy()
{
  QFrame *freeBusyPage = new QFrame();
  addPage(freeBusyPage, i18n("&Free/Busy") );
  freeBusyPage->setWhatsThis(
        i18n("The Free/Busy tab allows you to see whether "
       "other attendees are free or busy during your event.") );

  QBoxLayout *topLayout = new QVBoxLayout( freeBusyPage );

  mFreeBusy = new KOEditorFreeBusy( spacingHint(), freeBusyPage );
  topLayout->addWidget( mFreeBusy );
}

void KOEventEditor::editIncidence( Incidence *incidence )
{
  Event*event = dynamic_cast<Event*>(incidence);
  if ( event ) {
    init();

    mEvent = event;
    readEvent(mEvent);
  }

  setCaption( i18n("Edit Event") );
}

void KOEventEditor::newEvent()
{
  init();
  mEvent = 0;
  loadDefaults();
  setCaption( i18n("New Event") );
}

void KOEventEditor::setDates( const QDateTime &from, const QDateTime &to, bool allDay )
{
  mGeneral->setDefaults( from, to, allDay );
  mDetails->setDefaults();
  mAttachments->setDefaults();
  mRecurrence->setDefaults( from, to, allDay );
  if( mFreeBusy ) {
    if ( allDay )
      mFreeBusy->setDateTimes( from, to.addDays( 1 ) );
    else
      mFreeBusy->setDateTimes( from, to );
  }
}

void KOEventEditor::setTexts( const QString &summary, const QString &description )
{
  if ( description.isEmpty() && summary.contains("\n") ) {
    mGeneral->setDescription( summary );
    int pos = summary.find( "\n" );
    mGeneral->setSummary( summary.left( pos ) );
  } else {
    mGeneral->setSummary( summary );
    mGeneral->setDescription( description );
  }
}
  
void KOEventEditor::loadDefaults()
{
  QDateTime from( QDate::currentDate(), KOPrefs::instance()->mStartTime.time() );
  int addSecs = ( KOPrefs::instance()->mDefaultDuration.time().hour()*3600 ) +
                ( KOPrefs::instance()->mDefaultDuration.time().minute()*60 );
  QDateTime to( from.addSecs( addSecs ) );

  setDates( from, to, false );
}

bool KOEventEditor::processInput()
{
  kDebug(5850) << "KOEventEditor::processInput()" << endl;

  if ( !validateInput() || !mChanger ) return false;

  if ( mEvent ) {
    bool rc = true;
    Event *oldEvent = mEvent->clone();
    Event *event = mEvent->clone();

    kDebug(5850) << "KOEventEditor::processInput() write event." << endl;
    writeEvent( event );
    kDebug(5850) << "KOEventEditor::processInput() event written." << endl;

    if( *event == *mEvent )
      // Don't do anything
      kDebug(5850) << "Event not changed\n";
    else {
      kDebug(5850) << "Event changed\n";
      //IncidenceChanger::assignIncidence( mEvent, event );
      writeEvent( mEvent );
      mChanger->changeIncidence( oldEvent, mEvent );
    }
    delete event;
    delete oldEvent;
    return rc;
  } else {
    mEvent = new Event;
    mEvent->setOrganizer( Person( KOPrefs::instance()->fullName(),
                          KOPrefs::instance()->email() ) );
    writeEvent( mEvent );
    if ( !mChanger->addIncidence( mEvent, this ) ) {
      delete mEvent;
      mEvent = 0;
      return false;
    }
  }

  if ( mFreeBusy ) mFreeBusy->cancelReload();

  return true;
}

void KOEventEditor::processCancel()
{
  kDebug(5850) << "KOEventEditor::processCancel()" << endl;

  if ( mFreeBusy ) mFreeBusy->cancelReload();
}

void KOEventEditor::deleteEvent()
{
  kDebug(5850) << "Delete event" << endl;

  if ( mEvent )
    emit deleteIncidenceSignal( mEvent );
  emit dialogClose( mEvent );
  reject();
}

void KOEventEditor::readEvent( Event *event, bool tmpl )
{
  mGeneral->readEvent( event, tmpl );
  mDetails->readEvent( event );
  mRecurrence->readIncidence( event );
  mAttachments->readIncidence( event );
//  mAlarms->readIncidence( event );
  if ( mFreeBusy ) {
    mFreeBusy->readEvent( event );
    mFreeBusy->triggerReload();
  }

  createEmbeddedURLPages( event );
  readDesignerFields( event );
}

void KOEventEditor::writeEvent( Event *event )
{
  mGeneral->writeEvent( event );
  mDetails->writeEvent( event );
  mAttachments->writeIncidence( event );

  cancelRemovedAttendees( event );

  mRecurrence->writeIncidence( event );

  writeDesignerFields( event );
}

bool KOEventEditor::validateInput()
{
  if ( !mGeneral->validateInput() ) return false;
  if ( !mDetails->validateInput() ) return false;
  if ( !mRecurrence->validateInput() ) return false;

  return true;
}

int KOEventEditor::msgItemDelete()
{
  return KMessageBox::warningContinueCancel(this,
      i18n("This item will be permanently deleted."),
      i18n("KOrganizer Confirmation"),KGuiItem(i18n("Delete"),"edit-delete"));
}

void KOEventEditor::loadTemplate( /*const*/ CalendarLocal& cal )
{
  const Event::List events = cal.events();
  if ( events.count() == 0 ) {
    KMessageBox::error( this,
        i18n("Template does not contain a valid event.") );
  } else {
    kDebug(5850) << "KOEventEditor::slotLoadTemplate(): readTemplate" << endl;
    readEvent( events.first(), true );
  }
}

QStringList& KOEventEditor::templates() const
{
  return KOPrefs::instance()->mEventTemplates;
}

void KOEventEditor::slotSaveTemplate( const QString &templateName )
{
  kDebug(5006) << "SlotSaveTemplate" << endl;
  Event *event = new Event;
  writeEvent( event );
  saveAsTemplate( event, templateName );
}

QObject *KOEventEditor::typeAheadReceiver() const
{
  return mGeneral->typeAheadReceiver();
}

#include "koeventeditor.moc"
