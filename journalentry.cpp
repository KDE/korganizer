// $Id$
//
// Journal Entry

#include <qlabel.h>
#include <qmultilineedit.h>
#include <qlayout.h>

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>

#include "journal.h"
#include "calendar.h"

#include "journalentry.h"
#include "journalentry.moc"

JournalEntry::JournalEntry(Calendar *calendar,QWidget *parent) :
  QFrame(parent)
{
  mCalendar = calendar;
  mJournal = 0;
  mDirty = false;

  mTitleLabel = new QLabel("Title",this);
  mTitleLabel->setMargin(2);
  mTitleLabel->setAlignment(AlignCenter);
  
  mEditor = new QMultiLineEdit(this);
  connect(mEditor,SIGNAL(textChanged()),SLOT(setDirty()));
  
  QBoxLayout *topLayout = new QVBoxLayout(this);
  topLayout->addWidget(mTitleLabel);
  topLayout->addWidget(mEditor);
  
  mEditor->installEventFilter(this);
}

JournalEntry::~JournalEntry()
{
}

void JournalEntry::setDate(const QDate &date)
{
  writeJournal();

  mTitleLabel->setText(KGlobal::locale()->formatDate(date));
  
  mDate = date;
}

void JournalEntry::setJournal(Journal *journal)
{
  writeJournal();

  mJournal = journal;
  
  mEditor->setText(mJournal->description());

  mDirty = false;
}

Journal *JournalEntry::journal() const
{
  return mJournal;
}

void JournalEntry::setDirty()
{
  mDirty = true;
//  kdDebug() << "JournalEntry::setDirty()" << endl;
}

void JournalEntry::clear()
{
  mJournal = 0;
  mEditor->setText("");
}

bool JournalEntry::eventFilter( QObject *o, QEvent *e )
{
//  kdDebug() << "JournalEntry::event received " << e->type() << endl;

  if ( e->type() == QEvent::FocusOut ) {
    writeJournal();
  }
  return QFrame::eventFilter( o, e );    // standard event processing
}

void JournalEntry::writeJournal()
{
//  kdDebug() << "JournalEntry::writeJournal()" << endl;

  if (!mDirty) return;
 
  if (mEditor->text().isEmpty()) return;
 
//  kdDebug() << "JournalEntry::writeJournal()..." << endl;
  
  if (!mJournal) {
    mJournal = new Journal;
    mJournal->setDtStart(QDateTime(mDate,QTime(0,0,0)));
    mCalendar->addJournal(mJournal);
  }

  mJournal->setDescription(mEditor->text());

  mDirty = false;
}

void JournalEntry::flushEntry()
{
  if (!mDirty) return;
  
  writeJournal();
}
