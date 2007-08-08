/*
  This file is part of KOrganizer.

  Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
	large parts of code taken from KOEditorGeneralJournal.cpp:
  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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

#include "koeditorgeneraljournal.h"
#include "koprefs.h"
#include "koeditorgeneral.h"

#include <kcal/journal.h>

#include <ktextedit.h>
#include <kdateedit.h>
#include <ktimeedit.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>

#include <q3groupbox.h>
#include <QDateTime>
#include <QCheckBox>
#include <QLabel>
#include <QLayout>
#include <QHBoxLayout>
#include <QBoxLayout>


KOEditorGeneralJournal::KOEditorGeneralJournal( QObject *parent ) : QObject( parent )
{
}

KOEditorGeneralJournal::~KOEditorGeneralJournal()
{
}

void KOEditorGeneralJournal::initTitle( QWidget *parent, QBoxLayout *topLayout )
{
  QHBoxLayout *hbox = new QHBoxLayout();
  topLayout->addItem( hbox );

  QString whatsThis = i18n("Sets the title of this journal.");
  QLabel *summaryLabel = new QLabel( i18n("T&itle:"), parent );
  summaryLabel->setWhatsThis( whatsThis );
  QFont f = summaryLabel->font();
  f.setBold( true );
  summaryLabel->setFont( f );
  hbox->addWidget( summaryLabel );

  mSummaryEdit = new FocusLineEdit( parent );
  mSummaryEdit->setWhatsThis( whatsThis );
  summaryLabel->setBuddy( mSummaryEdit );
  hbox->addWidget( mSummaryEdit );
}


void KOEditorGeneralJournal::initDate( QWidget *parent, QBoxLayout *topLayout )
{
  QBoxLayout *dateLayout = new QHBoxLayout();
  topLayout->addItem( dateLayout );

  mDateLabel = new QLabel( i18n("&Date:"), parent );
  dateLayout->addWidget( mDateLabel );

  mDateEdit = new KDateEdit( parent );
  dateLayout->addWidget( mDateEdit );
  mDateLabel->setBuddy( mDateEdit );

  dateLayout->addStretch();

  mTimeCheckBox = new QCheckBox( i18n("&Time: "), parent );
  dateLayout->addWidget( mTimeCheckBox );

  mTimeEdit = new KTimeEdit( parent );
  dateLayout->addWidget( mTimeEdit );
  connect( mTimeCheckBox, SIGNAL(toggled(bool)),
           mTimeEdit, SLOT(setEnabled(bool)) );

  dateLayout->addStretch();
  setTime( QTime( -1, -1, -1 ) );
}

void KOEditorGeneralJournal::setDate( const QDate &date )
{
//  kDebug(5850) <<"KOEditorGeneralJournal::setDate(): Date:" << date.toString();

  mDateEdit->setDate( date );
}

void KOEditorGeneralJournal::setTime( const QTime &time )
{
kDebug()<<"KOEditorGeneralJournal::setTime, time="<<time.toString();
  bool validTime = time.isValid();
  mTimeCheckBox->setChecked( validTime );
  mTimeEdit->setEnabled( validTime );
  if ( validTime ) {
kDebug()<<"KOEditorGeneralJournal::setTime, time is valid";
    mTimeEdit->setTime( time );
  }
}

void KOEditorGeneralJournal::initDescription( QWidget *parent, QBoxLayout *topLayout )
{
  mDescriptionEdit = new KTextEdit( parent );
  mDescriptionEdit->append("");
  mDescriptionEdit->setReadOnly( false );
  mDescriptionEdit->setOverwriteMode( false );
  mDescriptionEdit->setLineWrapMode( KTextEdit::WidgetWidth );
  mDescriptionEdit->setTabChangesFocus( true );
  topLayout->addWidget( mDescriptionEdit );
}

void KOEditorGeneralJournal::readJournal( Journal *journal, bool tmpl )
{
  setSummary( journal->summary() );
  if ( !tmpl ) {
    setDate( journal->dtStart().date() );
    if ( !journal->floats() ) {
kDebug()<<"KOEditorGeneralJournal::readJournal, does not float, time="<<(journal->dtStart().time().toString());
      setTime( journal->dtStart().time() );
    } else {
kDebug()<<"KOEditorGeneralJournal::readJournal, does float";
      setTime( QTime( -1, -1, -1 ) );
    }
  }
  setDescription( journal->description() );
}

void KOEditorGeneralJournal::writeJournal( Journal *journal )
{
//  kDebug(5850) <<"KOEditorGeneralJournal::writeIncidence()";
  journal->setSummary( mSummaryEdit->text() );
  journal->setDescription( mDescriptionEdit->toPlainText() );

  KDateTime tmpDT( mDateEdit->date(), QTime(0,0,0), KOPrefs::instance()->timeSpec() );
  bool hasTime = mTimeCheckBox->isChecked();
  journal->setFloats( !hasTime );
  if ( hasTime ) {
    tmpDT.setTime( mTimeEdit->getTime() );
  }
  journal->setDtStart(tmpDT);

//  kDebug(5850) <<"KOEditorGeneralJournal::writeJournal() done";
}


void KOEditorGeneralJournal::setDescription( const QString &text )
{
  mDescriptionEdit->setPlainText( text );
}

void KOEditorGeneralJournal::setSummary( const QString &text )
{
  mSummaryEdit->setText( text );
}

void KOEditorGeneralJournal::finishSetup()
{
  QWidget::setTabOrder( mSummaryEdit, mDateEdit );
  QWidget::setTabOrder( mDateEdit, mTimeCheckBox );
  QWidget::setTabOrder( mTimeCheckBox, mTimeEdit );
  QWidget::setTabOrder( mTimeEdit, mDescriptionEdit );
  mSummaryEdit->setFocus();
}

bool KOEditorGeneralJournal::validateInput()
{
//  kDebug(5850) <<"KOEditorGeneralJournal::validateInput()";

  if (!mDateEdit->date().isValid()) {
    KMessageBox::sorry( 0,
        i18n("Please specify a valid date, for example '%1'.",
          KGlobal::locale()->formatDate( QDate::currentDate() ) ) );
    return false;
  }

  return true;
}

#include "koeditorgeneraljournal.moc"
