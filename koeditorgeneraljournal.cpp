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
#include "koeditorgeneral.h"
#include "koglobals.h"

#include <libkdepim/categoryselectdialog.h>

#include <kcal/journal.h>

#include <ktextedit.h>
#include <kdateedit.h>
#include <ktimeedit.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <KSqueezedTextLabel>

#include <q3groupbox.h>
#include <QDateTime>
#include <QCheckBox>
#include <QLabel>
#include <QLayout>
#include <QHBoxLayout>
#include <QBoxLayout>
#include <QPushButton>

KOEditorGeneralJournal::KOEditorGeneralJournal( QObject *parent,
                                                const char* name ) :
    KOEditorGeneral( parent, name )
{
  setObjectName( name );
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

  mDateEdit = new KPIM::KDateEdit( parent );
  dateLayout->addWidget( mDateEdit );
  mDateLabel->setBuddy( mDateEdit );

  dateLayout->addStretch();

  mTimeCheckBox = new QCheckBox( i18n("&Time: "), parent );
  dateLayout->addWidget( mTimeCheckBox );

  mTimeEdit = new KPIM::KTimeEdit( parent );
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

void KOEditorGeneralJournal::initCategories(QWidget *parent, QBoxLayout *topLayout)
{
  QBoxLayout *categoriesLayout = new QHBoxLayout();
  categoriesLayout->setSpacing( topLayout->spacing() );
  topLayout->addItem( categoriesLayout );

  QString whatsThis = i18n("Allows you to select the categories that this "
      "journal belongs to.");

  mCategoriesButton = new QPushButton(parent);
  mCategoriesButton->setText(i18n("Select Cate&gories..."));
  mCategoriesButton->setWhatsThis( whatsThis );
  connect(mCategoriesButton,SIGNAL(clicked()),SLOT(selectCategories()));
  categoriesLayout->addWidget(mCategoriesButton);

  mCategoriesLabel = new KSqueezedTextLabel(parent);
  mCategoriesLabel->setWhatsThis( whatsThis );
  mCategoriesLabel->setFrameStyle(QFrame::Panel|QFrame::Sunken);
  categoriesLayout->addWidget(mCategoriesLabel,1);
}

void KOEditorGeneralJournal::setCategories( const QStringList &categories )
{
  mCategoriesLabel->setText( categories.join(",") );
  mCategories = categories;
}

void KOEditorGeneralJournal::selectCategories()
{
  KPIM::CategorySelectDialog *categoryDialog = new KPIM::CategorySelectDialog( KOPrefs::instance(), mCategoriesButton  );
  KOGlobals::fitDialogToScreen( categoryDialog );
  categoryDialog->setSelected( mCategories );

  connect(categoryDialog, SIGNAL(editCategories()), this, SIGNAL(openCategoryDialog()));

  if ( categoryDialog->exec() ) {
    setCategories( categoryDialog->selectedCategories() );
  }
  delete categoryDialog;
}

void KOEditorGeneralJournal::readJournal( Journal *journal, bool tmpl )
{
  setSummary( journal->summary() );
  if ( !tmpl ) {
    setDate( journal->dtStart().date() );
    if ( !journal->allDay() ) {
kDebug()<<"KOEditorGeneralJournal::readJournal, is not all-day, time="<<(journal->dtStart().time().toString());
      setTime( journal->dtStart().time() );
    } else {
kDebug()<<"KOEditorGeneralJournal::readJournal, is an all-day";
      setTime( QTime( -1, -1, -1 ) );
    }
  }
  setDescription( journal->description(), journal->descriptionIsRich() );
  setCategories( journal->categories() );
}

void KOEditorGeneralJournal::writeJournal( Journal *journal )
{
//  kDebug(5850) <<"KOEditorGeneralJournal::writeIncidence()";
  journal->setSummary( mSummaryEdit->text() );
  if ( mHtmlCheckBox->isChecked() ) {
    journal->setDescription( mDescriptionEdit->toHtml(), true );
  }
  else {
    journal->setDescription( mDescriptionEdit->toPlainText(), false );
  }
  KDateTime tmpDT( mDateEdit->date(), QTime(0,0,0), KOPrefs::instance()->timeSpec() );
  bool hasTime = mTimeCheckBox->isChecked();
  journal->setAllDay( !hasTime );
  if ( hasTime ) {
    tmpDT.setTime( mTimeEdit->getTime() );
  }
  journal->setDtStart(tmpDT);
  journal->setCategories( mCategories );

//  kDebug(5850) <<"KOEditorGeneralJournal::writeJournal() done";
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
