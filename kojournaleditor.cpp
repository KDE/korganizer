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
#include "koeditorgeneraljournal.h"
#include "koeditordetails.h"
#include "koeditorattachments.h"
#include "kodialogmanager.h"
#include "koprefs.h"
#include "korganizer/baseview.h"

#include <kcal/journal.h>
#include <kcal/calendarlocal.h>

#include <kmessagebox.h>
#include <klocale.h>

#include <QLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QBoxLayout>

using namespace KCal;

KOJournalEditor::KOJournalEditor( Calendar *calendar, QWidget *parent )
  : KOIncidenceEditor( i18n( "Edit Journal Entry" ), calendar, parent ),
    mJournal( 0 ), mCalendar( 0 )
{
}

KOJournalEditor::~KOJournalEditor()
{
  emit dialogClose( mJournal );
}

void KOJournalEditor::init()
{
  setupGeneral();
  setupAttendeesTab();

  connect( mGeneral, SIGNAL(openCategoryDialog()),
           SIGNAL(editCategories()) );
  connect( this, SIGNAL(updateCategoryConfig()),
           mGeneral, SIGNAL(updateCategoryConfig()) );

  connect( mDetails, SIGNAL(updateAttendeeSummary(int)),
           mGeneral, SLOT(updateAttendeeSummary(int)) );
}

void KOJournalEditor::reload()
{
  if ( mJournal ) {
    readJournal( mJournal );
  }
}

void KOJournalEditor::setupGeneral()
{
  mGeneral = new KOEditorGeneralJournal( mCalendar, this );

  QFrame *topFrame = new QFrame();
  addPage( topFrame, i18nc( "@title general journal settings", "General" ) );

  QBoxLayout *topLayout = new QVBoxLayout( topFrame );
  if ( KOPrefs::instance()->mCompactDialogs ) {
    topLayout->setMargin( marginHint() );
  }
  topLayout->setSpacing( spacingHint() );

  mGeneral->initTitle( topFrame, topLayout );
  mGeneral->initDate( topFrame, topLayout );
  mGeneral->initDescription( topFrame, topLayout );
  mGeneral->initCategories( topFrame, topLayout );

  mGeneral->finishSetup();
}

void KOJournalEditor::editIncidence( Incidence *incidence, Calendar * )
{
  Journal *journal=dynamic_cast<Journal*>( incidence );
  if ( journal ) {
    init();

    mJournal = journal;
    readJournal( mJournal );
  }
}

void KOJournalEditor::newJournal()
{
  init();
  mJournal = 0;
  loadDefaults();
}

void KOJournalEditor::setTexts( const QString &summary,
                                const QString &description,
                                bool richDescription )
{
  if ( description.isEmpty() && summary.contains( "\n" ) ) {
    mGeneral->setDescription( summary, false );
    int pos = summary.indexOf( "\n" );
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
  if ( !validateInput() ) {
    return false;
  }

  if ( mJournal ) {
    Journal *oldJournal = mJournal->clone();
    fillJournal( mJournal );
    mChanger->changeIncidence( oldJournal, mJournal );
    delete oldJournal;
  } else {
    mJournal = new Journal;
    mJournal->setOrganizer( Person( KOPrefs::instance()->fullName(),
                            KOPrefs::instance()->email() ) );

    fillJournal( mJournal );

    if ( !mChanger->addIncidence( mJournal, this ) ) {
      delete mJournal;
      mJournal = 0;
      return false;
    }
  }

  return true;
}

void KOJournalEditor::deleteJournal()
{
  if ( mJournal ) {
    emit deleteIncidenceSignal( mJournal );
  }

  emit dialogClose( mJournal );
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

bool KOJournalEditor::incidenceModified() {
  Journal *newJournal = 0;

  if ( mJournal ) {
    newJournal = mJournal->clone();
    fillJournal( newJournal );
  }
  return mJournal && !( *newJournal == *mJournal );
}

void KOJournalEditor::readJournal( Journal *journal, bool tmpl )
{
  //TODO: just tmpl variable
  Q_UNUSED( tmpl );

  if ( !journal ) {
    return;
  }

  mGeneral->readJournal( journal );
  mDetails->readIncidence( journal );
}

void KOJournalEditor::fillJournal( Journal *journal )
{
  mGeneral->fillJournal( journal );
  mDetails->fillIncidence( journal );
}

bool KOJournalEditor::validateInput()
{
  return mGeneral->validateInput();
}

int KOJournalEditor::msgItemDelete()
{
  return KMessageBox::warningContinueCancel(
    this,
    i18n( "This journal entry will be permanently deleted." ),
    i18n( "KOrganizer Confirmation" ),
    KGuiItem( i18n( "Delete" ), "edit-delete" ) );
}

void KOJournalEditor::modified( int /*modification*/)
{
  // Play dump, just reload the Journal. This dialog has become so complicated that
  // there is no point in trying to be smart here...
  reload();
}

void KOJournalEditor::loadTemplate( CalendarLocal &cal )
{
  Journal::List journals = cal.journals();
  if ( journals.count() == 0 ) {
    KMessageBox::error( this, i18n( "Template does not contain a valid journal." ) );
  } else {
    readJournal( journals.first() );
  }
}

void KOJournalEditor::slotSaveTemplate( const QString &templateName )
{
  Journal *journal = new Journal;
  fillJournal( journal );
  saveAsTemplate( journal, templateName );
}

QStringList &KOJournalEditor::templates() const
{
  return KOPrefs::instance()->mJournalTemplates;
}

#include "kojournaleditor.moc"
