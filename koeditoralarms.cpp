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

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "koeditoralarms.h"

#include <QLayout>
#include <QPushButton>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QTextEdit>
#include <QRadioButton>

#include <kurlrequester.h>
#include <klocale.h>

#include <kcal/alarm.h>
#include <kcal/incidence.h>

#include <kpimutils/email.h>

class AlarmListViewItem : public QTreeWidgetItem
{
  public:
    AlarmListViewItem( QTreeWidget *parent, KCal::Alarm *alarm );
    virtual ~AlarmListViewItem();
    KCal::Alarm *alarm() const
    {
      return mAlarm;
    }
    void construct();
    enum AlarmViewColumns {
      ColAlarmType=0,
      ColAlarmOffset,
      ColAlarmRepeat };
  protected:
    KCal::Alarm *mAlarm;
};

AlarmListViewItem::AlarmListViewItem( QTreeWidget *parent, KCal::Alarm *alarm )
    : QTreeWidgetItem( parent )
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
    QString type( i18nc( "@option unknown alarm type", "Unknown" ) );
    switch ( mAlarm->type() ) {
    case KCal::Alarm::Display:
      type = i18nc( "@option popup reminder dialog", "Reminder Dialog" );
      break;
    case KCal::Alarm::Procedure:
      type = i18nc( "@option run application or script", "Application/Script" );
      break;
    case KCal::Alarm::Email:
      type = i18nc( "@option send email reminder", "Email" );
      break;
    case KCal::Alarm::Audio:
      type = i18nc( "@option play a sound", "Audio" );
      break;
    default: break;
    }
    setText( ColAlarmType, type );

    // Alarm offset:
    KLocalizedString offsetstr;
    int offset = 0;
    if ( mAlarm->hasStartOffset() ) {
      offset = mAlarm->startOffset().asSeconds();
      if ( offset < 0 ) {
        offsetstr =
          ki18nc( "@item@intable N days/hours/minutes before/after the start/end",
                  "%1 before the start" );
        offset = -offset;
      } else {
        offsetstr =
          ki18nc( "@item@intable N days/hours/minutes before/after the start/end",
                  "%1 after the start" );
      }
    } else if ( mAlarm->hasEndOffset() ) {
      offset = mAlarm->endOffset().asSeconds();
      if ( offset < 0 ) {
        offsetstr =
          ki18nc( "@item@intable N days/hours/minutes before/after the start/end",
                  "%1 before the end" );
        offset = -offset;
      } else {
        offsetstr =
          ki18nc( "@item@intable N days/hours/minutes before/after the start/end",
                  "%1 after the end" );
      }
    }

    offset = offset / 60; // make minutes
    int useoffset = offset;

    if ( offset % ( 24 * 60 ) == 0 && offset > 0 ) { // divides evenly into days?
      useoffset = offset / ( 24 * 60 );
      offsetstr =
        offsetstr.subs( i18ncp( "@item@intable alarm offset specified in days",
                                "1 day", "%1 days", useoffset ) );
    } else if ( offset % 60 == 0 && offset > 0 ) { // divides evenly into hours?
      useoffset = offset / 60;
      offsetstr =
        offsetstr.subs( i18ncp( "@item@intable alarm offset specified in hours",
                                "1 hour", "%1 hours", useoffset ) );
    } else {
      useoffset = offset;
      offsetstr =
        offsetstr.subs( i18ncp( "@item@intable alarm offset specified in minutes",
                                "1 minute", "%1 minutes", useoffset ) );
    }
    setText( ColAlarmOffset, offsetstr.toString() );

    // Alarm repeat
    if ( mAlarm->repeatCount() > 0 ) {
      setText( ColAlarmRepeat, i18nc( "@item@intable yes, the alarm repeats", "Yes" ) );
    }
  }
}

KOEditorAlarms::KOEditorAlarms( KCal::Alarm::List *alarms, QWidget *parent )
  : KDialog( parent ), mAlarms( alarms ), mCurrentItem( 0 )
{
  setCaption( i18nc( "@title", "Edit Reminders" ) );
  setButtons( Ok | Apply | Cancel );
  setDefaultButton( Ok );
  QWidget *widget = new QWidget( this );
  mWidget.setupUi( widget );
  setMainWidget( widget );
  connect( mWidget.mAlarmList, SIGNAL(itemSelectionChanged()),
           SLOT(itemSelectionChanged()) );
  connect( mWidget.mAddButton, SIGNAL(clicked()), SLOT(slotAdd()) );
  connect( mWidget.mRemoveButton, SIGNAL(clicked()), SLOT(slotRemove()) );
  connect( mWidget.mDuplicateButton, SIGNAL(clicked()), SLOT(slotDuplicate()) );

  connect( mWidget.mAlarmOffset, SIGNAL(valueChanged(int)), SLOT(changed()) );
  connect( mWidget.mOffsetUnit, SIGNAL(activated(int)), SLOT(changed()) );
  connect( mWidget.mBeforeAfter, SIGNAL(activated(int)), SLOT(changed()) );
  connect( mWidget.mRepeats, SIGNAL(toggled(bool)), SLOT(changed()) );
  connect( mWidget.mRepeatCount, SIGNAL(valueChanged(int)), SLOT(changed()) );
  connect( mWidget.mRepeatInterval, SIGNAL(valueChanged(int)), SLOT(changed()) );

  connect( mWidget.mTypeDisplayRadio, SIGNAL(clicked()), SLOT(slotDisplayRadioClicked()) );
  connect( mWidget.mTypeSoundRadio, SIGNAL(clicked()), SLOT(slotSoundRadioClicked()) );
  connect( mWidget.mTypeAppRadio, SIGNAL(clicked()), SLOT(slotAppRadioClicked() ));
  connect( mWidget.mTypeEmailRadio, SIGNAL(clicked()), SLOT(slotEmailRadioClicked()) );

  connect( mWidget.mDisplayText, SIGNAL(textChanged()), SLOT(changed()) );
  connect( mWidget.mSoundFile, SIGNAL(textChanged(const QString&)), SLOT(changed()) );
  connect( mWidget.mApplication, SIGNAL(textChanged(const QString&)), SLOT(changed()) );
  connect( mWidget.mAppArguments, SIGNAL(textChanged(const QString&)), SLOT(changed()) );
  connect( mWidget.mEmailAddress, SIGNAL(textChanged(const QString&)), SLOT(changed()) );

  connect( mWidget.mRepeats, SIGNAL(toggled(bool)),
           mWidget.mIntervalLabel, SLOT(setEnabled(bool)) );
  connect( mWidget.mRepeats, SIGNAL(toggled(bool)),
           mWidget.mRepeatInterval, SLOT(setEnabled(bool)) );
  connect( mWidget.mRepeats, SIGNAL(toggled(bool)),
           mWidget.mHowOftenLabel, SLOT(setEnabled(bool)) );
  connect( mWidget.mRepeats, SIGNAL(toggled(bool)),
           mWidget.mRepeatCount, SLOT(setEnabled(bool)) );

  connect( mWidget.mEmailText, SIGNAL(textChanged()), SLOT(changed()) );
  connect( this, SIGNAL(okClicked()), SLOT(slotOk()) );
  connect( this, SIGNAL(applyClicked()), SLOT(slotApply()) );
  init();
  mWidget.mTypeEmailRadio->hide();
}

KOEditorAlarms::~KOEditorAlarms()
{
}

void KOEditorAlarms::slotDisplayRadioClicked()
{
  mWidget.mTypeStack->setCurrentIndex(0);
  changed();
}

void KOEditorAlarms::slotSoundRadioClicked()
{
  mWidget.mTypeStack->setCurrentIndex(1);
  changed();
}

void KOEditorAlarms::slotAppRadioClicked()
{
  mWidget.mTypeStack->setCurrentIndex(2);
  changed();
}

void KOEditorAlarms::slotEmailRadioClicked()
{
  mWidget.mTypeStack->setCurrentIndex(3);
  changed();
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
  if ( !alarm ) {
    return;
  }

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
  mWidget.mBeforeAfter->setCurrentIndex( beforeafterpos );

  offset = offset / 60; // make minutes
  int useoffset = offset;

  if ( offset % ( 24 * 60 ) == 0 && offset > 0 ) { // divides evenly into days?
    useoffset = offset / ( 24 * 60 );
    mWidget.mOffsetUnit->setCurrentIndex( 2 );
  } else if ( offset % 60 == 0 && offset > 0 ) { // divides evenly into hours?
    useoffset = offset / 60;
    mWidget.mOffsetUnit->setCurrentIndex( 1 );
  } else {
    useoffset = offset;
    mWidget.mOffsetUnit->setCurrentIndex( 0 );
  }
  mWidget.mAlarmOffset->setValue( useoffset );

  // Repeating
  mWidget.mRepeats->setChecked( alarm->repeatCount() > 0 );
  if ( alarm->repeatCount() > 0 ) {
    mWidget.mRepeatCount->setValue( alarm->repeatCount() );
    mWidget.mRepeatInterval->setValue( alarm->snoozeTime().asSeconds() / 60 ); // show as minutes
  }
  int id = 0;

  switch ( alarm->type() ) {
  case KCal::Alarm::Audio:
    mWidget.mTypeSoundRadio->setChecked( true );
    mWidget.mSoundFile->setUrl( alarm->audioFile() );
    id = 1;
    break;
  case KCal::Alarm::Procedure:
    mWidget.mTypeAppRadio->setChecked( true );
    mWidget.mApplication->setUrl( alarm->programFile() );
    mWidget.mAppArguments->setText( alarm->programArguments() );
    id = 2;
    break;
  case KCal::Alarm::Email:
  {
    mWidget.mTypeEmailRadio->setChecked( true );
    QList<KCal::Person> addresses = alarm->mailAddresses();
    QStringList add;
    for ( QList<KCal::Person>::ConstIterator it = addresses.constBegin();
          it != addresses.constEnd(); ++it ) {
      add << (*it).fullName();
    }
    mWidget.mEmailAddress->setText( add.join( ", " ) );
    mWidget.mEmailText->setPlainText( alarm->mailText() );
    id = 3;
    break;
  }
  case KCal::Alarm::Display:
  case KCal::Alarm::Invalid:
  default:
    mWidget.mTypeDisplayRadio->setChecked( true );
    mWidget.mDisplayText->setPlainText( alarm->text() );
    break;
  }

  mWidget.mTypeStack->setCurrentIndex( id );

  mInitializing = false;
}

void KOEditorAlarms::writeAlarm( KCal::Alarm *alarm )
{
  // Offsets
  int offset = mWidget.mAlarmOffset->value() * 60; // minutes
  int offsetunit = mWidget.mOffsetUnit->currentIndex();
  if ( offsetunit >= 1 ) {
    offset *= 60; // hours
  }
  if ( offsetunit >= 2 ) {
    offset *= 24; // days
  }
  if ( offsetunit >= 3 ) {
    offset *= 7; // weeks
  }

  int beforeafterpos = mWidget.mBeforeAfter->currentIndex();
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
  if ( mWidget.mRepeats->isChecked() ) {
    alarm->setRepeatCount( mWidget.mRepeatCount->value() );
    alarm->setSnoozeTime( mWidget.mRepeatInterval->value() * 60 ); // convert back to seconds
  } else {
    alarm->setRepeatCount( 0 );
  }

  if ( mWidget.mTypeSoundRadio->isChecked() ) { // Audio
    alarm->setAudioAlarm( mWidget.mSoundFile->url().path() );
  } else if ( mWidget.mTypeAppRadio->isChecked() ) { // Procedure
    alarm->setProcedureAlarm( mWidget.mApplication->url().path(),
                              mWidget.mAppArguments->text() );
  } else if ( mWidget.mTypeEmailRadio->isChecked() ) { // Email
    QStringList addresses = KPIMUtils::splitAddressList( mWidget.mEmailAddress->text() );
    QList<KCal::Person> add;
    for ( QStringList::Iterator it = addresses.begin(); it != addresses.end(); ++it ) {
      add << KCal::Person::fromFullName( *it );
    }
    // TODO: Add a subject line and possibilities for attachments
    alarm->setEmailAlarm( QString(), mWidget.mEmailText->toPlainText(), add );
  } else { // Display
    alarm->setDisplayAlarm( mWidget.mDisplayText->toPlainText() );
  }
}

void KOEditorAlarms::itemSelectionChanged()
{
  if ( mWidget.mAlarmList->currentItem() ) {
    AlarmListViewItem *item =
      dynamic_cast<AlarmListViewItem*>( mWidget.mAlarmList->currentItem() );
    mCurrentItem = item;
    mWidget.mTimeGroup->setEnabled( item );
    mWidget.mTypeGroup->setEnabled( item );
    if ( item ) {
      readAlarm( item->alarm() );
    }
  }
}

void KOEditorAlarms::slotApply()
{
  // copy the mAlarms list
  if ( mAlarms ) {
    mAlarms->clear();
    for ( int i = 0; i < mWidget.mAlarmList->topLevelItemCount(); ++i ) {
      AlarmListViewItem *item =
        dynamic_cast< AlarmListViewItem *>( mWidget.mAlarmList->topLevelItem( i ) );
      if ( item ) {
        mAlarms->append( new KCal::Alarm( *( item->alarm() ) ) );
      }
    }
  }
}

void KOEditorAlarms::slotOk()
{
  slotApply();
  accept();
}

void KOEditorAlarms::slotAdd()
{
  mCurrentItem = new AlarmListViewItem( mWidget.mAlarmList, 0 );
  mWidget.mAlarmList->setCurrentItem( mCurrentItem );
  changed();
//   selectionChanged( mCurrentItem );
}

void KOEditorAlarms::slotDuplicate()
{
  if ( mCurrentItem ) {
    mCurrentItem = new AlarmListViewItem( mWidget.mAlarmList, mCurrentItem->alarm() );
    mWidget.mAlarmList->setCurrentItem( mCurrentItem );
//     selectionChanged( mCurrentItem );
  }
}

void KOEditorAlarms::slotRemove()
{
  if ( mCurrentItem ) {
    delete mCurrentItem;
    mCurrentItem = dynamic_cast<AlarmListViewItem*>( mWidget.mAlarmList->currentItem() );
    mWidget.mAlarmList->setCurrentItem( mCurrentItem );

  }
}

void KOEditorAlarms::init()
{
  mInitializing = true;
  KCal::Alarm::List::ConstIterator it;
  for ( it = mAlarms->constBegin(); it != mAlarms->constEnd(); ++it ) {
    new AlarmListViewItem( mWidget.mAlarmList, *it );
  }
  if ( mWidget.mAlarmList->topLevelItemCount() > 0 ) {
    mWidget.mAlarmList->setCurrentItem( mWidget.mAlarmList->topLevelItem( 0 ) );
  }
  mInitializing = false;
}

#include "koeditoralarms.moc"
