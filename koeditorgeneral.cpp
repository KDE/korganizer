/*
    This file is part of KOrganizer.
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


#include <qwidget.h>
#include <qtooltip.h>
#include <qlayout.h>
#include <qvbox.h>
#include <qbuttongroup.h>
#include <qvgroupbox.h>
#include <qwidgetstack.h>
#include <qdatetime.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qcombobox.h>

#include <kglobal.h>
#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <ksqueezedtextlabel.h>
#include <kstandarddirs.h>
#include <ktextedit.h>
#include <krestrictedline.h>

#include <libkcal/todo.h>
#include <libkcal/event.h>

#include <libkdepim/kdateedit.h>

#include "koprefs.h"
#include "koglobals.h"

#include "koeditorgeneral.h"
#include "koeditorgeneral.moc"

KOEditorGeneral::KOEditorGeneral(QObject* parent, const char* name) :
  QObject( parent, name)
{
}

KOEditorGeneral::~KOEditorGeneral()
{
}


FocusLineEdit::FocusLineEdit( QWidget *parent )
  : QLineEdit( parent ), mSkipFirst( true )
{
}

void FocusLineEdit::focusInEvent ( QFocusEvent *e )
{
  if ( !mSkipFirst ) {
    emit focusReceivedSignal();
  } else {
    mSkipFirst = false;
  }
  QLineEdit::focusInEvent( e );
}


void KOEditorGeneral::initHeader(QWidget *parent,QBoxLayout *topLayout)
{
  QGridLayout *headerLayout = new QGridLayout(topLayout);

#if 0
  mOwnerLabel = new QLabel(i18n("Owner:"),parent);
  headerLayout->addMultiCellWidget(mOwnerLabel,0,0,0,1);
#endif

  QLabel *summaryLabel = new QLabel(i18n("T&itle:"),parent);
  QFont f = summaryLabel->font();
  f.setBold( true );
  summaryLabel->setFont(f);
  headerLayout->addWidget(summaryLabel,1,0);

  mSummaryEdit = new FocusLineEdit(parent);
  connect( mSummaryEdit, SIGNAL( focusReceivedSignal() ),
           SIGNAL( focusReceivedSignal() ) );
  headerLayout->addWidget(mSummaryEdit,1,1);
  summaryLabel->setBuddy( mSummaryEdit );

  QLabel *locationLabel = new QLabel(i18n("&Location:"),parent);
  headerLayout->addWidget(locationLabel,2,0);

  mLocationEdit = new QLineEdit(parent);
  headerLayout->addWidget(mLocationEdit,2,1);
  locationLabel->setBuddy( mLocationEdit );
}

void KOEditorGeneral::initCategories(QWidget *parent, QBoxLayout *topLayout)
{
  QBoxLayout *categoriesLayout = new QHBoxLayout( topLayout );

  mCategoriesButton = new QPushButton(parent);
  mCategoriesButton->setText(i18n("Select Cate&gories..."));
  connect(mCategoriesButton,SIGNAL(clicked()),SIGNAL(openCategoryDialog()));
  categoriesLayout->addWidget(mCategoriesButton);

  mCategoriesLabel = new KSqueezedTextLabel(parent);
  mCategoriesLabel->setFrameStyle(QFrame::Panel|QFrame::Sunken);
  categoriesLayout->addWidget(mCategoriesLabel,1);
}

void KOEditorGeneral::initSecrecy(QWidget *parent, QBoxLayout *topLayout)
{
  QBoxLayout *secrecyLayout = new QHBoxLayout( topLayout );

  QLabel *secrecyLabel = new QLabel(i18n("Acc&ess:"),parent);
  secrecyLayout->addWidget(secrecyLabel);

  mSecrecyCombo = new QComboBox(parent);
  mSecrecyCombo->insertStringList(Incidence::secrecyList());
  secrecyLayout->addWidget(mSecrecyCombo);
  secrecyLabel->setBuddy( mSecrecyCombo );
}

void KOEditorGeneral::initDescription(QWidget *parent,QBoxLayout *topLayout)
{
  mDescriptionEdit = new KTextEdit(parent);
  mDescriptionEdit->append("");
  mDescriptionEdit->setReadOnly(false);
  mDescriptionEdit->setOverwriteMode(false);
  mDescriptionEdit->setWordWrap( KTextEdit::WidgetWidth );
  mDescriptionEdit->setTabChangesFocus( true );;
  topLayout->addWidget(mDescriptionEdit);
}

void KOEditorGeneral::initAlarm(QWidget *parent,QBoxLayout *topLayout)
{
  QBoxLayout *alarmLayout = new QHBoxLayout(topLayout);

  mAlarmBell = new QLabel(parent);
  mAlarmBell->setPixmap(KOGlobals::self()->smallIcon("bell"));
  alarmLayout->addWidget(mAlarmBell);

  mAlarmButton = new QCheckBox(i18n("&Reminder:"),parent);
  connect(mAlarmButton, SIGNAL(toggled(bool)), SLOT(enableAlarmEdit(bool)));
  alarmLayout->addWidget(mAlarmButton);

  mAlarmTimeEdit = new KRestrictedLine(parent, "alarmTimeEdit",
                  "1234567890");
  mAlarmTimeEdit->setText("");
  alarmLayout->addWidget(mAlarmTimeEdit);

  mAlarmIncrCombo = new QComboBox(false, parent);
  mAlarmIncrCombo->insertItem(i18n("minute(s)"));
  mAlarmIncrCombo->insertItem(i18n("hour(s)"));
  mAlarmIncrCombo->insertItem(i18n("day(s)"));
//  mAlarmIncrCombo->setMinimumHeight(20);
  alarmLayout->addWidget(mAlarmIncrCombo);

  mAlarmSoundButton = new QPushButton(parent);
  mAlarmSoundButton->setPixmap(KOGlobals::self()->smallIcon("playsound"));
  mAlarmSoundButton->setToggleButton(true);
  QToolTip::add(mAlarmSoundButton, i18n("No sound set"));
  connect(mAlarmSoundButton, SIGNAL(clicked()), SLOT(pickAlarmSound()));
  alarmLayout->addWidget(mAlarmSoundButton);

  mAlarmProgramButton = new QPushButton(parent);
  mAlarmProgramButton->setPixmap(KOGlobals::self()->smallIcon("runprog"));
  mAlarmProgramButton->setToggleButton(true);
  QToolTip::add(mAlarmProgramButton, i18n("No program set"));
  connect(mAlarmProgramButton, SIGNAL(clicked()), SLOT(pickAlarmProgram()));
  alarmLayout->addWidget(mAlarmProgramButton);

  if ( KOPrefs::instance()->mCompactDialogs ) {
    mAlarmSoundButton->hide();
    mAlarmProgramButton->hide();
  }
}

void KOEditorGeneral::pickAlarmSound()
{
  QString prefix = KGlobal::dirs()->findResourceDir("data", "korganizer/sounds/alert.wav");
  prefix += "/korganizer/sounds/alert.wav";
  if (!mAlarmSoundButton->isOn()) {
    mAlarmSound = "";
    QToolTip::remove(mAlarmSoundButton);
    QToolTip::add(mAlarmSoundButton, i18n("No sound set"));
  } else {
    QString fileName(KFileDialog::getOpenFileName(prefix,
                                                  i18n("*.wav|Wav Files"), 0));
    if (!fileName.isEmpty()) {
      mAlarmSound = fileName;
      QToolTip::remove(mAlarmSoundButton);
      QString dispStr = i18n("Playing '%1'").arg(fileName);
      QToolTip::add(mAlarmSoundButton, dispStr);
      mAlarmProgramButton->setOn(false);
    }
  }
  if (mAlarmSound.isEmpty())
    mAlarmSoundButton->setOn(false);
}

void KOEditorGeneral::pickAlarmProgram()
{
  if (!mAlarmProgramButton->isOn()) {
    mAlarmProgram = "";
    QToolTip::remove(mAlarmProgramButton);
    QToolTip::add(mAlarmProgramButton, i18n("No program set"));
  } else {
    QString fileName(KFileDialog::getOpenFileName(QString::null, QString::null, 0));
    if (!fileName.isEmpty()) {
      mAlarmProgram = fileName;
      QToolTip::remove(mAlarmProgramButton);
      QString dispStr = i18n("Running '%1'").arg(fileName);
      QToolTip::add(mAlarmProgramButton, dispStr);
      mAlarmSoundButton->setOn(false);
    }
  }
  if (mAlarmProgram.isEmpty())
    mAlarmProgramButton->setOn(false);
}



void KOEditorGeneral::enableAlarmEdit(bool enable)
{
  mAlarmTimeEdit->setEnabled(enable);
  mAlarmSoundButton->setEnabled(enable);
  mAlarmProgramButton->setEnabled(enable);
  mAlarmIncrCombo->setEnabled(enable);
}

void KOEditorGeneral::disableAlarmEdit(bool disable)
{
  enableAlarmEdit( !disable );
}

void KOEditorGeneral::enableAlarm( bool enable )
{
  enableAlarmEdit( enable );
}

void KOEditorGeneral::alarmDisable(bool disable)
{
  if (!disable) {
    mAlarmBell->setEnabled(true);
    mAlarmButton->setEnabled(true);
  } else {
    mAlarmBell->setEnabled(false);
    mAlarmButton->setEnabled(false);
    mAlarmButton->setChecked(false);
    mAlarmTimeEdit->setEnabled(false);
    mAlarmSoundButton->setEnabled(false);
    mAlarmProgramButton->setEnabled(false);
    mAlarmIncrCombo->setEnabled(false);
  }
}

void KOEditorGeneral::setCategories(const QString &str)
{
  mCategoriesLabel->setText(str);
  mCategories = str;
}

void KOEditorGeneral::setDefaults(bool allDay)
{
#if 0
  mOwnerLabel->setText(i18n("Owner: ") + KOPrefs::instance()->fullName());
#endif

  enableAlarmEdit( !allDay );

  // FIXME: Implement a KPrefsComboItem to solve this in a clean way.
  int alarmTime;
  int a[] = { 1,5,10,15,30 };
  int index = KOPrefs::instance()->mAlarmTime;
  if (index < 0 || index > 4) {
    alarmTime = 0;
  } else {
    alarmTime = a[index];
  }
  mAlarmTimeEdit->setText(QString::number(alarmTime));

  enableAlarmEdit( false );

  mSecrecyCombo->setCurrentItem(Incidence::SecrecyPublic);
}

void KOEditorGeneral::readIncidence(Incidence *event)
{
  mSummaryEdit->setText(event->summary());
  mLocationEdit->setText(event->location());

  mDescriptionEdit->setText(event->description());

#if 0
  // organizer information
  mOwnerLabel->setText(i18n("Owner: ") + event->organizer().fullName() );
#endif

  enableAlarmEdit( event->isAlarmEnabled() );

  if(!event->isAlarmEnabled()) {
    // FIXME: Implement a KPrefsComboItem to solve this in a clean way.
    int alarmTime;
    int a[] = { 1,5,10,15,30 };
    int index = KOPrefs::instance()->mAlarmTime;
    if (index < 0 || index > 4) {
      alarmTime = 0;
    } else {
      alarmTime = a[index];
    }
    mAlarmTimeEdit->setText(QString::number(alarmTime));
  }

  mSecrecyCombo->setCurrentItem(event->secrecy());

  // set up alarm stuff
  Alarm::List alarms = event->alarms();
  Alarm::List::ConstIterator it;
  for( it = alarms.begin(); it != alarms.end(); ++it ) {
    Alarm *alarm = *it;
    int offset;
    if ( alarm->hasTime() ) {
      QDateTime t = alarm->time();
      offset = event->dtStart().secsTo( t );
    } else {
      offset = alarm->startOffset().asSeconds();
    }
    offset = offset / -60; // make minutes
    int useoffset = offset;
    if (offset % (24*60) == 0) { // divides evenly into days?
      useoffset = offset / (24*60);
      mAlarmIncrCombo->setCurrentItem(2);
    } else if (offset % 60 == 0) { // divides evenly into hours?
      useoffset = offset / 60;
      mAlarmIncrCombo->setCurrentItem(1);
    }
    mAlarmTimeEdit->setText(QString::number( useoffset ));

    if (alarm->type() == Alarm::Procedure) {
      mAlarmProgram = alarm->programFile();
      mAlarmProgramButton->setOn(true);
      QString dispStr = i18n("Running '%1'").arg(mAlarmProgram);
      QToolTip::add(mAlarmProgramButton, dispStr);
    }
    else if (alarm->type() == Alarm::Audio) {
      mAlarmSound = alarm->audioFile();
      mAlarmSoundButton->setOn(true);
      QString dispStr = i18n("Playing '%1'").arg(mAlarmSound);
      QToolTip::add(mAlarmSoundButton, dispStr);
    }
    mAlarmButton->setChecked(alarm->enabled());
    enableAlarmEdit( alarm->enabled() );
// FIXME: Deal with multiple alarms
    break; // For now, stop after the first alarm
  }

  setCategories(event->categoriesStr());
}

void KOEditorGeneral::writeIncidence(Incidence *event)
{
//  kdDebug(5850) << "KOEditorGeneral::writeEvent()" << endl;

  event->setSummary(mSummaryEdit->text());
  event->setLocation(mLocationEdit->text());
  event->setDescription(mDescriptionEdit->text());
  event->setCategories(mCategories);
  event->setSecrecy(mSecrecyCombo->currentItem());

  // alarm stuff
  if (mAlarmButton->isChecked()) {
    if (event->alarms().count() == 0) event->newAlarm();
    Alarm::List alarms = event->alarms();
    Alarm::List::ConstIterator it;
    for( it = alarms.begin(); it != alarms.end(); ++it ) {
      Alarm *alarm = *it;
      alarm->setEnabled(true);

      QString tmpStr = mAlarmTimeEdit->text();
      int j = tmpStr.toInt() * -60;
      if (mAlarmIncrCombo->currentItem() == 1)
        j = j * 60;
      else if (mAlarmIncrCombo->currentItem() == 2)
        j = j * (60 * 24);
      alarm->setStartOffset( j );

      if (!mAlarmSound.isEmpty() && mAlarmSoundButton->isOn())
        alarm->setAudioAlarm(mAlarmSound);
      else
        alarm->setDisplayAlarm(QString::null);
      // FIXME: Make sure all alarm options are correctly set and don't erase other options!
      if (!mAlarmProgram.isEmpty() && mAlarmProgramButton->isOn())
        alarm->setProcedureAlarm(mAlarmProgram);

// FIXME: Deal with multiple alarms
      break; // For now, stop after the first alarm
    }
  } else {
    if ( !event->alarms().isEmpty() ) {
      Alarm *alarm = event->alarms().first();
      alarm->setEnabled(false);
      alarm->setType(Alarm::Invalid);
    }
  }
}

void KOEditorGeneral::setSummary( const QString &text )
{
  mSummaryEdit->setText( text );
}

void KOEditorGeneral::setDescription( const QString &text )
{
  mDescriptionEdit->setText( text );
}

QObject *KOEditorGeneral::typeAheadReceiver() const
{
  return mSummaryEdit;
}
