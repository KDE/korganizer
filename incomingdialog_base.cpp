#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file './incomingdialog_base.ui'
**
** Created: Fri Nov 17 11:22:27 2000
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "incomingdialog_base.h"

#include <qgroupbox.h>
#include <qheader.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a IncomingDialog_base which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
IncomingDialog_base::IncomingDialog_base( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "IncomingDialog_base" );
    resize( 618, 212 ); 
    setCaption( i18n( "Scheduler - Incoming Messages"  ) );
    grid = new QGridLayout( this ); 
    grid->setSpacing( 6 );
    grid->setMargin( 11 );

    GroupBox1 = new QGroupBox( this, "GroupBox1" );
    GroupBox1->setTitle( i18n( "Message"  ) );
    GroupBox1->setColumnLayout(0, Qt::Vertical );
    GroupBox1->layout()->setSpacing( 0 );
    GroupBox1->layout()->setMargin( 0 );
    grid_2 = new QGridLayout( GroupBox1->layout() );
    grid_2->setAlignment( Qt::AlignTop );
    grid_2->setSpacing( 6 );
    grid_2->setMargin( 11 );

    PushButton9 = new QPushButton( GroupBox1, "PushButton9" );
    PushButton9->setText( i18n( "Reject"  ) );

    grid_2->addWidget( PushButton9, 1, 0 );

    PushButton8 = new QPushButton( GroupBox1, "PushButton8" );
    PushButton8->setText( i18n( "Accept"  ) );

    grid_2->addWidget( PushButton8, 0, 0 );

    grid->addWidget( GroupBox1, 1, 1 );

    PushButton4 = new QPushButton( this, "PushButton4" );
    PushButton4->setText( i18n( "Retrieve Messages"  ) );

    grid->addWidget( PushButton4, 0, 1 );
    QSpacerItem* spacer = new QSpacerItem( 0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding );
    grid->addItem( spacer, 3, 1 );

    PushButton7 = new QPushButton( this, "PushButton7" );
    PushButton7->setText( i18n( "Close"  ) );

    grid->addWidget( PushButton7, 4, 1 );

    mMessageListView = new QListView( this, "mMessageListView" );
    mMessageListView->addColumn( i18n( "Summary" ) );
    mMessageListView->addColumn( i18n( "Method" ) );
    mMessageListView->addColumn( i18n( "Status" ) );
    mMessageListView->setAllColumnsShowFocus( TRUE );

    grid->addMultiCellWidget( mMessageListView, 0, 4, 0, 0 );

    PushButton7_2 = new QPushButton( this, "PushButton7_2" );
    PushButton7_2->setText( i18n( "Accept All"  ) );

    grid->addWidget( PushButton7_2, 2, 1 );

    // signals and slots connections
    connect( PushButton7, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( PushButton4, SIGNAL( clicked() ), this, SLOT( retrieve() ) );
    connect( PushButton7_2, SIGNAL( clicked() ), this, SLOT( acceptAllMessages() ) );
    connect( PushButton8, SIGNAL( clicked() ), this, SLOT( acceptMessage() ) );
    connect( PushButton9, SIGNAL( clicked() ), this, SLOT( rejectMessage() ) );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
IncomingDialog_base::~IncomingDialog_base()
{
    // no need to delete child widgets, Qt does it all for us
}

void IncomingDialog_base::acceptAllMessages()
{
    qWarning( "IncomingDialog_base::acceptAllMessages(): Not implemented yet!" );
}

void IncomingDialog_base::acceptMessage()
{
    qWarning( "IncomingDialog_base::acceptMessage(): Not implemented yet!" );
}

void IncomingDialog_base::rejectMessage()
{
    qWarning( "IncomingDialog_base::rejectMessage(): Not implemented yet!" );
}

void IncomingDialog_base::retrieve()
{
    qWarning( "IncomingDialog_base::retrieve(): Not implemented yet!" );
}

#include "incomingdialog_base.moc"
