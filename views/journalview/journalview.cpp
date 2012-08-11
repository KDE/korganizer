/*
  This file is part of KOrganizer.

  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2007 Mike McQuaid <mike@mikemcquaid.com>

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
#include "calprinter.h"
#include "kocorehelper.h"
#include "koglobals.h"

#include <calendarsupport/utils.h>

#include <KCalCore/Journal>

#include <KCalUtils/IncidenceFormatter>
#include <Akonadi/Calendar/ETMCalendar>

#include <KDialog>
#include <KTextBrowser>

#include <QEvent>
#include <QHBoxLayout>
#include <QPushButton>

JournalDateView::JournalDateView( const Akonadi::ETMCalendar::Ptr &calendar, QWidget *parent )
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
void JournalDateView::addJournal( const Akonadi::Item &j )
{
  QMap<Akonadi::Item::Id,JournalView *>::Iterator pos = mEntries.find( j.id() );
  if ( pos != mEntries.end() ) {
    return;
  }

  QWidget *container = new QWidget( this );
  QHBoxLayout *layout = new QHBoxLayout( container );
  layout->addStretch( 1 );
  JournalView *entry = new JournalView( j, mCalendar, this );
  layout->addWidget( entry, 3/*stretch*/ );
  layout->addStretch( 1 );

  entry->show();
  entry->setDate( mDate );
  entry->setIncidenceChanger( mChanger );

  mEntries.insert( j.id(), entry );
  connect( this, SIGNAL(setIncidenceChangerSignal(Akonadi::IncidenceChanger*)),
           entry, SLOT(setIncidenceChanger(Akonadi::IncidenceChanger*)) );
  connect( this, SIGNAL(setDateSignal(QDate)),
           entry, SLOT(setDate(QDate)) );
  connect( entry, SIGNAL(deleteIncidence(Akonadi::Item)),
           this, SIGNAL(deleteIncidence(Akonadi::Item)) );
  connect( entry, SIGNAL(editIncidence(Akonadi::Item)),
           this, SIGNAL(editIncidence(Akonadi::Item)) );
  connect( entry, SIGNAL(incidenceSelected(Akonadi::Item,QDate)),
                  SIGNAL(incidenceSelected(Akonadi::Item,QDate)) );
}

Akonadi::Item::List JournalDateView::journals() const
{
  Akonadi::Item::List l;
  Q_FOREACH ( const JournalView *const i, mEntries ) {
    l.push_back( i->journal() );
  }
  return l;
}

void JournalDateView::setIncidenceChanger( Akonadi::IncidenceChanger *changer )
{
  mChanger = changer;
  emit setIncidenceChangerSignal( changer );
}

void JournalDateView::emitNewJournal()
{
  emit newJournal( mDate );
}

void JournalDateView::journalEdited( const Akonadi::Item &journal )
{
  QMap<Akonadi::Item::Id,JournalView *>::Iterator pos = mEntries.find( journal.id() );
  if ( pos == mEntries.end() ) {
    return;
  }

  pos.value()->setJournal( journal );

}

void JournalDateView::journalDeleted( const Akonadi::Item &journal )
{
  QMap<Akonadi::Item::Id,JournalView *>::Iterator pos = mEntries.find( journal.id() );
  if ( pos == mEntries.end() ) {
    return;
  }

  delete pos.value();
  mEntries.remove( journal.id() );
}

JournalView::JournalView( const Akonadi::Item &j,
                          const Akonadi::ETMCalendar::Ptr &calendar,
                          QWidget *parent )
  : QFrame( parent ), mJournal( j ), mCalendar( calendar )
{
  mDirty = false;
  mWriteInProgress = false;
  mChanger = 0;

  QVBoxLayout *verticalLayout = new QVBoxLayout( this );
  verticalLayout->setSpacing( KDialog::spacingHint() );
  verticalLayout->setMargin( KDialog::marginHint() );

  mBrowser = new KTextBrowser( this );
  mBrowser->viewport()->installEventFilter( this );
  mBrowser->setFrameStyle( QFrame::NoFrame );
  verticalLayout->addWidget( mBrowser );

  QHBoxLayout *buttonsLayout = new QHBoxLayout();
  verticalLayout->addLayout( buttonsLayout );
  buttonsLayout->addStretch();

  mEditButton = new QPushButton( this );
  mEditButton->setObjectName( "editButton" );
  mEditButton->setText( i18n( "&Edit" ) );
  mEditButton->setIcon( KOGlobals::self()->smallIcon( "document-properties" ) );
  mEditButton->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
  mEditButton->setToolTip( i18n( "Edit this journal entry" ) );
  mEditButton->setWhatsThis( i18n( "Opens an editor dialog for this journal entry" ) );
  buttonsLayout->addWidget( mEditButton );
  connect( mEditButton, SIGNAL(clicked()), this, SLOT(editItem()) );

  mDeleteButton = new QPushButton( this );
  mDeleteButton->setObjectName( "deleteButton" );
  mDeleteButton->setText( i18n( "&Delete" ) );
  QPixmap pix = KOGlobals::self()->smallIcon( "edit-delete" );
  mDeleteButton->setIcon( pix );
  mDeleteButton->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
  mDeleteButton->setToolTip( i18n( "Delete this journal entry" ) );
  mDeleteButton->setWhatsThis( i18n( "Delete this journal entry" ) );
  buttonsLayout->addWidget( mDeleteButton );
  connect( mDeleteButton, SIGNAL(pressed()), this, SLOT(deleteItem()) );

  mPrintButton = new QPushButton( this );
  mPrintButton->setText( i18n( "&Print" ) );
  mPrintButton->setObjectName( "printButton" );
  mPrintButton->setIcon( KOGlobals::self()->smallIcon( "document-print" ) );
  mPrintButton->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
  mPrintButton->setToolTip( i18n( "Print this journal entry" ) );
  mPrintButton->setWhatsThis( i18n( "Opens a print dialog for this journal entry" ) );
  buttonsLayout->addWidget( mPrintButton );
  connect( mPrintButton, SIGNAL(clicked()), this, SLOT(printItem()) );

  readJournal( mJournal );
  mDirty = false;
  setFrameStyle( QFrame::Box );
  // These probably shouldn't be hardcoded
  setStyleSheet( "QFrame { border: 1px solid; border-radius: 7px; } " );
  mBrowser->setStyleSheet( "QFrame { border: 0px solid white } " );
}

JournalView::~JournalView()
{
}

bool JournalView::eventFilter ( QObject *object, QEvent *event )
{
  Q_UNUSED( object );

  // object is our KTextBrowser
  if ( !mJournal.isValid() ) {
    return false;
  }

  switch( event->type() ) {
    case QEvent::MouseButtonPress:
      emit incidenceSelected( mJournal, mDate );
      break;
    case QEvent::MouseButtonDblClick:
      emit editIncidence( mJournal );
      break;
    default:
      break;
  }

  return false;
}

void JournalView::deleteItem()
{
  if ( CalendarSupport::hasJournal( mJournal ) ) {
    emit deleteIncidence( mJournal );
  }
}

void JournalView::editItem()
{
  if ( CalendarSupport::hasJournal( mJournal ) ) {
    emit editIncidence( mJournal );
  }
}

void JournalView::printItem()
{
  if ( const KCalCore::Journal::Ptr j = CalendarSupport::journal( mJournal ) ) {
    KOCoreHelper helper;
    CalPrinter printer( this, mCalendar.data(), &helper, true );
    connect( this, SIGNAL(configChanged()), &printer, SLOT(updateConfig()) );

    Incidence::List selectedIncidences;
    selectedIncidences.append( j );

    //make sure to clear and then restore the view stylesheet, else the view
    //stylesheet is propagated to the child print dialog. see bug 303902
    const QString ss = styleSheet();
    setStyleSheet( QString() );
    printer.print( KOrg::CalPrinterBase::Incidence,
                   mDate, mDate, selectedIncidences );
    setStyleSheet( ss );
  }
}

void JournalView::setCalendar( const Akonadi::ETMCalendar::Ptr &cal )
{
  mCalendar = cal;
}

void JournalView::setDate( const QDate &date )
{
  mDate = date;
}

void JournalView::setJournal( const Akonadi::Item &journal )
{
  if ( !CalendarSupport::hasJournal( journal ) ) {
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

void JournalView::readJournal( const Akonadi::Item &j )
{
  int baseFontSize = KGlobalSettings::generalFont().pointSize();
  mJournal = j;
  const KCalCore::Journal::Ptr journal = CalendarSupport::journal( j );
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
  cursor.insertText( KCalUtils::IncidenceFormatter::dateTimeToString(
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

  if ( mCalendar ) {
    mEditButton->setEnabled( mCalendar->hasRight( j, Akonadi::Collection::CanChangeItem ) );
    mDeleteButton->setEnabled( mCalendar->hasRight( j, Akonadi::Collection::CanDeleteItem ) );
  }

}

#include "journalview.moc"
