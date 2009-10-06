/*
  This file is part of KOrganizer.

  Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
	large parts of code taken from KOEditorGeneralJournal.cpp:
  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
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

#include "koeditorgeneraljournal.h"
#include "koprefs.h"

#include <libkdepim/kdateedit.h>
#include <libkdepim/ktimeedit.h>

#include <KCal/Journal>

#include <KLocale>
#include <KMessageBox>
#include <KRichTextWidget>
#include <KSqueezedTextLabel>

#include <QCheckBox>
#include <QLabel>
#include <QBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTimer>

KOEditorGeneralJournal::KOEditorGeneralJournal( KOrg::CalendarBase *calendar, QObject *parent )
  : KOEditorGeneral( calendar, parent )
{
  setType( "Journal" );
}

KOEditorGeneralJournal::~KOEditorGeneralJournal()
{
}

void KOEditorGeneralJournal::initTitle( QWidget *parent, QBoxLayout *topLayout )
{
  mParent = parent;

  QHBoxLayout *hbox = new QHBoxLayout();
  topLayout->addItem( hbox );

  QString whatsThis = i18n( "Sets the title of this journal." );
  QLabel *titleLabel = new QLabel( i18nc( "journal title", "T&itle:" ), parent );
  titleLabel->setWhatsThis( whatsThis );
  QFont f = titleLabel->font();
  f.setBold( true );
  titleLabel->setFont( f );
  hbox->addWidget( titleLabel );

  mTitleEdit = new FocusLineEdit( parent );
  connect( mTitleEdit, SIGNAL(focusReceivedSignal()),
           SIGNAL(focusReceivedSignal()) );
  QTimer::singleShot( 0, mTitleEdit, SLOT(setFocus()) );
  mTitleEdit->setWhatsThis( whatsThis );
  titleLabel->setBuddy( mTitleEdit );
  hbox->addWidget( mTitleEdit );
}

void KOEditorGeneralJournal::initDate( QWidget *parent, QBoxLayout *topLayout )
{
  QBoxLayout *dateLayout = new QHBoxLayout();
  topLayout->addItem( dateLayout );

  mDateLabel = new QLabel( i18n( "&Date:" ), parent );
  dateLayout->addWidget( mDateLabel );

  mDateEdit = new KPIM::KDateEdit( parent );
  dateLayout->addWidget( mDateEdit );
  mDateLabel->setBuddy( mDateEdit );

  dateLayout->addStretch();

  mTimeCheckBox = new QCheckBox( i18n( "&Time: " ), parent );
  dateLayout->addWidget( mTimeCheckBox );

  mTimeEdit = new KPIM::KTimeEdit( parent );
  dateLayout->addWidget( mTimeEdit );
  connect( mTimeCheckBox, SIGNAL(toggled(bool)), mTimeEdit, SLOT(setEnabled(bool)) );

  dateLayout->addStretch();
  setTime( QTime( -1, -1, -1 ) );
}

void KOEditorGeneralJournal::setDate( const QDate &date )
{
  mDateEdit->setDate( date );
}

void KOEditorGeneralJournal::setTime( const QTime &time )
{
  bool validTime = time.isValid();
  mTimeCheckBox->setChecked( validTime );
  mTimeEdit->setEnabled( validTime );
  if ( validTime ) {
    mTimeEdit->setTime( time );
  }
}

void KOEditorGeneralJournal::initCategories( QWidget *parent, QBoxLayout *topLayout )
{
  QBoxLayout *categoriesLayout = new QHBoxLayout();
  categoriesLayout->setSpacing( topLayout->spacing() );
  topLayout->addItem( categoriesLayout );

  QString whatsThis = i18n( "Allows you to select the categories that this journal belongs to." );

  mCategoriesButton = new QPushButton( parent );
  mCategoriesButton->setText( i18n( "Select Cate&gories..." ) );
  mCategoriesButton->setWhatsThis( whatsThis );
  connect( mCategoriesButton, SIGNAL(clicked()), SLOT(selectCategories()) );
  categoriesLayout->addWidget( mCategoriesButton );

  mCategoriesLabel = new KSqueezedTextLabel( parent );
  mCategoriesLabel->setWhatsThis( whatsThis );
  mCategoriesLabel->setFrameStyle( QFrame::Panel|QFrame::Sunken );
  categoriesLayout->addWidget( mCategoriesLabel, 1 );
}

void KOEditorGeneralJournal::readJournal( Journal *journal, bool tmpl )
{
  setSummary( journal->summary() );
  if ( !tmpl ) {
    KDateTime startDT = journal->dtStart();
    if ( startDT.isUtc() ) {
      startDT = startDT.toLocalZone();
    }
    setDate( startDT.date() );
    if ( !journal->allDay() ) {
      setTime( startDT.time() );
    } else {
      setTime( QTime( -1, -1, -1 ) );
    }
  }
  setDescription( journal->description(), journal->descriptionIsRich() );
  setCategories( journal->categories() );
}

void KOEditorGeneralJournal::fillJournal( Journal *journal )
{
  journal->setSummary( mTitleEdit->text() );
  if ( mRichDescription->isChecked() ) {
    journal->setDescription( mDescriptionEdit->toHtml(),
                             true );
  } else {
    journal->setDescription( mDescriptionEdit->toPlainText(),
                             false );
  }
  KDateTime tmpDT( mDateEdit->date(), QTime( 0, 0, 0 ), KOPrefs::instance()->timeSpec() );
  bool hasTime = mTimeCheckBox->isChecked();
  journal->setAllDay( !hasTime );
  if ( hasTime ) {
    tmpDT.setTime( mTimeEdit->getTime() );
  }
  journal->setDtStart( tmpDT );
  journal->setCategories( mCategories );
}

void KOEditorGeneralJournal::setSummary( const QString &text )
{
  mTitleEdit->setText( text );
}

void KOEditorGeneralJournal::finishSetup()
{
  QWidget::setTabOrder( mTitleEdit, mDateEdit );
  QWidget::setTabOrder( mDateEdit, mTimeCheckBox );
  QWidget::setTabOrder( mTimeCheckBox, mTimeEdit );
  QWidget::setTabOrder( mTimeEdit, mDescriptionEdit );
  QWidget::setTabOrder( mDescriptionEdit, mCategoriesButton );

  mTitleEdit->setFocus();
}

bool KOEditorGeneralJournal::validateInput()
{

  if ( !mDateEdit->date().isValid() ) {
    KMessageBox::sorry( mParent,
                        i18n( "Please specify a valid date, for example '%1'.",
                              KGlobal::locale()->formatDate( QDate::currentDate() ) ) );
    return false;
  }

  if ( mTitleEdit->text().isEmpty() ) {
    KMessageBox::sorry( mParent,
                        i18nc( "@info", "Please specify a journal title." ) );
    return false;
  }

  return true;
}

bool KOEditorGeneralJournal::setAlarmOffset( Alarm *alarm, int value ) const
{
  Q_UNUSED( alarm );
  Q_UNUSED( value );
  // journals don't have alarms
  return false;
}

#include "koeditorgeneraljournal.moc"
