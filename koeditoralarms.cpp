/*
    This file is part of KOrganizer.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include "koeditoralarms_base.h"
#include "koeditoralarms.h"

#include <qlayout.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qbuttongroup.h>
#include <qtextedit.h>
#include <qwidgetstack.h>
#include <qradiobutton.h>

#include <kurlrequester.h>
#include <klocale.h>
#include <kdebug.h>

#include <libkcal/alarm.h>
#include <libkcal/incidence.h>

#include <libemailfunctions/email.h>

class AlarmListViewItem : public QListViewItem
{
  public:
    AlarmListViewItem( QListView *parent, KCal::Alarm *alarm );
    virtual ~AlarmListViewItem();
    KCal::Alarm *alarm() const { return mAlarm; }
    void construct();
    enum AlarmViewColumns { ColAlarmType=0, ColAlarmOffset, ColAlarmRepeat };
  protected:
    KCal::Alarm *mAlarm;
};

AlarmListViewItem::AlarmListViewItem( QListView *parent, KCal::Alarm *alarm )
    : QListViewItem( parent )
{
  if ( alarm ) {
    mAlarm = new KCal::Alarm( *alarm );
  } else {
    mAlarm = new KCal::Alarm( 0 );
  }
  construct();
}

AlarmListViewItem::~AlarmListViewItem()
{
  delete mAlarm;
}

void AlarmListViewItem::construct()
{
  if ( mAlarm ) {
    // Alarm type:
    QString type;
    switch ( mAlarm->type() ) {
      case KCal::Alarm::Display:
        type = i18n("Reminder Dialog");
        break;
      case KCal::Alarm::Procedure:
        type = i18n("Program");
        break;
      case KCal::Alarm::Email:
        type = i18n("Email");
        break;
      case KCal::Alarm::Audio:
        type = i18n("Audio");
        break;
      default:
        type = i18n("Unknown");
        break;
    }
    setText( ColAlarmType, type );

    // Alarm offset:
    QString offsetstr;
    int offset = 0;
    if ( mAlarm->hasStartOffset() ) {
      offset = mAlarm->startOffset().asSeconds();
      if ( offset < 0 ) {
        offsetstr = i18n("N days/hours/minutes before/after the start/end", "%1 before the start");
        offset = -offset;
      } else {
        offsetstr = i18n("N days/hours/minutes before/after the start/end", "%1 after the start");
      }
    } else if ( mAlarm->hasEndOffset() ) {
      offset = mAlarm->endOffset().asSeconds();
      if ( offset < 0 ) {
        offsetstr = i18n("N days/hours/minutes before/after the start/end", "%1 before the end");
        offset = -offset;
      } else {
        offsetstr = i18n("N days/hours/minutes before/after the start/end", "%1 after the end");
      }
    }

    offset = offset / 60; // make minutes
    int useoffset = offset;

    if ( offset % (24*60) == 0 && offset>0 ) { // divides evenly into days?
      useoffset = offset / (24*60);
      offsetstr = offsetstr.arg( i18n("1 day", "%n days", useoffset ) );
    } else if (offset % 60 == 0 && offset>0 ) { // divides evenly into hours?
      useoffset = offset / 60;
      offsetstr = offsetstr.arg( i18n("1 hour", "%n hours", useoffset ) );
    } else {
      useoffset = offset;
      offsetstr = offsetstr.arg( i18n("1 minute", "%n minutes", useoffset ) );
    }
    setText( ColAlarmOffset, offsetstr );

    // Alarm repeat
    if ( mAlarm->repeatCount()>0 ) {
      setText( ColAlarmRepeat, i18n("Yes") );
    } else {
      setText( ColAlarmRepeat, i18n("No") );
    }
  }
}


KOEditorAlarms::KOEditorAlarms( KCal::Alarm::List *alarms, QWidget *parent,
                                const char *name )
  : KDialogBase( parent, name, true, i18n("Edit Reminders"), Ok | Cancel ), mAlarms( alarms ),mCurrentItem(0L)
{
  setMainWidget( mWidget = new KOEditorAlarms_base( this ) );
  mWidget->mAlarmList->setColumnWidthMode( 0, QListView::Maximum );
  mWidget->mAlarmList->setColumnWidthMode( 1, QListView::Maximum );
  connect( mWidget->mAlarmList, SIGNAL( selectionChanged( QListViewItem * ) ),
           SLOT( selectionChanged( QListViewItem * ) ) );
  connect( mWidget->mAddButton, SIGNAL( clicked() ), SLOT( slotAdd() ) );
  connect( mWidget->mRemoveButton, SIGNAL( clicked() ), SLOT( slotRemove() ) );
  connect( mWidget->mDuplicateButton, SIGNAL( clicked() ), SLOT( slotDuplicate() ) );

  connect( mWidget->mAlarmOffset, SIGNAL( valueChanged( int ) ), SLOT( changed() ) );
  connect( mWidget->mOffsetUnit, SIGNAL( activated( int ) ), SLOT( changed() ) );
  connect( mWidget->mBeforeAfter, SIGNAL( activated( int ) ), SLOT( changed() ) );
  connect( mWidget->mRepeats, SIGNAL( toggled( bool ) ), SLOT( changed() ) );
  connect( mWidget->mRepeatCount, SIGNAL( valueChanged( int ) ), SLOT( changed() ) );
  connect( mWidget->mRepeatInterval, SIGNAL( valueChanged( int ) ), SLOT( changed() ) );
  connect( mWidget->mAlarmType, SIGNAL(clicked(int)), SLOT( changed() ) );
  connect( mWidget->mDisplayText, SIGNAL( textChanged() ), SLOT( changed() ) );
  connect( mWidget->mSoundFile, SIGNAL( textChanged( const QString & ) ), SLOT( changed() ) );
  connect( mWidget->mApplication, SIGNAL( textChanged( const QString & ) ), SLOT( changed() ) );
  connect( mWidget->mAppArguments, SIGNAL( textChanged( const QString & ) ), SLOT( changed() ) );
  connect( mWidget->mEmailAddress, SIGNAL( textChanged( const QString & ) ), SLOT( changed() ) );
  connect( mWidget->mEmailText, SIGNAL( textChanged() ), SLOT( changed() ) );

  init();
  mWidget->mTypeEmailRadio->hide();
}

KOEditorAlarms::~KOEditorAlarms()
{
}

void KOEditorAlarms::changed()
{
  if ( !mInitializing && mCurrentItem ) {
    writeAlarm( mCurrentItem->alarm() );
    mCurrentItem->construct();
  }
}

void KOEditorAlarms::readAlarm( KCal::Alarm *alarm )
{
  if ( !alarm ) return;

  mInitializing = true;

  // Offsets
  int offset;
  int beforeafterpos = 0;
  if ( alarm->hasEndOffset() ) {
    beforeafterpos = 2;
    offset = alarm->endOffset().asSeconds();
  } else {
    // TODO: Also allow alarms at fixed times, not relative to start/end
    offset = alarm->startOffset().asSeconds();
  }
  // Negative offset means before the start/end...
  if ( offset < 0 ) {
    offset = -offset;
  } else {
    ++beforeafterpos;
  }
  mWidget->mBeforeAfter->setCurrentItem( beforeafterpos );

  offset = offset / 60; // make minutes
  int useoffset = offset;

  if ( offset % (24*60) == 0 && offset>0 ) { // divides evenly into days?
    useoffset = offset / (24*60);
    mWidget->mOffsetUnit->setCurrentItem( 2 );
  } else if (offset % 60 == 0 && offset>0 ) { // divides evenly into hours?
    useoffset = offset / 60;
    mWidget->mOffsetUnit->setCurrentItem( 1 );
  } else {
    useoffset = offset;
    mWidget->mOffsetUnit->setCurrentItem( 0 );
  }
  mWidget->mAlarmOffset->setValue( useoffset );


  // Repeating
  mWidget->mRepeats->setChecked( alarm->repeatCount()>0 );
  if ( alarm->repeatCount()>0 ) {
    mWidget->mRepeatCount->setValue( alarm->repeatCount() );
    mWidget->mRepeatInterval->setValue( alarm->snoozeTime() );
  }

  switch ( alarm->type() ) {
    case KCal::Alarm::Audio:
        mWidget->mAlarmType->setButton( 1 );
        mWidget->mSoundFile->setURL( alarm->audioFile() );
        break;
    case KCal::Alarm::Procedure:
        mWidget->mAlarmType->setButton( 2 );
        mWidget->mApplication->setURL( alarm->programFile() );
        mWidget->mAppArguments->setText( alarm->programArguments() );
        break;
    case KCal::Alarm::Email: {
        mWidget->mAlarmType->setButton( 3 );
        QValueList<KCal::Person> addresses = alarm->mailAddresses();
        QStringList add;
        for ( QValueList<KCal::Person>::ConstIterator it = addresses.begin();
              it != addresses.end(); ++it ) {
          add << (*it).fullName();
        }
        mWidget->mEmailAddress->setText( add.join(", ") );
        mWidget->mEmailText->setText( alarm->mailText() );
        break;}
    case KCal::Alarm::Display:
    case KCal::Alarm::Invalid:
    default:
        mWidget->mAlarmType->setButton( 0 );
        mWidget->mDisplayText->setText( alarm->text() );
        break;
  }

  mWidget->mTypeStack->raiseWidget( mWidget->mAlarmType->selectedId() );

  mInitializing = false;
}

void KOEditorAlarms::writeAlarm( KCal::Alarm *alarm )
{
  // Offsets
  int offset = mWidget->mAlarmOffset->value()*60; // minutes
  int offsetunit = mWidget->mOffsetUnit->currentItem();
  if ( offsetunit >= 1 ) offset *= 60; // hours
  if ( offsetunit >= 2 ) offset *= 24; // days
  if ( offsetunit >= 3 ) offset *= 7; // weeks

  int beforeafterpos = mWidget->mBeforeAfter->currentItem();
  if ( beforeafterpos % 2 == 0 ) { // before -> negative
    offset = -offset;
  }

  // TODO: Add possibility to specify a given time for the reminder
  if ( beforeafterpos / 2 == 0 ) { // start offset
    alarm->setStartOffset( KCal::Duration( offset ) );
  } else {
    alarm->setEndOffset( KCal::Duration( offset ) );
  }

  // Repeating
  if ( mWidget->mRepeats->isChecked() ) {
    alarm->setRepeatCount( mWidget->mRepeatCount->value() );
    alarm->setSnoozeTime( mWidget->mRepeatInterval->value() );
  } else {
    alarm->setRepeatCount( 0 );
  }

  switch ( mWidget->mAlarmType->selectedId() ) {
    case 1: // Audio
        alarm->setAudioAlarm( mWidget->mSoundFile->url() );
        break;
    case 2: // Procedure
        alarm->setProcedureAlarm( mWidget->mApplication->url(), mWidget->mAppArguments->text() );
        break;
    case 3: { // Email
        QStringList addresses = KPIM::splitEmailAddrList( mWidget->mEmailAddress->text() );
        QValueList<KCal::Person> add;
        for ( QStringList::Iterator it = addresses.begin(); it != addresses.end();
              ++it ) {
          add << KCal::Person( *it );
        }
        // TODO: Add a subject line and possibilities for attachments
        alarm->setEmailAlarm( QString::null, mWidget->mEmailText->text(),
                              add );
        break; }
    case 0: // Display
    default:
        alarm->setDisplayAlarm( mWidget->mDisplayText->text() );
        break;
  }
}

void KOEditorAlarms::selectionChanged( QListViewItem *listviewitem )
{
  AlarmListViewItem *item = dynamic_cast<AlarmListViewItem*>(listviewitem);
  mCurrentItem = item;
  mWidget->mTimeGroup->setEnabled( item );
  mWidget->mTypeGroup->setEnabled( item );
  if ( item ) {
    readAlarm( item->alarm() );
  }
}

void KOEditorAlarms::slotOk()
{
  // copy the mAlarms list
  if ( mAlarms ) {
    mAlarms->clear();
    QListViewItemIterator it( mWidget->mAlarmList );
    while ( it.current() ) {
      AlarmListViewItem *item = dynamic_cast<AlarmListViewItem*>(*it);
      if ( item ) {
        mAlarms->append( new KCal::Alarm( *(item->alarm()) ) );
      }
      ++it;
    }
  }
  accept();
}

void KOEditorAlarms::slotAdd()
{
  mCurrentItem = new AlarmListViewItem( mWidget->mAlarmList, 0 );
  mWidget->mAlarmList->setCurrentItem( mCurrentItem );
//   selectionChanged( mCurrentItem );
}

void KOEditorAlarms::slotDuplicate()
{
  if ( mCurrentItem ) {
    mCurrentItem = new AlarmListViewItem( mWidget->mAlarmList, mCurrentItem->alarm() );
    mWidget->mAlarmList->setCurrentItem( mCurrentItem );
//     selectionChanged( mCurrentItem );
  }
}

void KOEditorAlarms::slotRemove()
{
  if ( mCurrentItem ) {
    delete mCurrentItem;
    mCurrentItem = dynamic_cast<AlarmListViewItem*>( mWidget->mAlarmList->currentItem() );
    mWidget->mAlarmList->setSelected( mCurrentItem, true );

  }
}

void KOEditorAlarms::init()
{
  mInitializing = true;
  KCal::Alarm::List::ConstIterator it;
  for ( it = mAlarms->begin(); it != mAlarms->end(); ++it ) {
    new AlarmListViewItem( mWidget->mAlarmList, *it );
  }
  mWidget->mAlarmList->setSelected( mWidget->mAlarmList->firstChild(), true );
  mInitializing = false;
}

#include "koeditoralarms.moc"
