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
#include <QGroupBox>
#include <QButtonGroup>
#include <QRadioButton>
#include <QLayout>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QSplitter>
#include <QGridLayout>
#include <QGroupBox>

#include <kvbox.h>
#include <kprinter.h>
#include <ksimpleconfig.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <kstandardguiitem.h>

#include "korganizer/corehelper.h"

#include "calprinter.h"
#ifndef KORG_NOPRINTER
#include "calprinter.moc"

#include "calprintdefaultplugins.h"

CalPrinter::CalPrinter( QWidget *parent, Calendar *calendar, KOrg::CoreHelper *helper )
  : QObject( parent )
{
  mParent = parent;
  mConfig = new KSimpleConfig( "korganizer_printing.rc" );
  mCoreHelper = helper;

  init( calendar );
}

CalPrinter::~CalPrinter()
{
  kDebug(5850) << "~CalPrinter()" << endl;

  mPrintPlugins.clear();

  delete mConfig;
}

void CalPrinter::init( Calendar *calendar )
{
  mCalendar = calendar;

  mPrintPlugins.clear();

  mPrintPlugins = mCoreHelper->loadPrintPlugins();
  mPrintPlugins.prepend( new CalPrintTodos() );
  mPrintPlugins.prepend( new CalPrintMonth() );
  mPrintPlugins.prepend( new CalPrintWeek() );
  mPrintPlugins.prepend( new CalPrintDay() );
  mPrintPlugins.prepend( new CalPrintIncidence() );

  KOrg::PrintPlugin::List::Iterator it = mPrintPlugins.begin();
  for ( ; it != mPrintPlugins.end(); ++it ) {
    if ( *it ) {
      (*it)->setConfig( mConfig );
      (*it)->setCalendar( mCalendar );
      (*it)->setKOrgCoreHelper( mCoreHelper );
      (*it)->doLoadConfig();
    }
  }
}

void CalPrinter::setDateRange( const QDate &fd, const QDate &td )
{
  KOrg::PrintPlugin::List::Iterator it = mPrintPlugins.begin();
  for ( ; it != mPrintPlugins.end(); ++it ) {
    (*it)->setDateRange( fd, td );
  }
}

void CalPrinter::print( int type, const QDate &fd, const QDate &td,
                        Incidence::List selectedIncidences, bool preview )
{
  KOrg::PrintPlugin::List::Iterator it = mPrintPlugins.begin();
  for ( it = mPrintPlugins.begin(); it != mPrintPlugins.end(); ++it ) {
    (*it)->setSelectedIncidences( selectedIncidences );
  }
  CalPrintDialog printDialog( mPrintPlugins, mParent );
  printDialog.setOrientation( CalPrinter::ePrintOrientation( mConfig->readEntry("Orientation", 1 ) ) );
  printDialog.setPreview( preview );
  printDialog.setPrintType( type );
  setDateRange( fd, td );

  if ( printDialog.exec() == QDialog::Accepted ) {
    mConfig->writeEntry( "Orientation", (int)printDialog.orientation() );

    // Save all changes in the dialog
    for ( it = mPrintPlugins.begin(); it != mPrintPlugins.end(); ++it ) {
      (*it)->doSaveConfig();
    }
    doPrint( printDialog.selectedPlugin(), printDialog.orientation(), preview );
  }
  for ( it = mPrintPlugins.begin(); it != mPrintPlugins.end(); ++it ) {
    (*it)->setSelectedIncidences( Incidence::List() );
  }
}

void CalPrinter::doPrint( KOrg::PrintPlugin *selectedStyle,
                          CalPrinter::ePrintOrientation dlgorientation, bool preview )
{
  if ( !selectedStyle ) {
    KMessageBox::error( mParent,
                 i18n("Unable to print, no valid print style was returned."),
                 i18n("Printing error") );
    return;
  }
  KPrinter printer;

  printer.setPreviewOnly( preview );
  switch ( dlgorientation ) {
    case eOrientPlugin:
      printer.setOrientation( selectedStyle->defaultOrientation() );
      break;
    case eOrientPortrait:
      printer.setOrientation( KPrinter::Portrait );
      break;
    case eOrientLandscape:
      printer.setOrientation( KPrinter::Landscape );
      break;
    case eOrientPrinter:
    default:
      break;
  }

  if ( preview || printer.setup( mParent, i18n("Print Calendar") ) ) {
    selectedStyle->doPrint( &printer );
  }
}

///////////////////////////////////////////////////////////////////////////////

void CalPrinter::updateConfig()
{
}



/****************************************************************************/

CalPrintDialog::CalPrintDialog( KOrg::PrintPlugin::List plugins,
                                QWidget *parent )
  : KDialog( parent )
{
  setCaption( i18n( "Print" ) );
  setButtons( Ok | Cancel );
  setModal( true );
  KVBox *page = new KVBox( this );
  setMainWidget( page );

  QSplitter *splitter = new QSplitter( page );
  splitter->setOrientation( Qt::Horizontal );

  QGroupBox *typeBox = new QGroupBox( i18n( "Print Style" ), splitter );
  QBoxLayout *typeLayout = new QVBoxLayout( typeBox );
  mTypeGroup = new QButtonGroup( typeBox );
  // use the minimal width possible = max width of the radio buttons, not extensible
/*  mTypeGroup->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)4,
    (QSizePolicy::SizeType)5, 0, 0,
      mTypeGroup->sizePolicy().hasHeightForWidth() ) );*/

  QWidget *splitterRight = new QWidget( splitter );
  QGridLayout *splitterRightLayout = new QGridLayout( splitterRight );
  splitterRightLayout->setMargin( marginHint() );
  splitterRightLayout->setSpacing( spacingHint() );

  mConfigArea = new QStackedWidget( splitterRight );
  splitterRightLayout->addMultiCellWidget( mConfigArea, 0,0, 0, 1 );
  QLabel *orientationLabel = new QLabel( i18n("Page &orientation:"),
                                         splitterRight );
  splitterRightLayout->addWidget( orientationLabel, 1, 0 );

  mOrientationSelection = new QComboBox( splitterRight );
  mOrientationSelection->addItem( i18n("Use Default Orientation of Selected Style") );
  mOrientationSelection->addItem( i18n("Use Printer Default") );
  mOrientationSelection->addItem( i18n("Portrait") );
  mOrientationSelection->addItem( i18n("Landscape") );
  splitterRightLayout->addWidget( mOrientationSelection, 1, 1 );

  // signals and slots connections
  connect( mTypeGroup, SIGNAL( clicked( int ) ), SLOT( setPrintType( int ) ) );
  orientationLabel->setBuddy( mOrientationSelection );

  // First insert the config widgets into the widget stack. This possibly assigns
  // proper ids (when two plugins have the same sortID), so store them in a map
  // and use these new IDs to later sort the plugins for the type selection.
  for ( KOrg::PrintPlugin::List::Iterator it = plugins.begin();
        it != plugins.end(); ++it ) {
    int newid = mConfigArea->insertWidget( (*it)->sortID(), (*it)->configWidget( mConfigArea ) );
    mPluginIDs[newid] = (*it);
  }
  // Insert all plugins with in sorted order; plugins with clashing IDs will be first...
  QMap<int, KOrg::PrintPlugin*>::ConstIterator mapit;
  for ( mapit = mPluginIDs.begin(); mapit != mPluginIDs.end(); ++mapit ) {
    KOrg::PrintPlugin *p = mapit.value();
    QRadioButton *radioButton = new QRadioButton( p->description() );
    radioButton->setEnabled( p->enabled() );
//     radioButton->setMinimumHeight( radioButton->sizeHint().height() - 5 );
    mTypeGroup->addButton( radioButton, mapit.key() );
    typeLayout->addWidget( radioButton );
  }

  setMinimumSize( minimumSizeHint() );
  resize( minimumSizeHint() );
}

CalPrintDialog::~CalPrintDialog()
{
}

void CalPrintDialog::setPreview(bool preview)
{
  if ( preview ) {
    setButtonText( Ok, i18n("&Preview") );
  } else {
    setButtonText( Ok, KStandardGuiItem::print().text() );
  }
}

void CalPrintDialog::setPrintType( int i )
{
  mConfigArea->setCurrentIndex( i );
  mConfigArea->currentWidget()->raise();
}

void CalPrintDialog::setOrientation( CalPrinter::ePrintOrientation orientation )
{
  mOrientation = orientation;
  mOrientationSelection->setCurrentIndex( mOrientation );
}

KOrg::PrintPlugin *CalPrintDialog::selectedPlugin()
{
  int id = mConfigArea->currentIndex();
  if ( mPluginIDs.contains( id ) ) {
    return mPluginIDs[id];
  } else {
    return 0;
  }
}

void CalPrintDialog::slotOk()
{
  mOrientation = (CalPrinter::ePrintOrientation)mOrientationSelection->currentIndex();

  QMap<int, KOrg::PrintPlugin*>::Iterator it = mPluginIDs.begin();
  for ( ; it != mPluginIDs.end(); ++it ) {
    if ( it.value() )
      it.value()->readSettingsWidget();
  }
}

#endif
