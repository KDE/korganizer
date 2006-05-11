/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
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

//
// Journal Entry

#include <QLabel>
#include <QLayout>
#include <QCheckBox>

#include <QToolTip>
#include <QToolButton>
//Added by qt3to4:
#include <QPixmap>
#include <QGridLayout>
#include <QEvent>

#include <kdebug.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <ktextedit.h>
#include <ktimeedit.h>
#include <klineedit.h>
#include <kactivelabel.h>
#include <kstdguiitem.h>
#include <kmessagebox.h>

#include <libkcal/journal.h>
#include <libkcal/calendar.h>
#include <kvbox.h>

#include "kodialogmanager.h"
#include "incidencechanger.h"
#include "koglobals.h"

#include "journalentry.h"
#include "journalentry.moc"

class JournalTitleLable : public KActiveLabel
{
public:
  JournalTitleLable( QWidget *parent ) : KActiveLabel( parent ) {}

  void openLink( const QString &/*link*/ ) {}
};


JournalDateEntry::JournalDateEntry( Calendar *calendar, QWidget *parent ) :
  KVBox( parent ), mCalendar( calendar )
{
//kDebug(5850)<<"JournalEntry::JournalEntry, parent="<<parent<<endl;
  mChanger = 0;

  mTitle = new JournalTitleLable( this );
#warning "kde4: porting"  
  //mTitle->setMargin(2);
  mTitle->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
  connect( mTitle, SIGNAL( linkClicked( const QString & ) ),
           this, SLOT( emitNewJournal() ) );
}

JournalDateEntry::~JournalDateEntry()
{
}

void JournalDateEntry::setDate(const QDate &date)
{
  QString dtstring = QString( "<qt><center><b><i>%1</i></b>  " )
                     .arg( KGlobal::locale()->formatDate(date) );

  dtstring += " <font size=\"-1\"><a href=\"#\">" +
              i18n("[Add Journal Entry]") +
              "</a></font></center></qt>";

  mTitle->setHtml( dtstring );
  mDate = date;
  emit setDateSignal( date );
}

void JournalDateEntry::clear()
{
  QList<JournalEntry*> values( mEntries.values() );

  QList<JournalEntry*>::Iterator it = values.begin();
  for ( ; it != values.end(); ++it ) {
    delete (*it);
  }
  mEntries.clear();
}

// should only be called by the KOJournalView now.
void JournalDateEntry::addJournal( Journal *j )
{
  QMap<Journal*,JournalEntry*>::Iterator pos = mEntries.find( j );
  if ( pos != mEntries.end() ) return;

  JournalEntry *entry = new JournalEntry( j, this );
  entry->show();
  entry->setDate( mDate );
  entry->setIncidenceChanger( mChanger );

  mEntries.insert( j, entry );
  connect( this, SIGNAL( setIncidenceChangerSignal( IncidenceChangerBase * ) ),
           entry, SLOT( setIncidenceChanger( IncidenceChangerBase * ) ) );
  connect( this, SIGNAL( setDateSignal( const QDate & ) ),
           entry, SLOT( setDate( const QDate & ) ) );
  connect( this, SIGNAL( flushEntries() ),
           entry, SLOT( flushEntry() ) );
  connect( entry, SIGNAL( deleteIncidence( Incidence* ) ),
           this, SIGNAL( deleteIncidence( Incidence* ) ) );
  connect( entry, SIGNAL( editIncidence( Incidence* ) ),
           this, SIGNAL( editIncidence( Incidence* ) ) );
}

Journal::List JournalDateEntry::journals() const
{
  QList<Journal*> jList( mEntries.keys() );
  Journal::List l;
  QList<Journal*>::Iterator it = jList.begin();
  for ( ; it != jList.end(); ++it ) {
    l.append( *it );
  }
  return l;
}

void JournalDateEntry::setIncidenceChanger( IncidenceChangerBase *changer )
{
  mChanger = changer;
  emit setIncidenceChangerSignal( changer );
}

void JournalDateEntry::emitNewJournal()
{
  emit newJournal( mDate );
}

void JournalDateEntry::journalEdited( Journal *journal )
{
  QMap<Journal*,JournalEntry*>::Iterator pos = mEntries.find( journal );
  if ( pos == mEntries.end() ) return;

  pos.value()->setJournal( journal );

}

void JournalDateEntry::journalDeleted( Journal *journal )
{
  QMap<Journal*,JournalEntry*>::Iterator pos = mEntries.find( journal );
  if ( pos == mEntries.end() ) return;

  delete pos.value();
}





JournalEntry::JournalEntry( Journal* j, QWidget *parent ) :
  QWidget( parent ), mJournal( j )
{
//kDebug(5850)<<"JournalEntry::JournalEntry, parent="<<parent<<endl;
  mDirty = false;
  mWriteInProgress = false;
  mChanger = 0;
  mReadOnly = false;

  mLayout = new QGridLayout( this );
  mLayout->setSpacing( KDialog::spacingHint() );
  mLayout->setMargin( KDialog::marginHint() );

  QString whatsThis = i18n("Sets the Title of this journal entry.");

  mTitleLabel = new QLabel( i18n("&Title: "), this );
  mLayout->addWidget( mTitleLabel, 0, 0 );
  mTitleEdit = new KLineEdit( this );
  mLayout->addWidget( mTitleEdit, 0, 1 );
  mTitleLabel->setBuddy( mTitleEdit );

  mTitleLabel->setWhatsThis( whatsThis );
  mTitleEdit->setWhatsThis( whatsThis );

  mTimeCheck = new QCheckBox( i18n("Ti&me: "), this );
  mLayout->addWidget( mTimeCheck, 0, 2 );
  mTimeEdit = new KTimeEdit( this );
  mLayout->addWidget( mTimeEdit, 0, 3 );
  connect( mTimeCheck, SIGNAL(toggled(bool)),
           this, SLOT(timeCheckBoxToggled(bool)) );
  mTimeCheck->setWhatsThis( i18n("Determines whether this journal entry has "
                                    "a time associated with it") );
  mTimeEdit->setWhatsThis( i18n( "Sets the time associated with this journal "
                                    " entry" ) );

  mDeleteButton = new QToolButton( this );
  mDeleteButton->setObjectName( "deleteButton" );
  QPixmap pix = KOGlobals::self()->smallIcon( "editdelete" );
  mDeleteButton->setIcon( pix );
  mDeleteButton->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
  mDeleteButton->setToolTip( i18n("Delete this journal entry") );
  mDeleteButton->setWhatsThis( i18n("Delete this journal entry") );
  mLayout->addWidget( mDeleteButton, 0, 4 );
  connect( mDeleteButton, SIGNAL(pressed()), this, SLOT(deleteItem()) );

  mEditButton = new QToolButton( this );
  mEditButton->setObjectName( "editButton" );
  mEditButton->setIcon( KOGlobals::self()->smallIcon( "edit" ) );
  mEditButton->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
  mEditButton->setToolTip( i18n("Edit this journal entry") );
  mEditButton->setWhatsThis( i18n("Opens an editor dialog for this journal entry") );
  mLayout->addWidget( mEditButton, 0, 5 );
  connect( mEditButton, SIGNAL(clicked()), this, SLOT( editItem() ) );


  mEditor = new KTextEdit(this);
  mLayout->addWidget( mEditor, 1, 0, 2, 6 );

  connect( mTitleEdit, SIGNAL(textChanged( const QString& )), SLOT(setDirty()) );
  connect( mTimeCheck, SIGNAL(toggled(bool)), SLOT(setDirty()) );
  connect( mTimeEdit, SIGNAL(timeChanged(QTime)), SLOT(setDirty()) );
  connect( mEditor, SIGNAL(textChanged()), SLOT(setDirty()) );

  mEditor->installEventFilter(this);

  readJournal( mJournal );
  mDirty = false;
}

JournalEntry::~JournalEntry()
{
  writeJournal();
}

void JournalEntry::deleteItem()
{
/*  KMessageBox::ButtonCode *code = KMessageBox::warningContinueCancel(this,
      i18n("The journal \"%1\" on %2 will be permanently deleted.")
               .arg( mJournal->summary() )
               .arg( mJournal->dtStartStr() ),
  i18n("KOrganizer Confirmation"), KStdGuiItem::del() );
  if ( code == KMessageBox::Yes ) {*/
    if ( mJournal )
      emit deleteIncidence( mJournal );
//   }
}

void JournalEntry::editItem()
{
  writeJournal();
  if ( mJournal )
    emit editIncidence( mJournal );
}

void JournalEntry::setReadOnly( bool readonly )
{
  mReadOnly = readonly;
  mTitleEdit->setReadOnly( mReadOnly );
  mEditor->setReadOnly( mReadOnly );
  mTimeCheck->setEnabled( !mReadOnly );
  mTimeEdit->setEnabled( !mReadOnly && mTimeCheck->isChecked() );
  mDeleteButton->setEnabled( !mReadOnly );
}


void JournalEntry::setDate(const QDate &date)
{
  writeJournal();
  mDate = date;
}

void JournalEntry::setJournal(Journal *journal)
{
  if ( !mWriteInProgress )
    writeJournal();
  if ( !journal ) return;

  mJournal = journal;
  readJournal( journal );

  mDirty = false;
}

void JournalEntry::setDirty()
{
  mDirty = true;
  kDebug(5850) << "JournalEntry::setDirty()" << endl;
}

bool JournalEntry::eventFilter( QObject *o, QEvent *e )
{
//  kDebug(5850) << "JournalEntry::event received " << e->type() << endl;

  if ( e->type() == QEvent::FocusOut || e->type() == QEvent::Hide ||
       e->type() == QEvent::Close ) {
    writeJournal();
  }
  return QWidget::eventFilter( o, e );    // standard event processing
}


void JournalEntry::readJournal( Journal *j )
{
  mJournal = j;
  mTitleEdit->setText( mJournal->summary() );
  bool hasTime = !mJournal->doesFloat();
  mTimeCheck->setChecked( hasTime );
  mTimeEdit->setEnabled( hasTime );
  if ( hasTime ) {
    mTimeEdit->setTime( mJournal->dtStart().time() );
  }
  //TODO: Allow rich text
  mEditor->setPlainText( mJournal->description() );
  setReadOnly( mJournal->isReadOnly() );
}

void JournalEntry::writeJournalPrivate( Journal *j )
{
  j->setSummary( mTitleEdit->text() );
  bool hasTime = mTimeCheck->isChecked();
  QTime tm( mTimeEdit->getTime() );
  j->setDtStart( QDateTime( mDate, hasTime?tm:QTime(0,0,0) ) );
  j->setFloats( !hasTime );
  j->setDescription( mEditor->toPlainText() );
}

void JournalEntry::writeJournal()
{
//  kDebug(5850) << "JournalEntry::writeJournal()" << endl;

  if ( mReadOnly || !mDirty || !mChanger ) {
    kDebug(5850)<<"Journal either read-only, unchanged or no changer object available"<<endl;
    return;
  }
  bool newJournal = false;
  mWriteInProgress = true;

  Journal *oldJournal = 0;

  if ( !mJournal ) {
    newJournal = true;
    mJournal = new Journal;
    writeJournalPrivate( mJournal );
    if ( !mChanger->addIncidence( mJournal ) ) {
      KODialogManager::errorSaveIncidence( this, mJournal );
      delete mJournal;
      mJournal = 0;
    }
  } else {
    oldJournal = mJournal->clone();
    if ( mChanger->beginChange( mJournal ) ) {
      writeJournalPrivate( mJournal );
      mChanger->changeIncidence( oldJournal, mJournal, KOGlobals::DESCRIPTION_MODIFIED );
      mChanger->endChange( mJournal );
    }
    delete oldJournal;
  }
  mDirty = false;
  mWriteInProgress = false;
}

void JournalEntry::flushEntry()
{
  if (!mDirty) return;

  writeJournal();
}

void JournalEntry::timeCheckBoxToggled(bool on)
{
  mTimeEdit->setEnabled(on);
  if(on)
    mTimeEdit->setFocus();
}
