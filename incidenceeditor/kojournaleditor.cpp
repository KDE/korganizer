/*
  This file is part of KOrganizer.

  Copyright (c) 1997, 1998 Preston Brown <pbrown@kde.org>
  Copyright (c) 2000-2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include "kojournaleditor.h"
#include "koeditorconfig.h"
#include "koeditordetails.h"
#include "koeditorgeneraljournal.h"

#include <akonadi/kcal/utils.h> //krazy:exclude=camelcase since kdepim/akonadi
#include <akonadi/kcal/incidencechanger.h>

#include <Akonadi/CollectionComboBox>
#include <Akonadi/KCal/IncidenceMimeTypeVisitor>

#include <KLocale>

#include <QTabWidget>
#include <QVBoxLayout>

KOJournalEditor::KOJournalEditor( QWidget *parent )
  : KOIncidenceEditor( QString(),
                       QStringList() << Akonadi::IncidenceMimeTypeVisitor::journalMimeType(),
                       parent )
{
}

KOJournalEditor::~KOJournalEditor()
{
  emit dialogClose( mIncidence );
}

void KOJournalEditor::init()
{
  setupGeneral();
  setupAttendeesTab();

  connect( mGeneral, SIGNAL(openCategoryDialog()),
           SIGNAL(editCategories()) );
  connect( this, SIGNAL(updateCategoryConfig()),
           mGeneral, SIGNAL(updateCategoryConfig()) );

}

void KOJournalEditor::setupGeneral()
{
  mGeneral = new KOEditorGeneralJournal( this );

  QFrame *topFrame = new QFrame();
  mTabWidget->addTab( topFrame, i18nc( "@title general journal settings", "General" ) );

  QVBoxLayout *topLayout = new QVBoxLayout( topFrame );
  mGeneral->initTitle( topFrame, topLayout );
  mGeneral->initDate( topFrame, topLayout );
  mGeneral->initDescription( topFrame, topLayout );
  mGeneral->initCategories( topFrame, topLayout );

  mGeneral->finishSetup();
}

void KOJournalEditor::newJournal()
{
  init();
  mIncidence = Item();
  loadDefaults();
  setCaption( i18nc( "@title:window", "New Journal" ) );
}

void KOJournalEditor::setTexts( const QString &summary, const QString &description,
                                bool richDescription )
{
  if ( description.isEmpty() && summary.contains( '\n' ) ) {
    mGeneral->setDescription( summary, false );
    int pos = summary.indexOf( '\n' );
    mGeneral->setSummary( summary.left( pos ) );
  } else {
    mGeneral->setSummary( summary );
    mGeneral->setDescription( description, richDescription );
  }
}

void KOJournalEditor::loadDefaults()
{
  setDate( QDate::currentDate() );
  setTime( QTime::currentTime() );
}

bool KOJournalEditor::processInput()
{
  kDebug();
  if ( !validateInput() || !mChanger ) {
    return false;
  }

  if ( Akonadi::hasJournal( mIncidence ) ) {
    bool rc = true;
    Journal::Ptr oldJournal( Akonadi::journal( mIncidence )->clone() );
    Journal::Ptr journal( Akonadi::journal( mIncidence )->clone() );

    fillJournal( journal.get() );

    if ( *oldJournal == *journal ) {
      // Don't do anything
    } else {
      journal = Akonadi::journal( mIncidence );
      if ( mChanger->beginChange( mIncidence ) ) {
        journal->startUpdates(); //merge multiple mIncidence->updated() calls into one
        fillJournal( journal.get() );
        rc = mChanger->changeIncidence( oldJournal,
                                        mIncidence,
                                        Akonadi::IncidenceChanger::NOTHING_MODIFIED,
                                        this );
        journal->endUpdates();
        mChanger->endChange( mIncidence );
      } else {
        return false;
      }
    }
    return rc;
  } else {
    Journal::Ptr j( new Journal );
    j->setOrganizer( Person( KOEditorConfig::instance()->fullName(),
                             KOEditorConfig::instance()->email() ) );
    fillJournal( j.get() );
    //PENDING(AKONADI_PORT) review: mIncidence will be != the newly created item
    mIncidence.setPayload( j );
    Akonadi::Collection col = mCalSelector->currentCollection();
    if ( !mChanger->addIncidence( j, col, this ) ) {
      mIncidence = Item();
      return false;
    }
  }
  return true;
}

void KOJournalEditor::deleteJournal()
{
  if ( Akonadi::hasJournal( mIncidence ) ) {
    emit deleteIncidenceSignal( mIncidence );
  }

  emit dialogClose( mIncidence );
  reject();
}

void KOJournalEditor::setDate( const QDate &date )
{
  mGeneral->setDate( date );
}

void KOJournalEditor::setTime( const QTime &time )
{
  mGeneral->setTime( time );
}

bool KOJournalEditor::incidenceModified()
{
  Journal::Ptr newJournal;
  Journal *oldJournal = 0;
  bool modified;

  if ( Akonadi::hasJournal( mIncidence ) ) { // modification
    oldJournal = Akonadi::journal( mIncidence ).get();
  } else { // new one
    oldJournal = &mInitialJournal;
  }

  newJournal.reset( oldJournal->clone() );
  fillJournal( newJournal.get() );
  modified = !( *newJournal == *oldJournal );
  return modified;
}

bool KOJournalEditor::read( const Item &item, const QDate &date, bool tmpl )
{
  const Journal::Ptr journal = Akonadi::journal( item );
  if ( !journal ) {
    return false;
  }

  mGeneral->readJournal( journal.get(), date, tmpl );
  mDetails->readIncidence( journal.get() );

  return true;
}

void KOJournalEditor::fillJournal( Journal *journal )
{
  mGeneral->fillJournal( journal );
  mDetails->fillIncidence( journal );
}

bool KOJournalEditor::validateInput()
{
  if ( !mGeneral->validateInput() ) {
    return false;
  }
  if ( !mDetails->validateInput() ) {
    return false;
  }
  return true;
}

void KOJournalEditor::modified()
{
  // Play dumb, just reload the Journal. This dialog has become so complicated
  // that there is no point in trying to be smart here...
  readIncidence( mIncidence, QDate(), true );
}

void KOJournalEditor::show()
{
  fillJournal( &mInitialJournal );
  KOIncidenceEditor::show();
}

#include "kojournaleditor.moc"
