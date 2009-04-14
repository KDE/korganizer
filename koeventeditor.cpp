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

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "koeventeditor.h"
#include "koprefs.h"
#include "koeditorgeneralevent.h"
#include "koeditoralarms.h"
#include "koeditorrecurrence.h"
#include "koeditordetails.h"
#include "koeditorfreebusy.h"
#include "kogroupware.h"
#include "kohelper.h"
#include "kodialogmanager.h"
#include "incidencechanger.h"

#include <kcal/calendarresources.h>
#include <kcal/resourcecalendar.h>
#include <kcal/incidenceformatter.h>
#include <kcal/calendarlocal.h>

#include <kiconloader.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <QFrame>
#include <QPixmap>
#include <QPointer>
#include <QLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QBoxLayout>

KOEventEditor::KOEventEditor( Calendar *calendar, QWidget *parent )
  : KOIncidenceEditor( QString(), calendar, parent ),
    mEvent( 0 ), mCalendar( 0 ), mGeneral( 0 ), mRecurrence( 0 ), mFreeBusy( 0 )
{
}

KOEventEditor::~KOEventEditor()
{
  if ( !mIsCounter ) {
    emit dialogClose( mEvent );
  }
}

bool KOEventEditor::incidenceModified()
{
  Event *newEvent = 0;
  Event *oldEvent = 0;
  bool modified;

  if ( mEvent ) { // modification    
    oldEvent = mEvent;
  } else { // new one
    oldEvent = &mInitialEvent;
  }

  newEvent = oldEvent->clone();
  fillEvent( newEvent );  

  modified = !( *newEvent == *oldEvent );

  delete newEvent;

  return modified;
}

void KOEventEditor::init()
{
  setupGeneral();
  setupRecurrence();
  setupFreeBusy();
  setupDesignerTabs( "event" );

  // Propagate date time settings to recurrence tab
  connect( mGeneral, SIGNAL(dateTimesChanged(const QDateTime&,const QDateTime& )),
           mRecurrence, SLOT(setDateTimes(const QDateTime&,const QDateTime&)) );
  connect( mGeneral, SIGNAL(dateTimeStrChanged(const QString&)),
           mRecurrence, SLOT(setDateTimeStr(const QString&)) );
  connect( mFreeBusy, SIGNAL(dateTimesChanged(const QDateTime&,const QDateTime&)),
           mRecurrence, SLOT(setDateTimes(const QDateTime&,const QDateTime&)) );

  // Propagate date time settings to gantt tab and back
  connect( mGeneral, SIGNAL(dateTimesChanged(const QDateTime&,const QDateTime&)),
           mFreeBusy, SLOT(slotUpdateGanttView(const QDateTime&,const QDateTime&)) );
  connect( mFreeBusy, SIGNAL(dateTimesChanged(const QDateTime&,const QDateTime&)),
           mGeneral, SLOT(setDateTimes(const QDateTime&,const QDateTime&)) );

  connect( mGeneral, SIGNAL(focusReceivedSignal()),
           SIGNAL(focusReceivedSignal()) );

  connect( mGeneral, SIGNAL(openCategoryDialog()),
           SIGNAL(editCategories()) );
  connect( this, SIGNAL(updateCategoryConfig()),
           mGeneral, SIGNAL(updateCategoryConfig()) );

  connect( mFreeBusy, SIGNAL(updateAttendeeSummary(int)),
           mGeneral, SLOT(updateAttendeeSummary(int)) );

  connect( mGeneral, SIGNAL(editRecurrence()),
           mRecurrenceDialog, SLOT(show()) );
  connect( mRecurrenceDialog, SIGNAL(okClicked()),
           SLOT(updateRecurrenceSummary()) );

  connect( mGeneral, SIGNAL(acceptInvitation()),
           mFreeBusy, SLOT(acceptForMe()) );
  connect( mGeneral, SIGNAL(declineInvitation()),
           mFreeBusy, SLOT(declineForMe()) );

  updateRecurrenceSummary();
}

void KOEventEditor::reload()
{
  if ( mEvent ) {
    readEvent( mEvent, true );
  }
}

void KOEventEditor::setupGeneral()
{
  mGeneral = new KOEditorGeneralEvent( mCalendar, this );

  if( KOPrefs::instance()->mCompactDialogs ) {
    QFrame *topFrame = new QFrame();
    addPage( topFrame, i18nc( "@title:tab general event settings", "General" ) );
    topFrame->setWhatsThis( i18nc( "@info:whatsthis",
                                   "The General tab allows you to set the most "
                                   "common options for the event." ) );

    QBoxLayout *topLayout = new QVBoxLayout( topFrame );
    topLayout->setSpacing( spacingHint() );

    mGeneral->initHeader( topFrame, topLayout );
    mGeneral->initTime( topFrame, topLayout );
    mGeneral->initAlarm( topFrame, topLayout );
    mGeneral->enableAlarm( false );

    topLayout->addStretch( 1 );

    QFrame *topFrame2 = new QFrame();
    addPage( topFrame2, i18nc( "@title:tab", "Details" ) );

    QBoxLayout *topLayout2 = new QVBoxLayout( topFrame2 );
    topLayout2->setSpacing( spacingHint() );

    mGeneral->initClass( topFrame2, topLayout2 );
    mGeneral->initSecrecy( topFrame2, topLayout2 );
    mGeneral->initDescription( topFrame2, topLayout2 );
  } else {
    QFrame *topFrame = new QFrame();
    addPage( topFrame, i18nc( "@title:tab general event settings", "&General" ) );
    topFrame->setWhatsThis( i18nc( "@info:whatsthis",
                                   "The General tab allows you to set the most "
                                   "common options for the event." ) );

    QBoxLayout *topLayout = new QVBoxLayout( topFrame );
    topLayout->setSpacing( spacingHint() );

    mGeneral->initInvitationBar( topFrame, topLayout );
    mGeneral->initHeader( topFrame, topLayout );
    mGeneral->initTime( topFrame, topLayout );
    mGeneral->initDescription( topFrame, topLayout );
    mGeneral->initAttachments( topFrame, topLayout );
    connect( mGeneral, SIGNAL(openURL(const KUrl&)),
             this, SLOT(openURL(const KUrl&)) );
    connect( this, SIGNAL(signalAddAttachments(const QStringList&,const QStringList&,bool)),
             mGeneral, SLOT(addAttachments(const QStringList&,const QStringList&,bool)) );
  }

  mGeneral->finishSetup();
}

void KOEventEditor::modified( int modification )
{
  Q_UNUSED( modification );

  // Play dumb, just reload the event. This dialog has become so complicated
  // that there is no point in trying to be smart here...
  reload();
}

void KOEventEditor::setupRecurrence()
{
#if 0
  QFrame *topFrame = new QFrame();
  addPage( topFrame, i18nc( "@title:tab", "Rec&urrence" ) );

  topFrame->setWhatsThis( i18nc( "@info:whatsthis",
                                 "The Recurrence tab allows you to set options "
                                 "on how often this event recurs." ) );

  QBoxLayout *topLayout = new QVBoxLayout( topFrame );

  mRecurrence = new KOEditorRecurrence( topFrame );
  topLayout->addWidget( mRecurrence );
#endif
  mRecurrenceDialog = new KOEditorRecurrenceDialog( this );
  mRecurrenceDialog->hide();
  mRecurrence = mRecurrenceDialog->editor();
}

void KOEventEditor::setupFreeBusy()
{
  QFrame *freeBusyPage = new QFrame();
  addPage( freeBusyPage, i18nc( "@title:tab", "&Attendees" ) );
  freeBusyPage->setWhatsThis( i18nc( "@info:whatsthis",
                                     "The Free/Busy tab allows you to see "
                                     "whether other attendees are free or busy "
                                     "during your event." ) );

  QBoxLayout *topLayout = new QVBoxLayout( freeBusyPage );

  mAttendeeEditor = mFreeBusy = new KOEditorFreeBusy( spacingHint(), freeBusyPage );
  topLayout->addWidget( mFreeBusy );
}

void KOEventEditor::editIncidence( Incidence *incidence, Calendar *calendar )
{
  Event*event = dynamic_cast<Event*>( incidence );
  if ( event ) {
    init();

    mEvent = event;
    mCalendar = calendar;
    readEvent( mEvent, false );
  }

  setCaption( i18nc( "@title:window",
                     "Edit Event : %1", KOHelper::resourceLabel( calendar, incidence ) ) );
}

void KOEventEditor::newEvent()
{
  init();
  mEvent = 0;
  loadDefaults();
  setCaption( i18nc( "@title:window", "New Event" ) );
}

void KOEventEditor::setDates( const QDateTime &from, const QDateTime &to, bool allDay )
{
  mGeneral->setDefaults( from, to, allDay );
  mRecurrence->setDefaults( from, to, allDay );
  if ( mFreeBusy ) {
    if ( allDay ) {
      mFreeBusy->setDateTimes( from, to.addDays( 1 ) );
    } else {
      mFreeBusy->setDateTimes( from, to );
    }
  }
}

void KOEventEditor::setTexts( const QString &summary, const QString &description,
                              bool richDescription )
{
  if ( description.isEmpty() && summary.contains( "\n" ) ) {
    mGeneral->setDescription( summary, richDescription );
    int pos = summary.indexOf( "\n" );
    mGeneral->setSummary( summary.left( pos ) );
  } else {
    mGeneral->setSummary( summary );
    mGeneral->setDescription( description, richDescription );
  }
}

void KOEventEditor::loadDefaults()
{
  QDateTime from( QDate::currentDate(), KOPrefs::instance()->mStartTime.time() );
  int addSecs = ( KOPrefs::instance()->mDefaultDuration.time().hour() * 3600 ) +
                ( KOPrefs::instance()->mDefaultDuration.time().minute() * 60 );
  QDateTime to( from.addSecs( addSecs ) );

  setDates( from, to, false );
}

bool KOEventEditor::processInput()
{
  if ( !validateInput() || !mChanger ) {
    return false;
  }

  QPointer<KOEditorFreeBusy> freeBusy( mFreeBusy );

  if ( mEvent ) {
    bool rc = true;
    Event *oldEvent = mEvent->clone();
    Event *event = mEvent->clone();

    fillEvent( event );

    if ( *event == *mEvent ) {
      // Don't do anything
      if ( mIsCounter ) {
        KMessageBox::information(
          this,
          i18n( "You didn't change the event, "
                "thus no counter proposal has been sent to the organizer." ),
          i18n( "No changes" ) );
      }
    } else {
      //IncidenceChanger::assignIncidence( mEvent, event );
      fillEvent( mEvent );
      if ( mIsCounter ) {
        KOGroupware::instance()->sendCounterProposal( mCalendar, oldEvent, mEvent );
        // add dummy event at the position of the counter proposal
        Event *event = mEvent->clone();
        event->clearAttendees();
        event->setSummary( i18n( "My counter proposal for: %1", mEvent->summary() ) );
        mChanger->addIncidence( event );
      } else {
        mChanger->changeIncidence( oldEvent, mEvent );
      }
    }
    delete event;
    delete oldEvent;
    return rc;
  } else {
    mEvent = new Event;
    mEvent->setOrganizer( Person( KOPrefs::instance()->fullName(),
                          KOPrefs::instance()->email() ) );
    fillEvent( mEvent );
    if ( !mChanger->addIncidence( mEvent, this ) ) {
      delete mEvent;
      mEvent = 0;
      return false;
    }
  }
  // if "this" was deleted, freeBusy is 0 (being a guardedptr)
  if ( freeBusy ) {
    freeBusy->cancelReload();
  }

  return true;
}

void KOEventEditor::processCancel()
{
  if ( mFreeBusy ) {
    mFreeBusy->cancelReload();
  }
}

void KOEventEditor::deleteEvent()
{
  if ( mEvent ) {
    emit deleteIncidenceSignal( mEvent );
  }

  emit dialogClose( mEvent );
  reject();
}

void KOEventEditor::readEvent( Event *event, bool tmpl )
{
  if ( !event ) {
    return;
  }

  mGeneral->readEvent( event, tmpl );
  mRecurrence->readIncidence( event );
  if ( mFreeBusy ) {
    mFreeBusy->readIncidence( event );
    mFreeBusy->triggerReload();
  }

  createEmbeddedURLPages( event );
  readDesignerFields( event );

  if ( mIsCounter ) {
    mGeneral->invitationBar()->hide();
  }
}

void KOEventEditor::fillEvent( Event *event )
{
  mGeneral->fillEvent( event );
  if ( mFreeBusy ) {
    mFreeBusy->fillIncidence( event );
  }

  cancelRemovedAttendees( event );

  mRecurrence->fillIncidence( event );

  writeDesignerFields( event );
}

bool KOEventEditor::validateInput()
{
  if ( !mGeneral->validateInput() ) {
    return false;
  }
  if ( !mDetails->validateInput() ) {
    return false;
  }
  if ( !mRecurrence->validateInput() ) {
    return false;
  }

  return true;
}

int KOEventEditor::msgItemDelete()
{
  return KMessageBox::warningContinueCancel(
    this,
    i18nc( "@info", "This item will be permanently deleted." ),
    i18nc( "@title:window", "KOrganizer Confirmation" ),
    KGuiItem( i18nc( "@action:button", "Delete" ), "edit-delete" ) );
}

void KOEventEditor::loadTemplate( CalendarLocal &cal )
{
  Event::List events = cal.events();
  if ( events.count() == 0 ) {
    KMessageBox::error( this, i18nc( "@info", "Template does not contain a valid event." ) );
  } else {
    readEvent( events.first(), true );
  }
}

QStringList &KOEventEditor::templates() const
{
  return KOPrefs::instance()->mEventTemplates;
}

void KOEventEditor::slotSaveTemplate( const QString &templateName )
{
  Event *event = new Event;
  fillEvent( event );
  saveAsTemplate( event, templateName );
}

QObject *KOEventEditor::typeAheadReceiver() const
{
  return mGeneral->typeAheadReceiver();
}

void KOEventEditor::updateRecurrenceSummary()
{
  Event *ev =  new Event();
  fillEvent( ev );
  mGeneral->updateRecurrenceSummary( IncidenceFormatter::recurrenceString( ev ) );
  delete ev;
}

void KOEventEditor::selectInvitationCounterProposal( bool enable )
{
  KOIncidenceEditor::selectInvitationCounterProposal( enable );
  if ( enable ) {
    mGeneral->invitationBar()->hide();
  }
}

void KOEventEditor::show()
{
  mGeneral->fillEvent( &mInitialEvent );
  KOIncidenceEditor::show();
}

#include "koeventeditor.moc"
