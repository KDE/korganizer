// ArchiveDialog -- archive/delete past appointments.
// $Id$

#include <qpushbt.h>
#include <qbttngrp.h>
#include <qradiobt.h>
#include <qlined.h>
#include <qlabel.h>

#include <kapp.h>
#include <klocale.h>

#include "kdateedit.h"
#include "koarchivedlg.h"
#include "koarchivedlg.moc"

ArchiveDialog::ArchiveDialog(QWidget *parent, const char *name)
  : QDialog(parent, name, TRUE)
{
  setFixedSize(400,200); // reasonable defaults?
  setCaption(i18n("Archive / Delete Past Appointments - KOrganizer"));
  
  QLabel *dateLabel;
  dateLabel = new QLabel(this);
  dateLabel->setText("Appointments older than:");
  dateLabel->adjustSize();
  dateLabel->move(10, 10);

  KDateEdit *de;
  de = new KDateEdit(this);
  de->move(dateLabel->geometry().topRight().x()+10,10);
  
  QButtonGroup *deleteGroup;
  deleteGroup = new QButtonGroup(this);
  //deleteGroup->setFrameStyle(QFrame::NoFrame);
  deleteGroup->move(10, dateLabel->geometry().bottomLeft().y()+10);

  QRadioButton *deleteRB, *archiveRB;
  deleteRB = new QRadioButton(deleteGroup);
  deleteRB->setText("Delete Events");
  deleteRB->adjustSize();
  deleteRB->move(10, 10);
  
  archiveRB = new QRadioButton(deleteGroup);
  archiveRB->setText("Archive Events");
  archiveRB->adjustSize();
  archiveRB->move(10, deleteRB->geometry().bottomLeft().y());

  deleteGroup->adjustSize();

  QLabel *archivePL;
  archivePL = new QLabel(this);
  archivePL->setText("Archive File:");
  archivePL->adjustSize();
  archivePL->move(10, deleteGroup->geometry().bottomLeft().y()+10);

  QLineEdit *archiveFnEdit;
  archiveFnEdit = new QLineEdit(this);
  archiveFnEdit->setFixedWidth(150);
  archiveFnEdit->adjustSize();
  archiveFnEdit->move(archivePL->geometry().topRight().x()+10,
		      archivePL->geometry().topRight().y());

  
  QPushButton *browse;
  browse = new QPushButton(this);
  browse->setText("Browse");
  browse->adjustSize();
  browse->move(archiveFnEdit->geometry().topRight().x()+10,
	       archivePL->geometry().topRight().y());

  QFrame *sepFrame;
  sepFrame = new QFrame(this);
  sepFrame->setFrameStyle(QFrame::Sunken|QFrame::HLine);
  sepFrame->setFixedWidth(this->width()-20);
  sepFrame->move(10, archiveFnEdit->geometry().bottomLeft().y()+10);

  QPushButton *ok, *cancel; 

  cancel = new QPushButton( "Cancel", this ); 
  cancel->setDefault(TRUE);
  cancel->setGeometry( this->width()-180, this->height()-40, 80,30 );
  connect( cancel, SIGNAL(clicked()), SLOT(reject()) );

  ok = new QPushButton( "OK", this );  
  ok->setGeometry( this->width()-90, this->height()-40, 80,30 ); 
  connect( ok, SIGNAL(clicked()), SLOT(accept()) );

}

ArchiveDialog::~ArchiveDialog()
{
}

void ArchiveDialog::accept()
{
  
}
