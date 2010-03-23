/*
  This file is part of KOrganizer.

  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2007 Mike Arthur <mike@mikearthur.co.uk>

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

// Journal Entry

#include "journalview.h"
#include "kodialogmanager.h"
#include "incidencechanger.h"
#include "koglobals.h"
#include "kocorehelper.h"
#include "calprinter.h"

#include <akonadi/kcal/calendar.h>
#include <kcal/incidenceformatter.h>
#include <kcal/journal.h>

#include <akonadi/kcal/utils.h>

#include <kdebug.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <ktimeedit.h>
#include <klineedit.h>
#include <kstandardguiitem.h>
#include <kmessagebox.h>
#include <kvbox.h>
#include <KTextBrowser>

#include <QLabel>
#include <QLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QPixmap>
#include <QGridLayout>
#include <QEvent>

#include "journalview.moc"

using namespace Akonadi;

JournalDateView::JournalDateView( Akonadi::Calendar *calendar, QWidget *parent )
  : KVBox( parent ), mCalendar( calendar )
{
  mChanger = 0;
}

JournalDateView::~JournalDateView()
{
}

void JournalDateView::setDate( const QDate &date )
{
  mDate = date;
  emit setDateSignal( date );
}

void JournalDateView::clear()
{
  qDeleteAll( mEntries );
  mEntries.clear();
}

// should only be called by the KOJournalView now.
void JournalDateView::addJournal( const Item &j )
{
  QMap<Item::Id,JournalView *>::Iterator pos = mEntries.find( j.id() );
  if ( pos != mEntries.end() ) {
    return;
  }

  JournalView *entry = new JournalView( j, this );
  entry->show();
  entry->setCalendar( mCalendar );
  entry->setDate( mDate );
  entry->setIncidenceChanger( mChanger );

  mEntries.insert( j.id(), entry );
  connect( this, SIGNAL(setIncidenceChangerSignal(IncidenceChangerBase *)),
           entry, SLOT(setIncidenceChanger(IncidenceChangerBase *)) );
  connect( this, SIGNAL(setDateSignal(const QDate &)),
           entry, SLOT(setDate(const QDate &)) );
  connect( entry, SIGNAL(deleteIncidence(Akonadi::Item)),
           this, SIGNAL(deleteIncidence(Akonadi::Item)) );
  connect( entry, SIGNAL(editIncidence(Akonadi::Item)),
           this, SIGNAL(editIncidence(Akonadi::Item)) );
}

Item::List JournalDateView::journals() const
{
  Item::List l;
  Q_FOREACH ( const JournalView *const i, mEntries ) {
    l.push_back( i->journal() );
  }
  return l;
}

void JournalDateView::setIncidenceChanger( IncidenceChangerBase *changer )
{
  mChanger = changer;
  emit setIncidenceChangerSignal( changer );
}

void JournalDateView::emitNewJournal()
{
  emit newJournal( mDate );
}

void JournalDateView::journalEdited( const Item &journal )
{
  QMap<Item::Id,JournalView *>::Iterator pos = mEntries.find( journal.id() );
  if ( pos == mEntries.end() ) {
    return;
  }

  pos.value()->setJournal( journal );

}

void JournalDateView::journalDeleted( const Item &journal )
{
  QMap<Item::Id,JournalView *>::Iterator pos = mEntries.find( journal.id() );
  if ( pos == mEntries.end() ) {
    return;
  }

  delete pos.value();
  mEntries.remove( journal.id() );
}

JournalView::JournalView( const Item &j, QWidget *parent )
  : QWidget( parent ), mJournal( j )
{
  mDirty = false;
  mWriteInProgress = false;
  mChanger = 0;
  mReadOnly = false;

  mLayout = new QGridLayout( this );
  mLayout->setSpacing( KDialog::spacingHint() );
  mLayout->setMargin( KDialog::marginHint() );

  mBrowser = new KTextBrowser( this );
  mBrowser->setFrameStyle( QFrame::StyledPanel );
  mLayout->addWidget( mBrowser, 0, 0, 1, 3 );

  mEditButton = new QPushButton( this );
  mEditButton->setObjectName( "editButton" );
  mEditButton->setText( i18n( "&Edit" ) );
  mEditButton->setIcon( KOGlobals::self()->smallIcon( "document-properties" ) );
  mEditButton->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
  mEditButton->setToolTip( i18n( "Edit this journal entry" ) );
  mEditButton->setWhatsThis( i18n( "Opens an editor dialog for this journal entry" ) );
  mLayout->addWidget( mEditButton, 1, 0 );
  connect( mEditButton, SIGNAL(clicked()), this, SLOT(editItem()) );

  mDeleteButton = new QPushButton( this );
  mDeleteButton->setObjectName( "deleteButton" );
  mDeleteButton->setText( i18n( "&Delete" ) );
  QPixmap pix = KOGlobals::self()->smallIcon( "edit-delete" );
  mDeleteButton->setIcon( pix );
  mDeleteButton->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
  mDeleteButton->setToolTip( i18n( "Delete this journal entry" ) );
  mDeleteButton->setWhatsThis( i18n( "Delete this journal entry" ) );
  mLayout->addWidget( mDeleteButton, 1, 1 );
  connect( mDeleteButton, SIGNAL(pressed()), this, SLOT(deleteItem()) );

  mPrintButton = new QPushButton( this );
  mPrintButton->setText( i18n( "&Print" ) );
  mPrintButton->setObjectName( "printButton" );
  mPrintButton->setIcon( KOGlobals::self()->smallIcon( "document-print" ) );
  mPrintButton->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
  mPrintButton->setToolTip( i18n( "Print this journal entry" ) );
  mPrintButton->setWhatsThis( i18n( "Opens a print dialog for this journal entry" ) );
  mLayout->addWidget( mPrintButton, 1, 2 );
  connect( mPrintButton, SIGNAL(clicked()), this, SLOT(printItem()) );

  readJournal( mJournal );
  mDirty = false;
}

JournalView::~JournalView()
{
}

void JournalView::deleteItem()
{
  if ( Akonadi::hasJournal( mJournal ) ) {
    emit deleteIncidence( mJournal );
  }
}

void JournalView::editItem()
{
  if ( Akonadi::hasJournal( mJournal ) ) {
    emit editIncidence( mJournal );
  }
}

void JournalView::printItem()
{
  if ( const Journal::Ptr j = Akonadi::journal( mJournal ) ) {
    KOCoreHelper helper;
    CalPrinter printer( this, mCalendar, &helper, true );
    connect( this, SIGNAL(configChanged()), &printer, SLOT(updateConfig()) );

    Incidence::List selectedIncidences;
    selectedIncidences.append( j.get() );

    printer.print( KOrg::CalPrinterBase::Incidence,
                   mDate, mDate, selectedIncidences );
  }
}

void JournalView::setReadOnly( bool readonly )
{
  mReadOnly = readonly;
  mEditButton->setEnabled( !mReadOnly );
  mDeleteButton->setEnabled( !mReadOnly );
}

void JournalView::setCalendar( Akonadi::Calendar *cal )
{
  mCalendar = cal;
}

void JournalView::setDate( const QDate &date )
{
  mDate = date;
}

void JournalView::setJournal( const Item &journal )
{
  if ( !Akonadi::hasJournal( journal ) ) {
    return;
  }

  mJournal = journal;
  readJournal( journal );

  mDirty = false;
}

void JournalView::setDirty()
{
  mDirty = true;
  kDebug();
}

bool JournalView::eventFilter( QObject *o, QEvent *e )
{
  return QWidget::eventFilter( o, e );    // standard event processing
}

void JournalView::readJournal( const Item &j )
{
  int baseFontSize = KGlobalSettings::generalFont().pointSize();
  mJournal = j;
  const Journal::Ptr journal = Akonadi::journal( j );
  mBrowser->clear();
  QTextCursor cursor = QTextCursor( mBrowser->textCursor() );
  cursor.movePosition( QTextCursor::Start );

  QTextBlockFormat bodyBlock = QTextBlockFormat( cursor.blockFormat() );
  //FIXME: Do padding
  bodyBlock.setTextIndent( 2 );
  QTextCharFormat bodyFormat = QTextCharFormat( cursor.charFormat() );
  if ( !journal->summary().isEmpty() ) {
    QTextCharFormat titleFormat = bodyFormat;
    titleFormat.setFontWeight( QFont::Bold );
    titleFormat.setFontPointSize( baseFontSize + 4 );
    cursor.insertText( journal->summary(), titleFormat );
    cursor.insertBlock();
  }
  QTextCharFormat dateFormat = bodyFormat;
  dateFormat.setFontWeight( QFont::Bold );
  dateFormat.setFontPointSize( baseFontSize + 1 );
  cursor.insertText( IncidenceFormatter::dateTimeToString(
                       journal->dtStart(), journal->allDay() ), dateFormat );
  cursor.insertBlock();
  cursor.insertBlock();
  cursor.setBlockCharFormat( bodyFormat );
  const QString description = journal->description();
  if ( journal->descriptionIsRich() ) {
    mBrowser->insertHtml( description );
  } else {
    mBrowser->insertPlainText( description );
  }
  setReadOnly( journal->isReadOnly() );
}
