//quickdialog.cpp
// (C)1998 by Fester Zigterman

#include "quickdialog.h"
#include "quickdialog.moc"

QuickDialog::QuickDialog( CalObject *cal, QWidget *parent, char *name )
	: QFrame( parent, name )
{
	qCalendar = cal;
	qEvent = 0L;
	qLayout = new QVBoxLayout( this );
	qButtonLayout = new QHBoxLayout();
	qLayout->addWidget( summaryBox = new QLineEdit( this ) );
	qLayout->addStretch();
	qLayout->addWidget( descriptionBox = new QMultiLineEdit( this ) );
	qLayout->addLayout( qButtonLayout );
	button = new QPushButton( "Open Dialog", this );
	connect( button, SIGNAL( clicked() ), this, SLOT( slot_openDlg() ) );
	qButtonLayout->addWidget( button );
	button = new QPushButton( "New", this );
	connect( button, SIGNAL( clicked() ), this, SLOT( slot_new() ) );
	qButtonLayout->addWidget( button );
}

QuickDialog::~QuickDialog(){};

void QuickDialog::updateDialog()
{
	if( ! Modified )
	{
		qEvent = selectedEvent;
		fillDialog();
	}
}

void QuickDialog::setSelected( KOEvent *ev )
{
	selectedEvent = ev;
	updateDialog();
}

void QuickDialog::fillDialog()
{
	if( qEvent )
	{
		summaryBox->setText( qEvent->getSummary() );
		descriptionBox->setText( qEvent->getDescription() );
	}
}

void QuickDialog::slot_new(){}
void QuickDialog::slot_openDlg(){}
