/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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


#include <qwidget.h>
#include <qtooltip.h>
#include <qlayout.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qbuttongroup.h>
#include <qvgroupbox.h>
#include <qwidgetstack.h>
#include <qdatetime.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qwhatsthis.h>

#include <kglobal.h>
#include <kdialog.h>
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
#include <libkdepim/categoryselectdialog.h>

#include "koprefs.h"
#include "koglobals.h"

#include "koeditorgeneral.h"
#include "koeditoralarms.h"
#include "koeditorattachments.h"
#include "koeditorgeneral.moc"
#include "kohelper.h"

KOEditorGeneral::KOEditorGeneral(QObject* parent, const char* name) :
  QObject( parent, name ), mAttachments(0)
{
  mAlarmList.setAutoDelete( true );
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


void KOEditorGeneral::initHeader( QWidget *parent,QBoxLayout *topLayout)
{
  QGridLayout *headerLayout = new QGridLayout();
  headerLayout->setSpacing( topLayout->spacing() );
  topLayout->addLayout( headerLayout );

#if 0
  mOwnerLabel = new QLabel(i18n("Owner:"),parent);
  headerLayout->addMultiCellWidget(mOwnerLabel,0,0,0,1);
#endif

  QString whatsThis = i18n("Sets the Title of this event or to-do.");
  QLabel *summaryLabel = new QLabel( i18n("T&itle:"), parent );
  QWhatsThis::add( summaryLabel, whatsThis );
  QFont f = summaryLabel->font();
  f.setBold( true );
  summaryLabel->setFont(f);
  headerLayout->addWidget(summaryLabel,1,0);

  mSummaryEdit = new FocusLineEdit( parent );
  QWhatsThis::add( mSummaryEdit, whatsThis );
  connect( mSummaryEdit, SIGNAL( focusReceivedSignal() ),
           SIGNAL( focusReceivedSignal() ) );
  headerLayout->addWidget(mSummaryEdit,1,1);
  summaryLabel->setBuddy( mSummaryEdit );

  mAttendeeSummaryLabel = new QLabel( parent );
  updateAttendeeSummary( 0 );
  headerLayout->addWidget( mAttendeeSummaryLabel, 1, 2 );

  whatsThis = i18n("Sets where the event or to-do will take place.");
  QLabel *locationLabel = new QLabel( i18n("&Location:"), parent );
  QWhatsThis::add( locationLabel, whatsThis );
  headerLayout->addWidget(locationLabel,2,0);

  mLocationEdit = new QLineEdit( parent );
  QWhatsThis::add( mLocationEdit, whatsThis );
  headerLayout->addMultiCellWidget( mLocationEdit, 2, 2, 1, 2 );
  locationLabel->setBuddy( mLocationEdit );

  QBoxLayout *thirdLineLayout = new QHBoxLayout();
  headerLayout->addMultiCellLayout( thirdLineLayout, 3, 3, 0, 2 );

  mResourceLabel = new QLabel( parent );
  mResourceLabel->hide();
  thirdLineLayout->addWidget( mResourceLabel );

  whatsThis = i18n("Allows you to select the categories that this event or to-do belongs to.");
  QLabel *categoriesLabel = new QLabel( i18n("Categories:"), parent );
  QWhatsThis::add( categoriesLabel, whatsThis );
  thirdLineLayout->addWidget( categoriesLabel );
  mCategoriesLabel = new KSqueezedTextLabel( parent );
  QWhatsThis::add( mCategoriesLabel, whatsThis );
  mCategoriesLabel->setFrameStyle(QFrame::Panel|QFrame::Sunken);
  thirdLineLayout->addWidget( mCategoriesLabel );

  mCategoriesButton = new QPushButton( parent );
  mCategoriesButton->setText(i18n("&Select..."));
  QWhatsThis::add( mCategoriesButton, whatsThis );
  connect(mCategoriesButton,SIGNAL(clicked()),SLOT(selectCategories()));
  thirdLineLayout->addWidget( mCategoriesButton );
}

void KOEditorGeneral::initSecrecy(QWidget *parent, QBoxLayout *topLayout)
{
  QBoxLayout *secrecyLayout = new QHBoxLayout( topLayout );

  QLabel *secrecyLabel = new QLabel(i18n("Acc&ess:"),parent);
  QString whatsThis = i18n("Sets whether the access to this event or to-do "
  			   "is restricted. Please note that KOrganizer "
			   "currently does not use this setting, so the "
			   "implementation of the restrictions will depend "
			   "on the groupware server. This means that events "
			   "or to-dos marked as private or confidential may "
			   "be visible to others.");
  QWhatsThis::add( secrecyLabel, whatsThis );
  secrecyLayout->addWidget(secrecyLabel);

  mSecrecyCombo = new QComboBox(parent);
  QWhatsThis::add( mSecrecyCombo, whatsThis );
  mSecrecyCombo->insertStringList(Incidence::secrecyList());
  secrecyLayout->addWidget(mSecrecyCombo);
  secrecyLabel->setBuddy( mSecrecyCombo );
}

void KOEditorGeneral::initDescription(QWidget *parent,QBoxLayout *topLayout)
{
  mDescriptionEdit = new KTextEdit(parent);
  QWhatsThis::add( mDescriptionEdit,
		   i18n("Sets the description for this event or to-do. This "
			"will be displayed in a reminder if one is set, "
			"as well as in a tooltip when you hover over the "
			"event.") );
  mDescriptionEdit->append("");
  mDescriptionEdit->setReadOnly(false);
  mDescriptionEdit->setOverwriteMode(false);
  mDescriptionEdit->setWordWrap( KTextEdit::WidgetWidth );
  mDescriptionEdit->setTabChangesFocus( true );;
  topLayout->addWidget(mDescriptionEdit, 4);
}

void KOEditorGeneral::initAlarm(QWidget *parent,QBoxLayout *topLayout)
{
  QBoxLayout *alarmLayout = new QHBoxLayout(topLayout);

  mAlarmBell = new QLabel(parent);
  mAlarmBell->setPixmap(KOGlobals::self()->smallIcon("bell"));
  alarmLayout->addWidget( mAlarmBell );


  mAlarmStack = new QWidgetStack( parent );
  alarmLayout->addWidget( mAlarmStack );

  mAlarmInfoLabel = new QLabel( i18n("No reminders configured"), mAlarmStack );
  mAlarmStack->addWidget( mAlarmInfoLabel, AdvancedAlarmLabel );

  QHBox *simpleAlarmBox = new QHBox( mAlarmStack );
  mAlarmStack->addWidget( simpleAlarmBox, SimpleAlarmPage );

  mAlarmButton = new QCheckBox(i18n("&Reminder:"), simpleAlarmBox );
  QWhatsThis::add( mAlarmButton,
       i18n("Activates a reminder for this event or to-do.") );

  QString whatsThis = i18n("Sets how long before the event occurs "
                           "the reminder will be triggered.");
  mAlarmTimeEdit = new QSpinBox( 0, 99999, 1, simpleAlarmBox, "alarmTimeEdit" );
  mAlarmTimeEdit->setValue( 0 );
  QWhatsThis::add( mAlarmTimeEdit, whatsThis );

  mAlarmIncrCombo = new QComboBox( false, simpleAlarmBox );
  QWhatsThis::add( mAlarmIncrCombo, whatsThis );
  mAlarmIncrCombo->insertItem( i18n("minute(s)") );
  mAlarmIncrCombo->insertItem( i18n("hour(s)") );
  mAlarmIncrCombo->insertItem( i18n("day(s)") );
//  mAlarmIncrCombo->setMinimumHeight(20);
  connect(mAlarmButton, SIGNAL(toggled(bool)), mAlarmTimeEdit, SLOT(setEnabled(bool)));
  connect(mAlarmButton, SIGNAL(toggled(bool)), mAlarmIncrCombo, SLOT(setEnabled(bool)));
  mAlarmTimeEdit->setEnabled( false );
  mAlarmIncrCombo->setEnabled( false );

  mAlarmEditButton = new QPushButton( i18n("Advanced"), parent );
  mAlarmEditButton->setEnabled( false );
  alarmLayout->addWidget( mAlarmEditButton );
  connect( mAlarmButton, SIGNAL(toggled(bool)), mAlarmEditButton, SLOT(setEnabled( bool)));
  connect( mAlarmEditButton, SIGNAL( clicked() ),
      SLOT( editAlarms() ) );

}

void KOEditorGeneral::initAttachments(QWidget *parent,QBoxLayout *topLayout)
{
  mAttachments = new KOEditorAttachments( KDialog::spacingHint(), parent );
  connect( mAttachments, SIGNAL( openURL( const KURL & ) ) ,
           this, SIGNAL( openURL( const KURL & ) ) );
  topLayout->addWidget( mAttachments, 1 );
}

void KOEditorGeneral::addAttachments( const QStringList &attachments,
                                      const QStringList &mimeTypes,
                                      bool inlineAttachments )
{
  QStringList::ConstIterator it;
  uint i = 0;
  for ( it = attachments.begin(); it != attachments.end(); ++it, ++i ) {
    QString mimeType;
    if ( mimeTypes.count() > i )
      mimeType = mimeTypes[ i ];
    mAttachments->addAttachment( *it, mimeType, QString(), !inlineAttachments );
  }
}

void KOEditorGeneral::selectCategories()
{
  KPIM::CategorySelectDialog *categoryDialog = new KPIM::CategorySelectDialog( KOPrefs::instance(), mCategoriesButton	 );
  KOGlobals::fitDialogToScreen( categoryDialog );
  categoryDialog->setSelected( mCategories );

  connect(categoryDialog, SIGNAL(editCategories()), this, SIGNAL(openCategoryDialog()));
  connect(this, SIGNAL(updateCategoryConfig()), categoryDialog, SLOT(updateCategoryConfig()));

  if ( categoryDialog->exec() ) {
    setCategories( categoryDialog->selectedCategories() );
  }
  delete categoryDialog;
}


void KOEditorGeneral::editAlarms()
{
  if ( mAlarmStack->id( mAlarmStack->visibleWidget() ) == SimpleAlarmPage ) {
    mAlarmList.clear();
    Alarm *al = alarmFromSimplePage();
    if ( al ) {
      mAlarmList.append( al );
    }
  }

  KOEditorAlarms *dlg = new KOEditorAlarms( &mAlarmList, mAlarmEditButton );
  if ( dlg->exec() != KDialogBase::Cancel ) {
    updateAlarmWidgets();
  }
}


void KOEditorGeneral::enableAlarm( bool enable )
{
  mAlarmStack->setEnabled( enable );
  mAlarmEditButton->setEnabled( enable );
}


void KOEditorGeneral::toggleAlarm( bool on )
{
    mAlarmButton->setChecked( on );
}

void KOEditorGeneral::setCategories( const QStringList &categories )
{
  mCategoriesLabel->setText( categories.join(",") );
  mCategories = categories;
}

void KOEditorGeneral::setDefaults(bool /*allDay*/)
{
#if 0
  mOwnerLabel->setText(i18n("Owner: ") + KOPrefs::instance()->fullName());
#endif

  mAlarmList.clear();
  updateDefaultAlarmTime();
  updateAlarmWidgets();

  mSecrecyCombo->setCurrentItem(Incidence::SecrecyPublic);
  mAttachments->setDefaults();
}

void KOEditorGeneral::updateDefaultAlarmTime()
{
  // FIXME: Implement a KPrefsComboItem to solve this in a clean way.
// FIXME: Use an int value for minutes instead of 5 hardcoded values
  int alarmTime;
  int a[] = { 1,5,10,15,30 };
  int index = KOPrefs::instance()->mAlarmTime;
  if (index < 0 || index > 4) {
    alarmTime = 0;
  } else {
    alarmTime = a[index];
  }
  mAlarmTimeEdit->setValue(alarmTime);
}

void KOEditorGeneral::updateAlarmWidgets()
{
  if ( mAlarmList.isEmpty() ) {
    mAlarmStack->raiseWidget( SimpleAlarmPage );
    mAlarmButton->setChecked( false );
    mAlarmEditButton->setEnabled( false );
  } else if ( mAlarmList.count() > 1 ) {
    mAlarmStack->raiseWidget( AdvancedAlarmLabel );
    mAlarmInfoLabel->setText( i18n("1 advanced reminder configured",
                                   "%n advanced reminders configured",
                                   mAlarmList.count() ) );
    mAlarmEditButton->setEnabled( true );
  } else {
    Alarm *alarm = mAlarmList.first();
    // Check if its the trivial type of alarm, which can be
    // configured with a simply spin box...

    if ( alarm->type() == Alarm::Display && alarm->text().isEmpty()
         && alarm->repeatCount() == 0 && !alarm->hasTime()
         && alarm->hasStartOffset() && alarm->startOffset().asSeconds() < 0 )  {
      mAlarmStack->raiseWidget( SimpleAlarmPage );
      mAlarmButton->setChecked( true );
      int offset = alarm->startOffset().asSeconds();

      offset = offset / -60; // make minutes
      int useoffset = offset;
      if (offset % (24*60) == 0) { // divides evenly into days?
        useoffset = offset / (24*60);
        mAlarmIncrCombo->setCurrentItem(2);
      } else if (offset % 60 == 0) { // divides evenly into hours?
        useoffset = offset / 60;
        mAlarmIncrCombo->setCurrentItem(1);
      }
      mAlarmTimeEdit->setValue( useoffset );
    } else {
      mAlarmStack->raiseWidget( AdvancedAlarmLabel );
      mAlarmInfoLabel->setText( i18n("1 advanced reminder configured") );
      mAlarmEditButton->setEnabled( true );
    }
  }
}

void KOEditorGeneral::readIncidence(Incidence *event, Calendar *calendar)
{
  mSummaryEdit->setText(event->summary());
  mLocationEdit->setText(event->location());

  mDescriptionEdit->setText(event->description());

#if 0
  // organizer information
  mOwnerLabel->setText(i18n("Owner: ") + event->organizer().fullName() );
#endif

  mSecrecyCombo->setCurrentItem(event->secrecy());

  // set up alarm stuff
  mAlarmList.clear();
  Alarm::List::ConstIterator it;
  Alarm::List alarms = event->alarms();
  for( it = alarms.begin(); it != alarms.end(); ++it ) {
    Alarm *al = new Alarm( *(*it) );
    al->setParent( 0 );
    mAlarmList.append( al );
  }
  updateDefaultAlarmTime();
  updateAlarmWidgets();

  setCategories(event->categories());

  mAttachments->readIncidence( event );

  QString resLabel = KOHelper::resourceLabel( calendar, event );
  if ( !resLabel.isEmpty() ) {
    mResourceLabel->setText( i18n( "Calendar: %1" ).arg( resLabel ) );
    mResourceLabel->show();
  }
}

Alarm *KOEditorGeneral::alarmFromSimplePage() const
{
  if ( mAlarmButton->isChecked() ) {
    Alarm *alarm = new Alarm( 0 );
    alarm->setDisplayAlarm("");
    alarm->setEnabled(true);
    QString tmpStr = mAlarmTimeEdit->text();
    int j = mAlarmTimeEdit->value() * -60;
    if (mAlarmIncrCombo->currentItem() == 1)
      j = j * 60;
    else if (mAlarmIncrCombo->currentItem() == 2)
      j = j * (60 * 24);
    alarm->setStartOffset( j );
    return alarm;
  } else {
    return 0;
  }
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
  event->clearAlarms();
  if ( mAlarmStack->id( mAlarmStack->visibleWidget() ) == SimpleAlarmPage ) {
    Alarm *al = alarmFromSimplePage();
    if ( al ) {
      al->setParent( event );
      event->addAlarm( al );
    }
  } else {
    // simply assign the list of alarms
    Alarm::List::ConstIterator it;
    for( it = mAlarmList.begin(); it != mAlarmList.end(); ++it ) {
      Alarm *al = new Alarm( *(*it) );
      al->setParent( event );
      al->setEnabled( true );
      event->addAlarm( al );
    }
  }
  mAttachments->writeIncidence( event );
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

void KOEditorGeneral::updateAttendeeSummary(int count)
{
  if ( count <= 0 )
    mAttendeeSummaryLabel->setText( i18n("No attendees") );
  else
    mAttendeeSummaryLabel->setText( i18n( "One attendee", "%n attendees", count ) );
}
