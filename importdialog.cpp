/*
    This file is part of KOrganizer.

    Copyright (c) 2003,2004 Cornelius Schumacher <schumacher@kde.org>

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

#include "importdialog.h"

#include "kocore.h"
#include "koprefs.h"
#include "korganizer.h"

#include <libkcal/calendarresources.h>
#include <libkcal/resourcelocal.h>
#include <libkcal/resourceremote.h>

#include <klocale.h>

#include <qlabel.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>

using namespace KCal;

ImportDialog::ImportDialog( const KURL &url, QWidget *parent )
  : KDialogBase( Plain, i18n("Import Calendar"), Ok | Cancel, Ok, parent,
                 0, true, true ),
    mUrl( url )
{
  QFrame *topFrame = plainPage();
  QVBoxLayout *topLayout = new QVBoxLayout( topFrame, 0, spacingHint() );

  QString txt = i18n("Import calendar at '%1' into KOrganizer.")
                .arg( mUrl.prettyURL() );

  topLayout->addWidget( new QLabel( txt, topFrame ) );

  QButtonGroup *radioBox = new QButtonGroup( 1, Horizontal, topFrame );
  radioBox->setFlat( true );
  topLayout->addWidget( radioBox );

  mAddButton = new QRadioButton( i18n("Add as new calendar"), radioBox );
  
  mMergeButton = new QRadioButton( i18n("Merge into existing calendar"),
                                   radioBox );

  mOpenButton = new QRadioButton( i18n("Open in separate window"), radioBox );

  mAddButton->setChecked( true );
}

ImportDialog::~ImportDialog()
{
}

void ImportDialog::slotOk()
{
  kdDebug() << "Adding resource for url '" << mUrl << "'" << endl;

  if ( mAddButton->isChecked() ) {
    CalendarResources *cr = KOCore::self()->calendarResources();

    CalendarResourceManager *manager = cr->resourceManager();

    ResourceCalendar *resource = 0;

    QString name;

    kdDebug() << "URL: " << mUrl << endl;
    if ( mUrl.isLocalFile() ) {
      kdDebug() << "Local Resource" << endl;
      resource = new ResourceLocal( mUrl.path() );
      resource->setTimeZoneId( KOPrefs::instance()->mTimeZoneId );
      name = mUrl.path();
    } else {
      kdDebug() << "Remote Resource" << endl;
      resource = new ResourceRemote( mUrl );
      resource->setTimeZoneId( KOPrefs::instance()->mTimeZoneId );
      name = mUrl.prettyURL();
    }

    if ( resource ) {
      resource->setResourceName( name );
      manager->add( resource );
    }

    emit dialogFinished( this );
  } else if ( mMergeButton->isChecked() ) {
    mMergeResource = new ResourceRemote( mUrl );
    mMergeResource->setTimeZoneId( KOPrefs::instance()->mTimeZoneId );
    connect( mMergeResource, SIGNAL( resourceLoaded( ResourceCalendar * ) ),
             SLOT( mergeResource( ResourceCalendar * ) ) );
    mMergeResource->open();
    mMergeResource->load();
  } else if ( mOpenButton->isChecked() ) {
    KOrganizer *korg = new KOrganizer;
    korg->init( true );
    korg->topLevelWidget()->show();
    korg->openURL( mUrl );
    emit dialogFinished( this );
  } else {
    kdError() << "ImportDialog: internal error." << endl;
    emit dialogFinished( this );
  }
  
  accept();
}

void ImportDialog::mergeResource( ResourceCalendar * )
{
  Calendar *cal = KOCore::self()->calendarResources();

  Event::List events = mMergeResource->rawEvents();
  Event::List::ConstIterator it;
  for( it = events.begin(); it != events.end(); ++it ) {
    Event *event = *it;
    Event *existingEvent = cal->event( event->uid() );
    if ( !existingEvent ) {
      cal->addEvent( event->clone() );
    } else {
      *existingEvent = *event;
    }
  }
  
  Todo::List todos = mMergeResource->rawTodos();
  Todo::List::ConstIterator it2;
  for( it2 = todos.begin(); it2 != todos.end(); ++it2 ) {
    Todo *todo = *it2;
    Todo *existingTodo = cal->todo( todo->uid() );
    if ( !existingTodo ) {
      cal->addTodo( todo->clone() );
    } else {
      *existingTodo = *todo;
    }
  }
  
  Journal::List journals = mMergeResource->journals();
  Journal::List::ConstIterator it3;
  for( it3 = journals.begin(); it3 != journals.end(); ++it3 ) {
    Journal *journal = *it3;
    Journal *existingJournal = cal->journal( journal->uid() );
    if ( !existingJournal ) {
      cal->addJournal( journal->clone() );
    } else {
      *existingJournal = *journal;
    }
  }

  emit dialogFinished( this );
}

#include "importdialog.moc"
