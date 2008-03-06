/*
  This file is part of KOrganizer.
  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
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
#include "koeditorgeneral.h"
#include "koeditoralarms.h"
#include "koprefs.h"
#include "koglobals.h"

#include <libkdepim/kdateedit.h>
#include <libkdepim/categoryselectdialog.h>

#include <kcal/todo.h>
#include <kcal/event.h>

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
#include <kvbox.h>
#include <KLineEdit>
#include <KComboBox>

#include <q3buttongroup.h>
#include <QWidget>
#include <QLayout>
#include <QDateTime>
#include <QLabel>
#include <QCheckBox>
#include <QPushButton>
#include <QSpinBox>
#include <QFocusEvent>
#include <QGridLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QBoxLayout>
#include <QTextCharFormat>
#include <QTextList>

#include "koeditorattachments.h"
#include "koeditorgeneral.moc"
#include "kohelper.h"

KOEditorGeneral::KOEditorGeneral( QObject *parent )
  : QObject( parent ), mAttachments(0)
{
  mAlarmList.setAutoDelete( true );
}

KOEditorGeneral::~KOEditorGeneral()
{
}

FocusLineEdit::FocusLineEdit( QWidget *parent )
  : KLineEdit( parent ), mSkipFirst( true )
{
}

void FocusLineEdit::focusInEvent ( QFocusEvent *e )
{
  if ( !mSkipFirst ) {
    emit focusReceivedSignal();
  } else {
    mSkipFirst = false;
  }
  KLineEdit::focusInEvent( e );
}

void KOEditorGeneral::initHeader( QWidget *parent, QBoxLayout *topLayout )
{
  QGridLayout *headerLayout = new QGridLayout();
  headerLayout->setSpacing( topLayout->spacing() );
  topLayout->addLayout( headerLayout );

  QString whatsThis = i18n( "Sets the Title of this event or to-do." );
  QLabel *summaryLabel = new QLabel( i18nc( "event or to-do title", "T&itle:" ), parent );
  summaryLabel->setWhatsThis( whatsThis );
  QFont f = summaryLabel->font();
  f.setBold( true );
  summaryLabel->setFont( f );
  headerLayout->addWidget( summaryLabel, 1, 0 );

  mSummaryEdit = new FocusLineEdit( parent );
  mSummaryEdit->setWhatsThis( whatsThis );
  connect( mSummaryEdit, SIGNAL( focusReceivedSignal() ),
           SIGNAL( focusReceivedSignal() ) );
  headerLayout->addWidget( mSummaryEdit, 1, 1 );
  summaryLabel->setBuddy( mSummaryEdit );

  mAttendeeSummaryLabel = new QLabel( parent );
  updateAttendeeSummary( 0 );
  headerLayout->addWidget( mAttendeeSummaryLabel, 1, 2 );

  whatsThis = i18n( "Sets where the event or to-do will take place." );
  QLabel *locationLabel = new QLabel( i18n( "&Location:" ), parent );
  locationLabel->setWhatsThis( whatsThis );
  headerLayout->addWidget( locationLabel, 2, 0 );

  mLocationEdit = new KLineEdit( parent );
  mLocationEdit->setWhatsThis( whatsThis );
  headerLayout->addWidget( mLocationEdit, 2, 1, 1, 1 );
  locationLabel->setBuddy( mLocationEdit );

  QBoxLayout *thirdLineLayout = new QHBoxLayout();
  headerLayout->addMultiCellLayout( thirdLineLayout, 3, 3, 0, 2 );

  mResourceLabel = new QLabel( parent );
  mResourceLabel->hide();
  thirdLineLayout->addWidget( mResourceLabel );

  whatsThis = i18nc( "@info:whatsthis",
                     "Allows you to select the categories that this event or to-do belongs to." );
  QLabel *categoriesLabel = new QLabel( i18n( "Categories:" ), parent );
  categoriesLabel->setWhatsThis( whatsThis );
  thirdLineLayout->addWidget( categoriesLabel );
  mCategoriesLabel = new KSqueezedTextLabel( parent );
  mCategoriesLabel->setWhatsThis( whatsThis );
  mCategoriesLabel->setFrameStyle( QFrame::Panel|QFrame::Sunken );
  thirdLineLayout->addWidget( mCategoriesLabel );

  mCategoriesButton = new QPushButton( parent );
  mCategoriesButton->setText( i18nc( "@action:button select a category", "&Select..." ) );
  mCategoriesButton->setWhatsThis( whatsThis );
  connect( mCategoriesButton, SIGNAL(clicked()), SLOT(selectCategories()) );
  thirdLineLayout->addWidget( mCategoriesButton );
}

void KOEditorGeneral::initSecrecy( QWidget *parent, QBoxLayout *topLayout )
{
  QBoxLayout *secrecyLayout = new QHBoxLayout();
  secrecyLayout->setSpacing( topLayout->spacing() );
  topLayout->addItem( secrecyLayout );

  QLabel *secrecyLabel = new QLabel( i18n( "Acc&ess:" ), parent );
  QString whatsThis = i18n( "Sets whether the access to this event or to-do "
                            "is restricted. Please note that KOrganizer "
                            "currently does not use this setting, so the "
                            "implementation of the restrictions will depend "
                            "on the groupware server. This means that events "
                            "or to-dos marked as private or confidential may "
                            "be visible to others." );
  secrecyLabel->setWhatsThis( whatsThis );
  secrecyLayout->addWidget(secrecyLabel);

  mSecrecyCombo = new KComboBox( parent );
  mSecrecyCombo->setWhatsThis( whatsThis );
  mSecrecyCombo->addItems( Incidence::secrecyList() );
  secrecyLayout->addWidget( mSecrecyCombo );
  secrecyLabel->setBuddy( mSecrecyCombo );
}

void KOEditorGeneral::initDescription( QWidget *parent, QBoxLayout *topLayout )
{
  QBoxLayout *htmlLayout = new QHBoxLayout();
  topLayout->addItem( htmlLayout );
  mRichDescription = new QCheckBox( i18n( "Rich text" ), parent );
  mRichDescription->setToolTip( i18n( "Toggle Rich Text" ) );
  connect( mRichDescription, SIGNAL(toggled(bool)),
           this, SLOT(setDescriptionRich(bool)) );
  mDescriptionBoldButton = new QPushButton( parent );
  mDescriptionBoldButton->setIcon( KIcon( "format-text-bold" ) );
  mDescriptionBoldButton->setToolTip( i18n( "Bold text" ) );
  connect( mDescriptionBoldButton, SIGNAL(clicked()),
           this, SLOT(toggleDescriptionBold()) );

  mDescriptionItalicButton = new QPushButton( parent );
  mDescriptionItalicButton->setIcon( KIcon( "format-text-italic" ) );
  mDescriptionItalicButton->setToolTip( i18n( "Italicize text" ) );
  connect( mDescriptionItalicButton, SIGNAL(clicked()),
           this, SLOT(toggleDescriptionItalic()) );

  mDescriptionUnderlineButton = new QPushButton( parent );
  mDescriptionUnderlineButton->setIcon( KIcon( "format-text-underline" ) );
  mDescriptionUnderlineButton->setToolTip( i18n( "Underline text" ) );
  connect( mDescriptionUnderlineButton, SIGNAL(clicked()),
           this, SLOT(toggleDescriptionUnderline()) );

  mDescriptionStrikethroughButton = new QPushButton( parent );
  mDescriptionStrikethroughButton->setIcon( KIcon( "format-text-strikethrough" ) );
  mDescriptionStrikethroughButton->setToolTip( i18n( "Strike-through text" ) );
  connect( mDescriptionStrikethroughButton, SIGNAL(clicked()),
           this, SLOT(toggleDescriptionStrikethrough()) );

  mDescriptionLeftAlignButton = new QPushButton( parent );
  mDescriptionLeftAlignButton->setIcon( KIcon( "format-justify-left" ) );
  mDescriptionLeftAlignButton->setToolTip( i18n( "Left-justify text" ) );
  connect( mDescriptionLeftAlignButton, SIGNAL(clicked()),
           this, SLOT(toggleDescriptionLeftAlign()) );

  mDescriptionCentreAlignButton = new QPushButton( parent );
  mDescriptionCentreAlignButton->setIcon( KIcon( "format-justify-center" ) );
  mDescriptionCentreAlignButton->setToolTip( i18n( "Center text" ) );
  connect( mDescriptionCentreAlignButton, SIGNAL(clicked()),
           this, SLOT(toggleDescriptionCentreAlign()) );

  mDescriptionRightAlignButton = new QPushButton( parent );
  mDescriptionRightAlignButton->setIcon( KIcon( "format-justify-right" ) );
  mDescriptionRightAlignButton->setToolTip( i18n( "Right-justify text" ) );
  connect( mDescriptionRightAlignButton, SIGNAL(clicked()),
           this, SLOT(toggleDescriptionRightAlign()) );

  mDescriptionUnorderedListButton = new QPushButton( parent );
  mDescriptionUnorderedListButton->setIcon( KIcon( "format-list-unordered" ) );
  mDescriptionUnorderedListButton->setToolTip( i18n( "Unordered-list item" ) );
  connect( mDescriptionUnorderedListButton, SIGNAL(clicked()),
           this, SLOT(toggleDescriptionUnorderedList()) );

  mDescriptionOrderedListButton = new QPushButton( parent );
  mDescriptionOrderedListButton->setIcon( KIcon( "format-list-ordered" ) );
  mDescriptionOrderedListButton->setToolTip( i18n( "Ordered-list item" ) );
  connect( mDescriptionOrderedListButton, SIGNAL(clicked()),
           this, SLOT(toggleDescriptionOrderedList()) );

  htmlLayout->addWidget( mRichDescription );
  htmlLayout->addWidget( mDescriptionBoldButton );
  htmlLayout->addWidget( mDescriptionItalicButton );
  htmlLayout->addWidget( mDescriptionUnderlineButton );
  htmlLayout->addWidget( mDescriptionStrikethroughButton );
  htmlLayout->addWidget( mDescriptionLeftAlignButton );
  htmlLayout->addWidget( mDescriptionCentreAlignButton );
  htmlLayout->addWidget( mDescriptionRightAlignButton );
  htmlLayout->addWidget( mDescriptionUnorderedListButton );
  htmlLayout->addWidget( mDescriptionOrderedListButton );
  htmlLayout->addStretch();

  mDescriptionEdit = new KTextEdit( parent );
  mDescriptionEdit->setWhatsThis(
    i18n( "Sets the description for this event, to-do or journal. "
          "This will be displayed in a reminder if one is set, "
          "as well as in a tooltip when you hover over the event." ) );
  mDescriptionEdit->append( "" );
  mDescriptionEdit->setReadOnly( false );
  mDescriptionEdit->setOverwriteMode( false );
  mDescriptionEdit->setLineWrapMode( KTextEdit::WidgetWidth );
  mDescriptionEdit->setTabChangesFocus( true );
  topLayout->addWidget( mDescriptionEdit, 4 );

  toggleDescriptionRichButtons( mRichDescription->isChecked() );
}

void KOEditorGeneral::initAlarm( QWidget *parent, QBoxLayout *topLayout )
{
  QBoxLayout *alarmLayout = new QHBoxLayout();
  alarmLayout->setSpacing( topLayout->spacing() );
  topLayout->addItem( alarmLayout );

  mAlarmBell = new QLabel( parent );
  mAlarmBell->setPixmap( KOGlobals::self()->smallIcon( "preferences-desktop-notification-bell" ) );
  alarmLayout->addWidget( mAlarmBell );

  mAlarmStack = new QStackedWidget( parent );
  alarmLayout->addWidget( mAlarmStack );

  mAlarmInfoLabel = new QLabel( "XXX reminders configured", mAlarmStack );
  mAlarmStack->insertWidget( AdvancedAlarmLabel, mAlarmInfoLabel );

  QWidget *simpleAlarmBox = new QWidget( mAlarmStack );
  mAlarmStack->insertWidget( SimpleAlarmPage, simpleAlarmBox );

  QBoxLayout *simpleAlarmLayout = new QHBoxLayout( simpleAlarmBox );

  mAlarmButton = new QCheckBox( i18n( "&Reminder:" ) );
  mAlarmButton->setWhatsThis( i18n( "Activates a reminder for this event or to-do." ) );
  simpleAlarmLayout->addWidget( mAlarmButton );

  QString whatsThis =
    i18n( "Sets how long before the event occurs the reminder will be triggered." );
  mAlarmTimeEdit = new QSpinBox();
  mAlarmTimeEdit->setRange( 0, 99999 );
  mAlarmTimeEdit->setObjectName( "alarmTimeEdit" );
  mAlarmTimeEdit->setValue( 0 );
  mAlarmTimeEdit->setWhatsThis( whatsThis );
  simpleAlarmLayout->addWidget( mAlarmTimeEdit );

  mAlarmIncrCombo = new KComboBox();
  mAlarmIncrCombo->setWhatsThis( whatsThis );
  mAlarmIncrCombo->addItem( i18nc( "@item:inlistbox alarm expressed in minutes", "minute(s)" ) );
  mAlarmIncrCombo->addItem( i18nc( "@item:inlistbox alarm expressed in hours", "hour(s)" ) );
  mAlarmIncrCombo->addItem( i18nc( "@item:inlistbox alarm expressed in days", "day(s)" ) );
//  mAlarmIncrCombo->setMinimumHeight(20);
  connect( mAlarmButton, SIGNAL(toggled(bool)), mAlarmTimeEdit, SLOT(setEnabled(bool)) );
  connect( mAlarmButton, SIGNAL(toggled(bool)), mAlarmIncrCombo, SLOT(setEnabled(bool)) );
  simpleAlarmLayout->addWidget( mAlarmIncrCombo );

  mAlarmTimeEdit->setEnabled( false );
  mAlarmIncrCombo->setEnabled( false );

  mAlarmEditButton = new QPushButton( i18nc( "advanced alarm settings", "Advanced..." ), parent );
  alarmLayout->addWidget( mAlarmEditButton );
  connect( mAlarmEditButton, SIGNAL(clicked()), SLOT(editAlarms()) );
}

void KOEditorGeneral::initAttachments( QWidget *parent, QBoxLayout *topLayout )
{
  mAttachments = new KOEditorAttachments( KDialog::spacingHint(), parent );
  connect( mAttachments, SIGNAL(openURL(const KUrl &)),
           this, SIGNAL(openURL(const KUrl &)) );
  topLayout->addWidget( mAttachments, 1 );
}

void KOEditorGeneral::addAttachments( const QStringList &attachments,
                                      const QStringList &mimeTypes,
                                      bool inlineAttachments )
{
  QStringList::ConstIterator it;
  int i = 0;
  for ( it = attachments.begin(); it != attachments.end(); ++it, ++i ) {
    QString mimeType;
    if ( mimeTypes.count() > i ) {
      mimeType = mimeTypes[ i ];
    }
    mAttachments->addAttachment( *it, mimeType, QString(), inlineAttachments );
  }
}

void KOEditorGeneral::selectCategories()
{
  KPIM::CategorySelectDialog *categoryDialog =
    new KPIM::CategorySelectDialog( KOPrefs::instance(), mCategoriesButton );
  KOGlobals::fitDialogToScreen( categoryDialog );
  categoryDialog->setSelected( mCategories );

  connect( categoryDialog, SIGNAL(editCategories()), this, SIGNAL(openCategoryDialog()) );
  connect( this, SIGNAL(updateCategoryConfig()), categoryDialog, SLOT(updateCategoryConfig()) );

  if ( categoryDialog->exec() ) {
    setCategories( categoryDialog->selectedCategories() );
  }
  delete categoryDialog;
}

void KOEditorGeneral::editAlarms()
{
  if ( mAlarmStack->indexOf( mAlarmStack->currentWidget() ) == SimpleAlarmPage ) {
    mAlarmList.clear();
    Alarm *al = alarmFromSimplePage();
    if ( al ) {
      mAlarmList.append( al );
    }
  }

  KOEditorAlarms *dlg = new KOEditorAlarms( &mAlarmList, mAlarmEditButton );
  if ( dlg->exec() != KDialog::Cancel ) {
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
  mCategoriesLabel->setText( categories.join( "," ) );
  mCategories = categories;
}

void KOEditorGeneral::setDefaults( bool allDay )
{
  Q_UNUSED( allDay );

  mAlarmList.clear();
  updateDefaultAlarmTime();
  updateAlarmWidgets();

  mSecrecyCombo->setCurrentIndex( Incidence::SecrecyPublic );
  mAttachments->setDefaults();
}

void KOEditorGeneral::updateDefaultAlarmTime()
{
  // FIXME: Implement a KPrefsComboItem to solve this in a clean way.
// FIXME: Use an int value for minutes instead of 5 hardcoded values
  int alarmTime;
  int a[] = { 1, 5, 10, 15, 30 };
  int index = KOPrefs::instance()->mAlarmTime;
  if ( index < 0 || index > 4 ) {
    alarmTime = 0;
  } else {
    alarmTime = a[index];
  }
  mAlarmTimeEdit->setValue( alarmTime );
}

void KOEditorGeneral::updateAlarmWidgets()
{
  if ( mAlarmList.isEmpty() ) {
    mAlarmStack->setCurrentIndex( SimpleAlarmPage );
    mAlarmButton->setChecked( false );
  } else if ( mAlarmList.count() > 1 ) {
    mAlarmStack->setCurrentIndex( AdvancedAlarmLabel );
    mAlarmInfoLabel->setText( i18np( "1 reminder configured",
                                     "%1 reminders configured",
                                     mAlarmList.count() ) );
  } else {
    Alarm *alarm = mAlarmList.first();
    // Check if its the trivial type of alarm, which can be
    // configured with a simply spin box...

    if ( alarm->type() == Alarm::Display && alarm->text().isEmpty() &&
         alarm->repeatCount() == 0 && !alarm->hasTime() &&
         alarm->hasStartOffset() && alarm->startOffset().asSeconds() < 0 ) {
      mAlarmStack->setCurrentIndex( SimpleAlarmPage );
      mAlarmButton->setChecked( true );
      int offset = alarm->startOffset().asSeconds();

      offset = offset / -60; // make minutes
      int useoffset = offset;
      if ( offset % ( 24 * 60 ) == 0 ) { // divides evenly into days?
        useoffset = offset / ( 24 * 60 );
        mAlarmIncrCombo->setCurrentIndex( 2 );
      } else if ( offset % 60 == 0 ) { // divides evenly into hours?
        useoffset = offset / 60;
        mAlarmIncrCombo->setCurrentIndex( 1 );
      }
      mAlarmTimeEdit->setValue( useoffset );
    } else {
      mAlarmStack->setCurrentIndex( AdvancedAlarmLabel );
      mAlarmInfoLabel->setText( i18n( "1 advanced reminder configured" ) );
    }
  }
}

void KOEditorGeneral::readIncidence( Incidence *incidence, Calendar *calendar )
{
  setSummary( incidence->summary() );
  mLocationEdit->setText( incidence->location() );
  setDescription( incidence->description(), incidence->descriptionIsRich() );

  switch( incidence->secrecy() ) {
  case Incidence::SecrecyPublic:
    mSecrecyCombo->setCurrentIndex( 0 );
    break;
  case Incidence::SecrecyPrivate:
    mSecrecyCombo->setCurrentIndex( 1 );
    break;
  case Incidence::SecrecyConfidential:
    mSecrecyCombo->setCurrentIndex( 2 );
    break;
  }

  // set up alarm stuff
  mAlarmList.clear();
  Alarm::List::ConstIterator it;
  Alarm::List alarms = incidence->alarms();
  for ( it = alarms.begin(); it != alarms.end(); ++it ) {
    Alarm *al = new Alarm( *(*it) );
    al->setParent( 0 );
    mAlarmList.append( al );
  }
  updateDefaultAlarmTime();
  updateAlarmWidgets();

  setCategories( incidence->categories() );

  mAttachments->readIncidence( incidence );

  QString resLabel = KOHelper::resourceLabel( calendar, incidence );
  if ( !resLabel.isEmpty() ) {
    mResourceLabel->setText( i18n( "Calendar: %1", resLabel ) );
    mResourceLabel->show();
  }
}

Alarm *KOEditorGeneral::alarmFromSimplePage() const
{
  if ( mAlarmButton->isChecked() ) {
    Alarm *alarm = new Alarm( 0 );
    alarm->setDisplayAlarm( "" );
    alarm->setEnabled( true );
    QString tmpStr = mAlarmTimeEdit->text();
    int j = mAlarmTimeEdit->value() * -60;
    if ( mAlarmIncrCombo->currentIndex() == 1 ) {
      j = j * 60;
    } else if ( mAlarmIncrCombo->currentIndex() == 2 ) {
      j = j * ( 60 * 24 );
    }
    alarm->setStartOffset( j );
    return alarm;
  } else {
    return 0;
  }
}

void KOEditorGeneral::writeIncidence( Incidence *incidence )
{
  incidence->setSummary( mSummaryEdit->text() );
  incidence->setLocation( mLocationEdit->text() );
  if ( mRichDescription->isChecked() ) {
    incidence->setDescription( mDescriptionEdit->toHtml(), true );
  } else {
    incidence->setDescription( mDescriptionEdit->toPlainText(), false );
  }
  incidence->setCategories( mCategories );
  switch( mSecrecyCombo->currentIndex() ) {
  case 1:
    incidence->setSecrecy( Incidence::SecrecyPrivate );
    break;
  case 2:
    incidence->setSecrecy( Incidence::SecrecyConfidential );
    break;
  default:
    incidence->setSecrecy( Incidence::SecrecyPublic );
  }

  // alarm stuff
  incidence->clearAlarms();
  if ( mAlarmStack->indexOf( mAlarmStack->currentWidget() ) == SimpleAlarmPage ) {
    Alarm *al = alarmFromSimplePage();
    if ( al ) {
      al->setParent( incidence );
      incidence->addAlarm( al );
    }
  } else {
    // simply assign the list of alarms
    Alarm::List::ConstIterator it;
    for ( it = mAlarmList.begin(); it != mAlarmList.end(); ++it ) {
      Alarm *al = new Alarm( *(*it) );
      al->setParent( incidence );
      al->setEnabled( true );
      incidence->addAlarm( al );
    }
  }
  mAttachments->writeIncidence( incidence );
}

void KOEditorGeneral::setSummary( const QString &text )
{
  mSummaryEdit->setText( text );
}

void KOEditorGeneral::setDescription( const QString &text, bool isRich )
{
  mRichDescription->setChecked( isRich );
  if ( isRich ) {
    mDescriptionEdit->setHtml( text );

  } else {
    mDescriptionEdit->setPlainText( text );
  }
}

QObject *KOEditorGeneral::typeAheadReceiver() const
{
  return mSummaryEdit;
}

void KOEditorGeneral::toggleDescriptionBold()
{
  mRichDescription->setChecked( true );
  QTextCursor cursor( mDescriptionEdit->textCursor() );
  if ( cursor.selectionStart() == cursor.selectionEnd() ) {
    cursor.select( QTextCursor::WordUnderCursor );
  }
  QTextCharFormat text;
  if ( cursor.charFormat().fontWeight() == QFont::Bold ) {
    text.setFontWeight( QFont::Normal );
  } else {
    text.setFontWeight( QFont::Bold );
  }
  cursor.mergeCharFormat( text );
}

void KOEditorGeneral::toggleDescriptionItalic()
{
  mRichDescription->setChecked( true );
  QTextCursor cursor( mDescriptionEdit->textCursor() );
  if ( cursor.selectionStart() == cursor.selectionEnd() ) {
    cursor.select( QTextCursor::WordUnderCursor );
  }
  QTextCharFormat text;
  text.setFontItalic( !cursor.charFormat().fontItalic() );
  cursor.mergeCharFormat( text );
}

void KOEditorGeneral::toggleDescriptionUnderline()
{
  mRichDescription->setChecked( true );
  QTextCursor cursor( mDescriptionEdit->textCursor() );
  if ( cursor.selectionStart() == cursor.selectionEnd() ) {
    cursor.select( QTextCursor::WordUnderCursor );
  }
  QTextCharFormat text;
  text.setFontUnderline( !cursor.charFormat().fontUnderline() );
  cursor.mergeCharFormat( text );
}

void KOEditorGeneral::toggleDescriptionStrikethrough()
{
  mRichDescription->setChecked( true );
  QTextCursor cursor( mDescriptionEdit->textCursor() );
  if ( cursor.selectionStart() == cursor.selectionEnd() ) {
    cursor.select( QTextCursor::WordUnderCursor );
  }
  QTextCharFormat text;
  text.setFontStrikeOut( !cursor.charFormat().fontStrikeOut() );
  cursor.mergeCharFormat( text );
}

void KOEditorGeneral::toggleDescriptionLeftAlign()
{
  setAlignment( Qt::AlignLeft );
}

void KOEditorGeneral::toggleDescriptionCentreAlign()
{
  setAlignment( Qt::AlignHCenter );
}

void KOEditorGeneral::toggleDescriptionRightAlign()
{
  setAlignment( Qt::AlignRight );
}

void KOEditorGeneral::setAlignment( Qt::Alignment alignment )
{
  mRichDescription->setChecked( true );
  QTextCursor cursor( mDescriptionEdit->textCursor() );
  cursor.select( QTextCursor::LineUnderCursor );
  QTextBlockFormat text;
  if ( alignment != cursor.blockFormat().alignment() ) {
    text.setAlignment( alignment );
    cursor.mergeBlockFormat( text );
  }
}

void KOEditorGeneral::toggleDescriptionOrderedList()
{
  createList( QTextListFormat::ListDecimal );
}

void KOEditorGeneral::toggleDescriptionUnorderedList()
{
  createList( QTextListFormat::ListDisc );
}

void KOEditorGeneral::toggleDescriptionRichButtons( bool rich )
{
  mDescriptionBoldButton->setEnabled( rich );
  mDescriptionItalicButton->setEnabled( rich );
  mDescriptionUnderlineButton->setEnabled( rich );
  mDescriptionStrikethroughButton->setEnabled( rich );
  mDescriptionLeftAlignButton->setEnabled( rich );
  mDescriptionCentreAlignButton->setEnabled( rich );
  mDescriptionRightAlignButton->setEnabled( rich );
  mDescriptionUnorderedListButton->setEnabled( rich );
  mDescriptionOrderedListButton->setEnabled( rich );
}

void KOEditorGeneral::setDescriptionRich( bool rich )
{
  toggleDescriptionRichButtons( rich );
  if ( !rich ) {
    QTextCursor cursor( mDescriptionEdit->textCursor() );
    cursor.select( QTextCursor::Document );
    cursor.setCharFormat( QTextCharFormat() );
    mDescriptionEdit->setPlainText( mDescriptionEdit->toPlainText() );
  }
}

QTextList *KOEditorGeneral::createList( QTextListFormat::Style style )
{
  mRichDescription->setChecked( true );
  QTextCursor cursor( mDescriptionEdit->textCursor() );
  return cursor.createList( style );
}

void KOEditorGeneral::updateAttendeeSummary( int count )
{
  if ( count <= 0 ) {
    mAttendeeSummaryLabel->setText( "No attendees" );
  } else {
    mAttendeeSummaryLabel->setText( i18np( "One attendee", "%1 attendees", count ) );
  }
}
