/*
    This file is part of KOrganizer.

    Copyright (c) 1998 Preston Brown <pbrown@kde.org>
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

#include <QStackedWidget>
#include <QButtonGroup>
#include <QRadioButton>
#include <QLayout>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QSplitter>
#include <kvbox.h>
#include <QGridLayout>
#include <QGroupBox>

#include <kprinter.h>
#include <ksimpleconfig.h>
#include <kdebug.h>
#include <kdeversion.h>

#include "korganizer/corehelper.h"

#include "calprinthelper.h"

#include "calprinter.h"
#ifndef KORG_NOPRINTER
#include "calprinter.moc"

#include "calprintdefaultplugins.h"

CalPrinter::CalPrinter( QWidget *parent, Calendar *calendar, KOrg::CoreHelper *helper )
  : QObject( parent ), mHelper( 0 )
{
  setObjectName( "CalPrinter" );
  mParent = parent;
  mConfig = new KSimpleConfig( "korganizer_printing.rc" );
  mCoreHelper = helper;

  init( new KPrinter, calendar );
}

CalPrinter::~CalPrinter()
{
  kDebug(5850) << "~CalPrinter()" << endl;

  KOrg::PrintPlugin::List::Iterator it = mPrintPlugins.begin();
  for ( ; it != mPrintPlugins.end(); ++it ) {
    (*it)->doSaveConfig();
  }
  qDeleteAll(mPrintPlugins);
  mPrintPlugins.clear();

  delete mConfig;
  delete mPrintDialog;
  delete mPrinter;
  if ( mHelper ) delete mHelper;
}

void CalPrinter::init( KPrinter *printer, Calendar *calendar )
{
  mCalendar = calendar;
  mPrinter = printer;
  mHelper = new CalPrintHelper( mPrinter, mCalendar, mConfig, mCoreHelper );

  mPrintPlugins.clear();

  mPrintPlugins = mCoreHelper->loadPrintPlugins();
  mPrintPlugins.prepend( new CalPrintTodos() );
  mPrintPlugins.prepend( new CalPrintMonth() );
  mPrintPlugins.prepend( new CalPrintWeek() );
  mPrintPlugins.prepend( new CalPrintDay() );

  mPrintDialog = new CalPrintDialog( mPrintPlugins, mPrinter, mParent );

  KOrg::PrintPlugin::List::Iterator it = mPrintPlugins.begin();
  for ( ; it != mPrintPlugins.end(); ++it ) {
    (*it)->setConfig( mConfig );
    (*it)->setCalendar( calendar );
    (*it)->setPrinter( printer );
    (*it)->setKOrgCoreHelper( mCoreHelper );
    (*it)->setCalPrintHelper( mHelper );

    (*it)->doLoadConfig();
  }
}

void CalPrinter::setDateRange( const QDate &fd, const QDate &td )
{
  foreach( KOrg::PrintPlugin *plugin, mPrintPlugins ) {
    plugin->setDateRange( fd, td );
  }
}

void CalPrinter::preview( PrintType type, const QDate &fd, const QDate &td )
{
  mPrintDialog->setPreview( true );
  mPrintDialog->setPrintType( int( type ) );
  setDateRange( fd, td );

  if ( mPrintDialog->exec() == QDialog::Accepted ) {
    doPrint( mPrintDialog->selectedPlugin(), true );
  }
}

void CalPrinter::print( PrintType type, const QDate &fd, const QDate &td )
{
  mPrintDialog->setPreview( false );
  mPrintDialog->setPrintType( int( type ) );
  setDateRange( fd, td );

  if ( mPrintDialog->exec() == QDialog::Accepted ) {
    doPrint( mPrintDialog->selectedPlugin(), false );
  }
}

void CalPrinter::doPrint( KOrg::PrintPlugin *selectedStyle, bool preview )
{
  if ( mHelper ) delete mHelper;
  KPrinter::Orientation printerOrientation = mPrinter->orientation();

  mHelper = new CalPrintHelper( mPrinter, mCalendar, mConfig, mCoreHelper );
  if ( preview ) mPrinter->setPreviewOnly( true );
  switch ( mPrintDialog->orientation() ) {
    case eOrientPlugin:
      mPrinter->setOrientation( selectedStyle->orientation() );
      break;
    case eOrientPortrait:
      mPrinter->setOrientation( KPrinter::Portrait );
      break;
    case eOrientLandscape:
      mPrinter->setOrientation( KPrinter::Landscape );
      break;
    case eOrientPrinter:
    default:
      break;
  }

  if ( preview || mPrinter->setup( mParent, i18n("Print Calendar") ) ) {
  selectedStyle->setKOrgCoreHelper( mCoreHelper );
  selectedStyle->setCalPrintHelper( mHelper );
    selectedStyle->doPrint();
  }
  mPrinter->setPreviewOnly( false );
  mPrinter->setOrientation( printerOrientation );
}

///////////////////////////////////////////////////////////////////////////////

void CalPrinter::updateConfig()
{
}



/****************************************************************************/

CalPrintDialog::CalPrintDialog( KOrg::PrintPlugin::List plugins, KPrinter *p,
                                QWidget *parent )
  : KDialog( parent ),
    mPrinter( p ), mPrintPlugins( plugins )
{
  setCaption( i18n("Print") );
  setButtons( Ok | Cancel );
  KVBox *page = new KVBox( this );
  page->setSpacing( spacingHint() );
  setMainWidget( page );

  QSplitter *splitter = new QSplitter( page );
  splitter->setOrientation( Qt::Horizontal );

  QGroupBox *typeBox = new QGroupBox( i18n("Print Style"), splitter );
  QBoxLayout *typeLayout = new QVBoxLayout( typeBox );
  mTypeGroup = new QButtonGroup( typeBox );
  // use the minimal width possible = max width of the radio buttons, not extensible
/*  mTypeGroup->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)4,
    (QSizePolicy::SizeType)5, 0, 0,
      mTypeGroup->sizePolicy().hasHeightForWidth() ) );*/

  QWidget *splitterRight = new QWidget( splitter );
  splitterRight->setObjectName( "splitterRight" );
  QGridLayout *splitterRightLayout = new QGridLayout( splitterRight );
  splitterRightLayout->setMargin( marginHint() );
  splitterRightLayout->setSpacing( spacingHint() );

  mConfigArea = new QStackedWidget( splitterRight);
  splitterRightLayout->addWidget( mConfigArea, 0, 0, 1, 2);

  QLabel *orientationLabel = new QLabel( i18n("Page &orientation:"), splitterRight );
  orientationLabel->setObjectName( "orientationLabel" );
  splitterRightLayout->addWidget( orientationLabel, 1, 0 );

  mOrientationSelection = new QComboBox( splitterRight );
  mOrientationSelection->setObjectName( "orientationCombo" );
  mOrientationSelection->addItem( i18n("Use Default Orientation of Selected Style") );
  mOrientationSelection->addItem( i18n("Use Printer Default") );
  mOrientationSelection->addItem( i18n("Portrait") );
  mOrientationSelection->addItem( i18n("Landscape") );
  splitterRightLayout->addWidget( mOrientationSelection, 1, 1 );

  // signals and slots connections
  connect( mTypeGroup, SIGNAL( buttonClicked( int ) ), SLOT( setPrintType( int ) ) );
  connect( this, SIGNAL( okClicked() ), SLOT( slotOk() ) );

  // buddies
  orientationLabel->setBuddy( mOrientationSelection );

  KOrg::PrintPlugin::List::Iterator it = mPrintPlugins.begin();
  QRadioButton *radioButton;
  int id = 0;
  for ( ; it != mPrintPlugins.end(); ++it ) {
    radioButton = new QRadioButton( (*it)->description(), typeBox );
    mTypeGroup->addButton( radioButton, id );
    typeLayout->addWidget( radioButton );
    radioButton->setMinimumHeight( radioButton->sizeHint().height() - 5 );

    mConfigArea->insertWidget( id, (*it)->configWidget( mConfigArea ) );
    id++;
  }

  setMinimumSize( minimumSizeHint() );
  resize( minimumSizeHint() );
}

CalPrintDialog::~CalPrintDialog()
{
}

void CalPrintDialog::setPreview(bool preview)
{
  setButtonGuiItem( KDialog::Ok, preview ? KGuiItem(i18n("&Preview")) : KStdGuiItem::print() );
}

void CalPrintDialog::setPrintType( int i )
{
  // FIXME: Make a safe correlation between type and the radio button

  QAbstractButton *btn = mTypeGroup->button( i );
  if ( btn ) btn->setChecked( true );
  mConfigArea->setCurrentIndex( i );
}

KOrg::PrintPlugin *CalPrintDialog::selectedPlugin()
{
  int pos = mTypeGroup->checkedId();
  if ( pos < 0 ) return 0;
  KOrg::PrintPlugin *retval = mPrintPlugins.at( pos );
  return retval;
}

void CalPrintDialog::slotOk()
{
  mOrientation = (CalPrinter::ePrintOrientation)mOrientationSelection->currentIndex();

  KOrg::PrintPlugin::List::Iterator it = mPrintPlugins.begin();
  for ( ; it != mPrintPlugins.end(); ++it ) {
    (*it)->readSettingsWidget();
  }
}

#endif
