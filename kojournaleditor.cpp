/*
    This file is part of KOrganizer.

    Copyright (c) 1997, 1998 Preston Brown
    Copyright (c) 2000-2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>		

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "kojournaleditor.h"

#include "koeditorgeneraljournal.h"
#include "kodialogmanager.h"
#include "koprefs.h"

#include <libkcal/journal.h>
#include <libkcal/calendarlocal.h>

#include <kmessagebox.h>
#include <klocale.h>
#include <kdebug.h>

#include <qlayout.h>

using namespace KCal;

KOJournalEditor::KOJournalEditor( Calendar *calendar, QWidget *parent ) :
  KOIncidenceEditor( i18n("Edit Journal"), calendar, parent )
{
  mJournal = 0;
}

KOJournalEditor::~KOJournalEditor()
{
  emit dialogClose( mJournal );
}

void KOJournalEditor::init()
{
  setupGeneral();
}

void KOJournalEditor::reload()
{
  kdDebug(5851)<<"reloading Journal"<<endl;
  if ( mJournal ) readJournal( mJournal );
}

void KOJournalEditor::setupGeneral()
{
  mGeneral = new KOEditorGeneralJournal(this);

  if (KOPrefs::instance()->mCompactDialogs) {
    QFrame *topFrame = addPage(i18n("General"));

    QBoxLayout *topLayout = new QVBoxLayout( topFrame );
    topLayout->setMargin( marginHint() );
    topLayout->setSpacing( spacingHint() );

    mGeneral->initDate( topFrame, topLayout );
    mGeneral->initDescription( topFrame, topLayout );
  } else {
    QFrame *topFrame = addPage(i18n("&General"));

    QBoxLayout *topLayout = new QVBoxLayout(topFrame);
    topLayout->setSpacing(spacingHint());

    mGeneral->initDate( topFrame, topLayout );
    mGeneral->initDescription( topFrame, topLayout );
  }

  mGeneral->finishSetup();
}

void KOJournalEditor::editIncidence( Incidence *incidence )
{
  Journal *journal=dynamic_cast<Journal*>(incidence);
  if (journal)
  {
    init();

    mJournal = journal;
    readJournal(mJournal);
  }
}

void KOJournalEditor::newJournal( QDate date )
{
  init();

  mJournal = 0;
  setDefaults( date );
}

void KOJournalEditor::newJournal( const QString &text )
{
  init();

  mJournal = 0;

  loadDefaults();

  mGeneral->setDescription( text );
}

void KOJournalEditor::newJournal( const QString &text, QDate date )
{
  init();

  mJournal = 0;

  loadDefaults();

  mGeneral->setDescription( text );
	mGeneral->setDate( date );
}

void KOJournalEditor::loadDefaults()
{
  setDefaults( QDate::currentDate() );
}

// @TODO: make sure calendar()->endChange is called somewhere!
bool KOJournalEditor::processInput()
{
  if ( !validateInput() ) return false;

  if ( mJournal ) {
    Journal *oldJournal = mJournal->clone();

    writeJournal( mJournal );

    mJournal->setRevision( mJournal->revision() + 1 );

    emit incidenceChanged( oldJournal, mJournal );

    delete oldJournal;
  } else {
    mJournal = new Journal;
//    mJournal->setOrganizer( KOPrefs::instance()->email() );
    mJournal->setOrganizer( Person( KOPrefs::instance()->fullName(), 
                            KOPrefs::instance()->email() ) );

    writeJournal( mJournal );

    if ( !mCalendar->addJournal( mJournal ) ) {
      KODialogManager::errorSaveIncidence( this, mJournal );
      delete mJournal;
      mJournal = 0;
      return false;
    }

    emit incidenceAdded( mJournal );
  }

  return true;
}

void KOJournalEditor::processCancel()
{
  if ( mJournal ) {
    emit editCanceled( mJournal );
  }
}

void KOJournalEditor::deleteJournal()
{
  kdDebug(5850) << "Delete journal" << endl;

  if ( mJournal )
    emit deleteIncidenceSignal( mJournal );
  emit dialogClose( mJournal );
  reject();
}

void KOJournalEditor::setDefaults( QDate date )
{
  mGeneral->setDefaults( date );
}

void KOJournalEditor::readJournal( Journal *journal )
{
  kdDebug(5851)<<"read Journal"<<endl;
  mGeneral->readJournal( journal );
}

void KOJournalEditor::writeJournal( Journal *journal )
{
  mGeneral->writeJournal( journal );
}

bool KOJournalEditor::validateInput()
{
  return mGeneral->validateInput();
}

int KOJournalEditor::msgItemDelete()
{
  return KMessageBox::warningContinueCancel( this,
      i18n("This journal entry will be permanently deleted."),
      i18n("KOrganizer Confirmation"), KGuiItem( i18n("Delete"), "editdelete" ));
}

void KOJournalEditor::modified( int /*modification*/)
{
  // Play dump, just reload the Journal. This dialog has become so complicated that
  // there is no point in trying to be smart here...
  reload();
}

void KOJournalEditor::slotLoadTemplate()
{
  CalendarLocal cal( KOPrefs::instance()->mTimeZoneId );
  Journal *journal = new Journal;
  QString templateName = loadTemplate( &cal, journal->type(),
                                       KOPrefs::instance()->mJournalTemplates );
  delete journal;
  if ( templateName.isEmpty() ) {
    return;
  }

  Journal::List journals = cal.journals();
  if ( journals.count() == 0 ) {
    KMessageBox::error( this,
        i18n("Template '%1' does not contain a valid journal.")
        .arg( templateName ) );
  } else {
    readJournal( journals.first() );
  }
}

void KOJournalEditor::saveTemplate( const QString &templateName )
{
  Journal *journal = new Journal;
  writeJournal( journal );
  saveAsTemplate( journal, templateName );
}

#include "kojournaleditor.moc"
