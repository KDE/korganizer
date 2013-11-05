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

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "calprinter.h"
#include "calprintdefaultplugins.h"
#include "korganizer/corehelper.h"

#include <KMessageBox>
#include <KPrintPreview>
#include <KStandardGuiItem>
#include <KVBox>
#include <kdeprintdialog.h> //krazy:exclude=camelcase TODO wait for kdelibs4.10

#include <QButtonGroup>
#include <QGridLayout>
#include <QGroupBox>
#include <QPrintDialog>
#include <QSplitter>
#include <QStackedWidget>
#include <QVBoxLayout>

CalPrinter::CalPrinter( QWidget *parent, const Akonadi::ETMCalendar::Ptr &calendar,
                        KOrg::CoreHelper *helper, bool uniqItem )
  : QObject( parent ), mUniqItem( uniqItem )
{
  mParent = parent;
  mConfig = new KConfig( QLatin1String("korganizer_printing.rc"), KConfig::SimpleConfig );
  mCoreHelper = helper;

  init( calendar );
}

CalPrinter::~CalPrinter()
{
  mPrintPlugins.clear();
  delete mConfig;
}

void CalPrinter::init( const Akonadi::ETMCalendar::Ptr &calendar )
{
  mCalendar = calendar;

  mPrintPlugins.clear();

  mPrintPlugins = mCoreHelper->loadPrintPlugins();
  if( !mUniqItem ) {
    mPrintPlugins.prepend( new CalPrintTodos() );
    mPrintPlugins.prepend( new CalPrintMonth() );
    mPrintPlugins.prepend( new CalPrintWeek() );
    mPrintPlugins.prepend( new CalPrintDay() );
  }
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
  KOrg::PrintPlugin::List::Iterator it;
  for ( it = mPrintPlugins.begin(); it != mPrintPlugins.end(); ++it ) {
    (*it)->setSelectedIncidences( selectedIncidences );
  }
  QPointer<CalPrintDialog> printDialog =
    new CalPrintDialog( type, mPrintPlugins, mParent, mUniqItem );

  KConfigGroup grp( mConfig, "" ); //orientation setting isn't in a group
  printDialog->setOrientation( CalPrinter::ePrintOrientation( grp.readEntry( "Orientation", 1 ) ) );
  printDialog->setPreview( preview );
  setDateRange( fd, td );

  if ( printDialog->exec() == QDialog::Accepted ) {
    grp.writeEntry( "Orientation", (int)printDialog->orientation() );

    // Save all changes in the dialog
    for ( it = mPrintPlugins.begin(); it != mPrintPlugins.end(); ++it ) {
      (*it)->doSaveConfig();
    }
    doPrint( printDialog->selectedPlugin(), printDialog->orientation(), preview );
  }
  delete printDialog;

  for ( it = mPrintPlugins.begin(); it != mPrintPlugins.end(); ++it ) {
    (*it)->setSelectedIncidences( Incidence::List() );
  }
}

void CalPrinter::doPrint( KOrg::PrintPlugin *selectedStyle,
                          CalPrinter::ePrintOrientation dlgorientation, bool preview )
{
  if ( !selectedStyle ) {
    KMessageBox::error(
      mParent,
      i18nc( "@info", "Unable to print, an invalid print style was specified." ),
      i18nc( "@title:window", "Printing error" ) );
    return;
  }

  QPrinter printer;
  switch ( dlgorientation ) {
  case eOrientPlugin:
    printer.setOrientation( selectedStyle->defaultOrientation() );
    break;
  case eOrientPortrait:
    printer.setOrientation( QPrinter::Portrait );
    break;
  case eOrientLandscape:
    printer.setOrientation( QPrinter::Landscape );
    break;
  case eOrientPrinter:
  default:
    break;
  }

  if ( preview ) {
    QPointer<KPrintPreview> printPreview = new KPrintPreview( &printer );
    selectedStyle->doPrint( &printer );
    printPreview->exec();
    delete printPreview;
  } else {
    QPointer<QPrintDialog> printDialog = KdePrint::createPrintDialog( &printer, mParent );
    if ( printDialog->exec() == QDialog::Accepted ) {
      selectedStyle->doPrint( &printer );
    }
    delete printDialog;
  }
}

void CalPrinter::updateConfig()
{
}

CalPrintDialog::CalPrintDialog( int initialPrintType, KOrg::PrintPlugin::List plugins,
                                QWidget *parent, bool uniqItem )
  : KDialog( parent )
{
  setCaption( i18nc( "@title:window", "Print" ) );
  setButtons( Ok | Cancel );
  setModal( true );
  KVBox *page = new KVBox( this );
  setMainWidget( page );

  QSplitter *splitter = new QSplitter( page );
  splitter->setOrientation( Qt::Horizontal );
  splitter->setChildrenCollapsible( false );
  QGroupBox *typeBox = new QGroupBox( i18nc( "@title:group", "Print Style" ), splitter );
  QBoxLayout *typeLayout = new QVBoxLayout( typeBox );
  mTypeGroup = new QButtonGroup( typeBox );

  QWidget *splitterRight = new QWidget( splitter );
  QGridLayout *splitterRightLayout = new QGridLayout( splitterRight );
  splitterRightLayout->setMargin( marginHint() );
  splitterRightLayout->setSpacing( spacingHint() );

  mConfigArea = new QStackedWidget( splitterRight );
  splitterRightLayout->addWidget( mConfigArea, 0, 0, 1, 2 );
  QLabel *orientationLabel = new QLabel( i18nc( "@label", "Page &orientation:" ), splitterRight );
  orientationLabel->setAlignment( Qt::AlignRight );
  splitterRightLayout->addWidget( orientationLabel, 1, 0 );

  mOrientationSelection = new KComboBox( splitterRight );
  mOrientationSelection->setToolTip(
    i18nc( "@info:tooltip", "Set the print orientation" ) );
  mOrientationSelection->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Choose if you want your output to be printed in \"portrait\" or "
           "\"landscape\". You can also default to the orientation best suited to "
           "the selected style or to your printer's default setting." ) );
  mOrientationSelection->addItem( i18nc( "@item:inlistbox",
                                         "Use Default Orientation of Selected Style" ) );
  mOrientationSelection->addItem( i18nc( "@item:inlistbox",
                                         "Use Printer Default" ) );
  mOrientationSelection->addItem( i18nc( "@item:inlistbox",
                                         "Portrait" ) );
  mOrientationSelection->addItem( i18nc( "@item:inlistbox",
                                         "Landscape" ) );
  splitterRightLayout->addWidget( mOrientationSelection, 1, 1 );

  // signals and slots connections
  connect( mTypeGroup, SIGNAL(buttonClicked(int)), SLOT(setPrintType(int)) );
  orientationLabel->setBuddy( mOrientationSelection );

  // First insert the config widgets into the widget stack. This possibly assigns
  // proper ids (when two plugins have the same sortID), so store them in a map
  // and use these new IDs to later sort the plugins for the type selection.
  for ( KOrg::PrintPlugin::List::ConstIterator it=plugins.constBegin();
        it != plugins.constEnd(); ++it ) {
    int newid = mConfigArea->insertWidget( (*it)->sortID(), (*it)->configWidget( mConfigArea ) );
    mPluginIDs[newid] = (*it);
  }
  // Insert all plugins in sorted order; plugins with clashing IDs will be first
  QMap<int, KOrg::PrintPlugin*>::ConstIterator mapit;
  int firstButton = true;
  int id = 0;
  for ( mapit = mPluginIDs.constBegin(); mapit != mPluginIDs.constEnd(); ++mapit ) {
    KOrg::PrintPlugin *p = mapit.value();
    QRadioButton *radioButton = new QRadioButton( p->description() );
    radioButton->setEnabled( p->enabled() );
    radioButton->setToolTip(
      i18nc( "@info:tooltip", "Select the type of print" ) );
    radioButton->setWhatsThis(
      i18nc( "@info:whatsthis",
             "Select one of the following types of prints you want to make. "
             "You may want to print an individual item, or all the items for a "
             "specific time range (like a day, week or month), or you may want "
             "to print your to-do list." ) );
    // Check the first available button (to ensure one is selected initially) and then
    // the button matching the desired print type -- if such is available!
    if ( ( firstButton || p->sortID() == initialPrintType ) && p->enabled() ) {
      firstButton = false;
      radioButton->setChecked( true );
      setPrintType( id );
    }
    mTypeGroup->addButton( radioButton, mapit.key() );
    typeLayout->addWidget( radioButton );
    id++;
  }
  if ( uniqItem ) {
    typeBox->hide();
  }
  typeLayout->insertStretch( -1, 100 );
  connect( this, SIGNAL(okClicked()), SLOT(slotOk()) );
  setMinimumSize( minimumSizeHint() );
  resize( minimumSizeHint() );
}

CalPrintDialog::~CalPrintDialog()
{
}

void CalPrintDialog::setPreview( bool preview )
{
  if ( preview ) {
    setButtonText( Ok, i18nc( "@action:button", "&Preview" ) );
  } else {
    setButtonText( Ok, KStandardGuiItem::print().text() );
  }
}

void CalPrintDialog::setPrintType( int i )
{
  mConfigArea->setCurrentIndex( i );
  mConfigArea->currentWidget()->raise();
  QAbstractButton *btn = mTypeGroup->button( i );
  if (btn) {
    btn->setChecked( true );
  }
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
  mOrientation =
    ( CalPrinter::ePrintOrientation )mOrientationSelection->currentIndex();

  QMap<int, KOrg::PrintPlugin*>::ConstIterator it = mPluginIDs.constBegin();
  for ( ; it != mPluginIDs.constEnd(); ++it ) {
    if ( it.value() ) {
      it.value()->readSettingsWidget();
    }
  }
}
