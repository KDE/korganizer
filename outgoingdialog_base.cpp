#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file './outgoingdialog_base.ui'
**
** Created: Sat Nov 4 13:58:55 2000
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "outgoingdialog_base.h"

#include <qheader.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a OutgoingDialog_base which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
OutgoingDialog_base::OutgoingDialog_base( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "OutgoingDialog_base" );
    resize( 621, 206 ); 
    setCaption( i18n( "Scheduler - Outgoing Messages"  ) );
    grid = new QGridLayout( this ); 
    grid->setSpacing( 6 );
    grid->setMargin( 11 );

    mMessageListView = new QListView( this, "mMessageListView" );
    mMessageListView->addColumn( i18n( "Summary" ) );
    mMessageListView->addColumn( i18n( "Method" ) );
    mMessageListView->addColumn( i18n( "Recipients" ) );
    mMessageListView->setAllColumnsShowFocus( TRUE );

    grid->addMultiCellWidget( mMessageListView, 0, 2, 0, 0 );

    PushButton5 = new QPushButton( this, "PushButton5" );
    PushButton5->setText( i18n( "Send Messages"  ) );

    grid->addWidget( PushButton5, 0, 1 );
    QSpacerItem* spacer = new QSpacerItem( 0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding );
    grid->addItem( spacer, 1, 1 );

    PushButton6 = new QPushButton( this, "PushButton6" );
    PushButton6->setText( i18n( "Close"  ) );

    grid->addWidget( PushButton6, 2, 1 );

    // signals and slots connections
    connect( PushButton6, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( PushButton5, SIGNAL( clicked() ), this, SLOT( send() ) );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
OutgoingDialog_base::~OutgoingDialog_base()
{
    // no need to delete child widgets, Qt does it all for us
}

void OutgoingDialog_base::send()
{
    qWarning( "OutgoingDialog_base::send(): Not implemented yet!" );
}

#include "outgoingdialog_base.moc"
