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
#include "koeditordetails.h"
#include "koeditorfreebusy.h"
#include "koeditorgeneralevent.h"
#include "koeditorrecurrence.h"
#include "koeditorconfig.h"
#include "korganizer/incidencechangerbase.h"
#include "../kogroupware.h"

#include <akonadi/kcal/utils.h>

#include <akonadi/kcal/incidencemimetypevisitor.h>

#include <KCal/IncidenceFormatter>

#include <KMessageBox>
#include <klocale.h>
#include <QTabWidget>
#include <QFrame>
#include <QVBoxLayout>

using namespace Akonadi;

KOEventEditor::KOEventEditor( QWidget *parent )
  : KOIncidenceEditor( QString(), QStringList() << Akonadi::IncidenceMimeTypeVisitor::eventMimeType(), parent ),
    mGeneral( 0 ), mRecurrence( 0 ), mFreeBusy( 0 )
{
  mInitialEvent = Event::Ptr( new Event );
  mInitialEventItem.setPayload(mInitialEvent);
}

KOEventEditor::~KOEventEditor()
{
  if ( !mIsCounter ) {
    emit dialogClose( mIncidence );
  }
}

bool KOEventEditor::incidenceModified()
{
  Event::Ptr oldEvent;
  if ( Akonadi::hasEvent( mIncidence ) ) { // modification
    oldEvent = Akonadi::event(mIncidence);
  } else { // new one
    oldEvent = mInitialEvent;
  }

  Event::Ptr newEvent( oldEvent->clone() );
  Akonadi::Item newEventItem;
  newEventItem.setPayload(newEvent);
  fillEvent( newEventItem );

  const bool modified = !( *newEvent == *oldEvent );
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

void KOEventEditor::setupGeneral()
{
  mGeneral = new KOEditorGeneralEvent( this );

  QFrame *topFrame = new QFrame();
  mTabWidget->addTab( topFrame, i18nc( "@title:tab general event settings", "&General" ) );
  topFrame->setWhatsThis( i18nc( "@info:whatsthis",
                                 "The General tab allows you to set the most "
                                 "common options for the event." ) );

  QVBoxLayout *topLayout = new QVBoxLayout( topFrame );
  mGeneral->initInvitationBar( topFrame, topLayout );
  mGeneral->initHeader( topFrame, topLayout );
  mGeneral->initTime( topFrame, topLayout );
  mGeneral->initDescription( topFrame, topLayout );
  mGeneral->initAttachments( topFrame, topLayout );
  connect( mGeneral, SIGNAL(openURL(const KUrl&)),
           this, SLOT(openURL(const KUrl&)) );
  connect( this, SIGNAL(signalAddAttachments(const QStringList&,const QStringList&,bool)),
           mGeneral, SLOT(addAttachments(const QStringList&,const QStringList&,bool)) );

  mGeneral->finishSetup();
}

void KOEventEditor::modified( int modification )
{
  Q_UNUSED( modification );

  // Play dumb, just reload the event. This dialog has become so complicated
  // that there is no point in trying to be smart here...
  readIncidence( mIncidence, QDate(), true );
}

void KOEventEditor::setupRecurrence()
{
#if 0
  QFrame *topFrame = new QFrame();
  mTabWidget->addTab( topFrame, i18nc( "@title:tab", "Rec&urrence" ) );

  topFrame->setWhatsThis( i18nc( "@info:whatsthis",
                                 "The Recurrence tab allows you to set options "
                                 "on how often this event recurs." ) );
  QVBoxLayout *topLayout = new QVBoxLayout( topFrame );
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
  mTabWidget->addTab( freeBusyPage, i18nc( "@title:tab", "&Attendees" ) );
  freeBusyPage->setWhatsThis( i18nc( "@info:whatsthis",
                                     "The Free/Busy tab allows you to see "
                                     "whether other attendees are free or busy "
                                     "during your event." ) );

  QVBoxLayout *topLayout = new QVBoxLayout( freeBusyPage );
  topLayout->setMargin(0);
  mAttendeeEditor = mFreeBusy = new KOEditorFreeBusy( spacingHint(), freeBusyPage );
  topLayout->addWidget( mFreeBusy );
}

void KOEventEditor::newEvent()
{
  init();
  mIncidence = Item();
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
  QDateTime from( QDate::currentDate(), KOEditorConfig::instance()->startTime().time() );
  int addSecs = ( KOEditorConfig::instance()->defaultDuration().time().hour() * 3600 ) +
                ( KOEditorConfig::instance()->defaultDuration().time().minute() * 60 );
  QDateTime to( from.addSecs( addSecs ) );

  setDates( from, to, false );
}

bool KOEventEditor::processInput()
{
  kDebug();
  if ( !validateInput() || !mChanger ) {
    return false;
  }

  QPointer<KOEditorFreeBusy> freeBusy( mFreeBusy );

  if ( Akonadi::hasEvent( mIncidence ) ) {
    Event::Ptr ev = Akonadi::event( mIncidence );
    bool rc = true;
    Event::Ptr oldEvent( ev->clone() );
    Event::Ptr event( ev->clone() );

    Akonadi::Item eventItem;
    eventItem.setPayload(event);
    fillEvent( eventItem );

    if ( *event == *oldEvent ) {
      // Don't do anything
      if ( mIsCounter ) {
        KMessageBox::information(
          this,
          i18nc( "@info",
                 "You did not modify the event so no counter proposal has "
                 "been sent to the organizer." ),
          i18nc( "@title:window", "No Changes" ) );
      }
    } else {
      if ( mIsCounter ) {
        Q_ASSERT( mIncidence.hasPayload<KCal::Event::Ptr>() );
        KOGroupware::instance()->sendCounterProposal( oldEvent.get(), event.get() );

        // add dummy event at the position of the counter proposal
        Event::Ptr event2( event->clone() );
        event2->clearAttendees();
        event2->setSummary( i18nc( "@item", "My counter proposal for: %1", ev->summary() ) );

        Akonadi::Collection col = mCalSelector->currentCollection();
        rc = mChanger->addIncidence( event2, col, this );
      } else {
        mChanger->beginChange( mIncidence );
        ev->startUpdates(); //merge multiple mIncidence->updated() calls into one
        fillEvent(mIncidence);
        rc = mChanger->changeIncidence( oldEvent, mIncidence, KOGlobals::NOTHING_MODIFIED, this );
        ev->endUpdates();
        mChanger->endChange( mIncidence );
      }
    }
    return rc;
  } else {
    //PENDING(AKONADI_PORT) review mIncidence will differ from newly created item
    Event::Ptr newEvent( new Event );
    newEvent->setOrganizer( Person( KOEditorConfig::instance()->fullName(),
                            KOEditorConfig::instance()->email() ) );
    mIncidence.setPayload( newEvent );
    fillEvent( mIncidence );
    Akonadi::Collection col = mCalSelector->currentCollection();
    if ( !mChanger->addIncidence( newEvent, col, this ) ) {
      mIncidence = Item();
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
  if ( Akonadi::hasEvent( mIncidence ) ) {
    emit deleteIncidenceSignal( mIncidence );
  }

  emit dialogClose( mIncidence );
  reject();
}

bool KOEventEditor::read( const Item &eventItem, const QDate &date, bool tmpl )
{
  if ( !Akonadi::hasEvent( eventItem ) ) {
    return false;
  }

  const Event::Ptr event = Akonadi::event( eventItem );
  mGeneral->readEvent( event.get(), date, tmpl );
  mRecurrence->readIncidence( event.get() );
  if ( mFreeBusy ) {
    mFreeBusy->readIncidence( event.get() );
    mFreeBusy->triggerReload();
  }

  createEmbeddedURLPages( event.get() );
  readDesignerFields( eventItem );

  if ( mIsCounter ) {
    mGeneral->invitationBar()->hide();
  }
  return true;
}

void KOEventEditor::fillEvent( const Akonadi::Item &item )
{
  KCal::Event::Ptr event = Akonadi::event(item);
  mGeneral->fillEvent( event.get() );
  if ( mFreeBusy ) {
    mFreeBusy->fillIncidence( event.get() );
  }
  cancelRemovedAttendees( item );
  mRecurrence->fillIncidence( event.get() );
  writeDesignerFields( event.get() );
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

QObject *KOEventEditor::typeAheadReceiver() const
{
  return mGeneral->typeAheadReceiver();
}

void KOEventEditor::updateRecurrenceSummary()
{
  Event::Ptr ev( new Event );

  Akonadi::Item evItem;
  evItem.setPayload(ev);
  fillEvent( evItem );

  mGeneral->updateRecurrenceSummary( IncidenceFormatter::recurrenceString( ev.get() ) );
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

  fillEvent( mInitialEventItem );
  KOIncidenceEditor::show();
}

#include "koeventeditor.moc"
