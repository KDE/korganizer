/*
    This file is part of KOrganizer.

    Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
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

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "koeditorgeneraljournal.h"

#include <libkcal/journal.h>

#include <ktextedit.h>
#include <kdateedit.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <qgroupbox.h>
#include <qdatetime.h>
#include <qlabel.h>
#include <qlayout.h>


KOEditorGeneralJournal::KOEditorGeneralJournal( QObject *parent,
                                                const char *name )
  : QObject( parent, name )
{
}

KOEditorGeneralJournal::~KOEditorGeneralJournal()
{
}


void KOEditorGeneralJournal::initDate( QWidget *parent, QBoxLayout *topLayout )
{
  QBoxLayout *dateLayout = new QVBoxLayout(topLayout);

  QGroupBox *dateGroupBox = new QGroupBox( 1, QGroupBox::Horizontal,
                                           i18n("Date"), parent );
  dateLayout->addWidget( dateGroupBox );

  QFrame *dateBoxFrame = new QFrame( dateGroupBox );

  QGridLayout *layoutDateBox = new QGridLayout( dateBoxFrame, 1, 2 );
  layoutDateBox->setSpacing(topLayout->spacing());


  mDateLabel = new QLabel( i18n("&Date:"), dateBoxFrame);
  layoutDateBox->addWidget( mDateLabel, 0, 0);

  mDateEdit = new KDateEdit(dateBoxFrame);
  layoutDateBox->addWidget(mDateEdit,0,1);
  mDateLabel->setBuddy( mDateEdit );

  // date widgets are checked if they contain a valid date
  connect( mDateEdit, SIGNAL( dateChanged(QDate) ),
           SLOT( startDateChanged(QDate) ) );
}

void KOEditorGeneralJournal::setDate( const QDate &date )
{
//  kdDebug(5850) << "KOEditorGeneralJournal::setDate(): Date: " << date.toString() << endl;

  mDateEdit->setDate( date );
}

void KOEditorGeneralJournal::initDescription( QWidget *parent, QBoxLayout *topLayout )
{
  mDescriptionEdit = new KTextEdit( parent );
  mDescriptionEdit->append("");
  mDescriptionEdit->setReadOnly( false );
  mDescriptionEdit->setOverwriteMode( false );
  mDescriptionEdit->setWordWrap( KTextEdit::WidgetWidth );
  mDescriptionEdit->setTabChangesFocus( true );
  topLayout->addWidget( mDescriptionEdit );
}

void KOEditorGeneralJournal::setDefaults( const QDate &date )
{
  setDate( date );
}

void KOEditorGeneralJournal::readJournal( Journal *event, bool tmpl )
{
  if ( !tmpl ) {
    // the rest is for the events only
    setDate( event->dtStart().date() );
  }
  setDescription( event->description() );
}

void KOEditorGeneralJournal::writeJournal( Journal *event )
{
//  kdDebug(5850) << "KOEditorGeneralJournal::writeIncidence()" << endl;

  event->setDescription( mDescriptionEdit->text() );
  event->setFloats( true );
  QDateTime tmpDT( mDateEdit->date(), QTime(0,0,0) );
  event->setDtStart(tmpDT);

//  kdDebug(5850) << "KOEditorGeneralJournal::writeJournal() done" << endl;
}


void KOEditorGeneralJournal::setDescription( const QString &text )
{
  mDescriptionEdit->setText( text );
}

void KOEditorGeneralJournal::finishSetup()
{
  QWidget::setTabOrder( mDateEdit, mDescriptionEdit );
  mDescriptionEdit->setFocus();
}

bool KOEditorGeneralJournal::validateInput()
{
//  kdDebug(5850) << "KOEditorGeneralJournal::validateInput()" << endl;

  if (!mDateEdit->inputIsValid()) {
    KMessageBox::sorry( 0,
        i18n("Please specify a valid date, for example '%1'.")
        .arg( KGlobal::locale()->formatDate( QDate::currentDate() ) ) );
    return false;
  }

  return true;
}

#include "koeditorgeneraljournal.moc"
