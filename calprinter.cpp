/*
    This file is part of KOrganizer.
    Copyright (c) 1998 Preston Brown

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

#include <qvbuttongroup.h>
#include <qwidgetstack.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qvbox.h>
#include <qsplitter.h>

#include <kprinter.h>
#include <ksimpleconfig.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kdeversion.h>

#include "koprefsdialog.h"

#include "calprinter.h"
#ifndef KORG_NOPRINTER
#include "calprinter.moc"

#include "calprintplugins.h"

CalPrinter::CalPrinter( QWidget *parent, Calendar *calendar )
  : QObject( parent, "CalPrinter" )
{
  mCalendar = calendar;
  mParent = parent;
  mPrinter = new KPrinter;
  mPrinter->setOrientation( KPrinter::Portrait );
  mConfig = new KSimpleConfig( "korganizer_printing.rc" );

  init( mPrinter, calendar );
}

CalPrinter::~CalPrinter()
{
  kdDebug(5850) << "~CalPrinter()" << endl;

  CalPrintBase *plug = mPrintPlugins.first();
  while ( plug ) {
    plug->doSaveConfig();
    plug = mPrintPlugins.next();
  }

  delete mConfig;
  delete mPrintDialog;
  delete mPrinter;
}

void CalPrinter::init( KPrinter *printer, Calendar *calendar )
{
  mPrintPlugins.setAutoDelete( true );
  mPrintPlugins.append( new CalPrintDay( printer, calendar, mConfig ) );
  mPrintPlugins.append( new CalPrintWeek( printer, calendar, mConfig ) );
  mPrintPlugins.append( new CalPrintMonth( printer, calendar, mConfig ) );
  mPrintPlugins.append( new CalPrintTodos( printer, calendar, mConfig ) );

  // @TODO: Add a plugin interface here
  mPrintDialog = new CalPrintDialog( mPrintPlugins, mPrinter, mParent );

  CalPrintBase *plug = mPrintPlugins.first();
  while ( plug ) {
    connect( mPrintDialog, SIGNAL( okClicked() ),
             plug, SLOT( readSettingsWidget() ) );

    plug->doLoadConfig();

    plug = mPrintPlugins.next();
  }
}

void CalPrinter::setupPrinter()
{
// TODO:REMOVEME:FIXME
  KMessageBox::sorry( mParent, i18n("Not implemented.") );
#if 0
  KOPrefsDialog *optionsDlg = new KOPrefsDialog(mParent);
  optionsDlg->readConfig();
  optionsDlg->showPrinterTab();
  connect(optionsDlg, SIGNAL(configChanged()),
          mParent, SLOT(updateConfig()));
  optionsDlg->show();
#endif
}

void CalPrinter::setDateRange( const QDate &fd, const QDate &td )
{
  CalPrintBase *plug = mPrintPlugins.first();
  while ( plug ) {
    plug->setDateRange( fd, td );
    plug = mPrintPlugins.next();
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

void CalPrinter::doPrint( CalPrintBase *selectedStyle, bool preview )
{
  mPrinter->setPreviewOnly( preview );
  if ( preview || mPrinter->setup( mParent, i18n("Print Calendar") ) ) {
    switch ( mPrintDialog->orientation() ) {
      case eOrientPlugin:
        mPrinter->setOrientation( selectedStyle->orientation());
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
    selectedStyle->doPrint();
  }
  mPrinter->setPreviewOnly( false );
}

///////////////////////////////////////////////////////////////////////////////

void CalPrinter::updateConfig()
{
}



/****************************************************************************/

CalPrintDialog::CalPrintDialog( QPtrList<CalPrintBase> plugins, KPrinter *p,
                                QWidget *parent, const char *name )
  : KDialogBase( parent, name, /*modal*/true, i18n("Print"), Ok | Cancel ),
    mPrinter( p ), mPrintPlugins( plugins )
{
  QVBox *page = makeVBoxMainWidget();

  QHBox *printerLayout = new QHBox( page );

  mPrinterLabel = new QLabel( printerLayout );
  QPushButton *setupButton = new QPushButton( i18n("&Setup Printer..."),
                                              printerLayout );
  setupButton->setSizePolicy( QSizePolicy(
      (QSizePolicy::SizeType)4, (QSizePolicy::SizeType)0,
      0, 0, setupButton->sizePolicy().hasHeightForWidth() ) );

  QSplitter *splitter = new QSplitter( page );
  splitter->setOrientation( QSplitter::Horizontal );

  mTypeGroup = new QVButtonGroup( i18n("View Type"), splitter, "buttonGroup" );
  // use the minimal width possible = max width of the radio buttons, not extensible
/*  mTypeGroup->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)4,
    (QSizePolicy::SizeType)5, 0, 0,
      mTypeGroup->sizePolicy().hasHeightForWidth() ) );*/

  QWidget *splitterRight = new QWidget( splitter, "splitterRight" );
  QGridLayout *splitterRightLayout = new QGridLayout( splitterRight );
  splitterRightLayout->setMargin( marginHint() );
  splitterRightLayout->setSpacing( spacingHint() );

  mConfigArea = new QWidgetStack( splitterRight, "configWidgetStack" );
  splitterRightLayout->addMultiCellWidget( mConfigArea, 0,0, 0,1 );

  QLabel *orientationLabel = new QLabel( i18n("Page &orientation:"),
                                         splitterRight, "orientationLabel" );
  splitterRightLayout->addWidget( orientationLabel, 1, 0 );

  mOrientationSelection = new QComboBox( splitterRight, "orientationCombo" );
  mOrientationSelection->insertItem( i18n("Use Default of Selected Style") );
  mOrientationSelection->insertItem( i18n("Use Default Setting of Printer") );
  mOrientationSelection->insertItem( i18n("Portrait") );
  mOrientationSelection->insertItem( i18n("Landscape") );
  splitterRightLayout->addWidget( mOrientationSelection, 1, 1 );

  // signals and slots connections
  connect( setupButton, SIGNAL( clicked() ), SLOT( setupPrinter() ) );
  connect( mTypeGroup, SIGNAL( clicked( int ) ), SLOT( setPrintType( int ) ) );

  // buddies
  orientationLabel->setBuddy( mOrientationSelection );

  CalPrintBase *plug = mPrintPlugins.first();
  QRadioButton *radioButton;
  int id = 0;
  while ( plug ) {
    radioButton = new QRadioButton( plug->description(), mTypeGroup );
    mTypeGroup->insert( radioButton, id );
    radioButton->setMinimumHeight( radioButton->sizeHint().height() - 5 );

    mConfigArea->addWidget( plug->configWidget( mConfigArea ), id );

    plug = mPrintPlugins.next();
    id++;
  }

  setMinimumSize( minimumSizeHint() );
  resize( minimumSizeHint() );
}

CalPrintDialog::~CalPrintDialog()
{
}

void CalPrintDialog::setupPrinter()
{
  if ( mPrinter->setup( this, i18n("Setup printer") ) ) {
    setPrinterLabel();
  }
}

void CalPrintDialog::setPreview(bool preview)
{
#if KDE_IS_VERSION( 3, 1, 93 )
  setButtonOK( preview ? i18n("&Preview") : KStdGuiItem::print() );
#else
  setButtonOKText( preview ? i18n("&Preview") : i18n("&Print...") );
#endif
  mPreviewText = preview ? i18n("<qt>Preview for printer <b>%1</b></qt>")
      : i18n( "<qt>Printing on printer <b>%1</b></qt>");
  setPrinterLabel();
}

void CalPrintDialog::setPrinterLabel()
{
  QString printerName( mPrinter->printerName() );
  if ( printerName.isEmpty() )
    mPrinterLabel->setText( mPreviewText.arg( i18n("[Unconfigured]") ) );
  else
    mPrinterLabel->setText( mPreviewText.arg( printerName ) );
}

void CalPrintDialog::setPrintType( int i )
{
  // @TODO: Make a safe correlation between type and the radio button

  mTypeGroup->setButton( i );
  mConfigArea->raiseWidget( i );
}

CalPrintBase *CalPrintDialog::selectedPlugin()
{
  int pos = mTypeGroup->id( mTypeGroup->selected() );
  if ( pos < 0 ) return 0;
  CalPrintBase *retval = mPrintPlugins.at( pos );
  return retval;
}

void CalPrintDialog::slotOk()
{
  mOrientation = (CalPrinter::ePrintOrientation)mOrientationSelection->currentItem();
  KDialogBase::slotOk();
}

#endif
