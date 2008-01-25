/*
    This file is part of KOrganizer.

    Copyright (c) 1997, 1998 Preston Brown <pbrown@kde.org>
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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "kojournaleditor.h"

#include "koeditorgeneraljournal.h"
#include "koeditordetails.h"
#include "kodialogmanager.h"
#include "koprefs.h"

#include <libkcal/journal.h>
#include <libkcal/calendarlocal.h>
#include <korganizer/baseview.h>

#include <kmessagebox.h>
#include <klocale.h>
#include <kdebug.h>

#include <qlayout.h>

using namespace KCal;

KOJournalEditor::KOJournalEditor( Calendar *calendar, QWidget *parent ) :
  KOIncidenceEditor( i18n("Edit Journal Entry"), calendar, parent )
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
  setupAttendeesTab();
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

    mGeneral->initTitle( topFrame, topLayout );
    mGeneral->initDate( topFrame, topLayout );
    mGeneral->initDescription( topFrame, topLayout );
  } else {
    QFrame *topFrame = addPage(i18n("&General"));

    QBoxLayout *topLayout = new QVBoxLayout(topFrame);
    topLayout->setSpacing(spacingHint());

    mGeneral->initTitle( topFrame, topLayout );
    mGeneral->initDate( topFrame, topLayout );
    mGeneral->initDescription( topFrame, topLayout );
  }

  mGeneral->finishSetup();
}

void KOJournalEditor::editIncidence( Incidence *incidence, Calendar * )
{
  Journal *journal=dynamic_cast<Journal*>(incidence);
  if (journal)
  {
    init();

    mJournal = journal;
    readJournal(mJournal);
  }
}


void KOJournalEditor::newJournal()
{
  init();
  mJournal = 0;
  loadDefaults();
}

void KOJournalEditor::setTexts( const QString &summary, const QString &description )
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



void KOJournalEditor::loadDefaults()
{
  setDate( QDate::currentDate() );
}

bool KOJournalEditor::processInput()
{
  if ( !validateInput() ) return false;

  if ( mJournal ) {
    Journal *oldJournal = mJournal->clone();
    writeJournal( mJournal );
    mChanger->changeIncidence( oldJournal, mJournal );
    delete oldJournal;
  } else {
    mJournal = new Journal;
    mJournal->setOrganizer( Person( KOPrefs::instance()->fullName(),
                            KOPrefs::instance()->email() ) );

    writeJournal( mJournal );

    if ( !mChanger->addIncidence( mJournal, this ) ) {
      KODialogManager::errorSaveIncidence( this, mJournal );
      delete mJournal;
      mJournal = 0;
      return false;
    }
  }

  return true;
}

void KOJournalEditor::deleteJournal()
{
  kdDebug(5850) << "Delete journal" << endl;

  if ( mJournal )
    emit deleteIncidenceSignal( mJournal );
  emit dialogClose( mJournal );
  reject();
}

void KOJournalEditor::setDate( const QDate &date )
{
  mGeneral->setDefaults( date );
  mDetails->setDefaults();
}

void KOJournalEditor::readJournal( Journal *journal )
{
  kdDebug(5851)<<"read Journal"<<endl;
  mGeneral->readJournal( journal );
  mDetails->readEvent( journal );
}

void KOJournalEditor::writeJournal( Journal *journal )
{
  mGeneral->writeJournal( journal );
  mDetails->writeEvent( journal );
}

bool KOJournalEditor::validateInput()
{
  return mGeneral->validateInput() && mDetails->validateInput();
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

void KOJournalEditor::loadTemplate( /*const*/ CalendarLocal& cal)
{
  Journal::List journals = cal.journals();
  if ( journals.count() == 0 ) {
    KMessageBox::error( this,
        i18n("Template does not contain a valid journal.") );
  } else {
    readJournal( journals.first() );
  }
}

void KOJournalEditor::slotSaveTemplate( const QString &templateName )
{
  Journal *journal = new Journal;
  writeJournal( journal );
  saveAsTemplate( journal, templateName );
}

QStringList& KOJournalEditor::templates() const
{
  return KOPrefs::instance()->mJournalTemplates;
}
#include "kojournaleditor.moc"
