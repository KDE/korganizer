// ArchiveDialog -- archive/delete past appointments.
// $Id$

#include <qlabel.h>
#include <qlayout.h>
#include <qdatetime.h>

#include <kapp.h>
#include <klocale.h>
#include <kurlrequester.h>
#include <kmessagebox.h>
#include <kglobal.h>
#include <kfiledialog.h>

#include "kdateedit.h"
#include "koarchivedlg.h"
#include "koarchivedlg.moc"

ArchiveDialog::ArchiveDialog(QWidget *parent, const char *name)
  : KDialogBase (Plain,i18n("Archive / Delete Past Appointments"),
                 User1|User2|Cancel,User1,parent,name,false,true,
                 i18n("Archive"),i18n("Delete"))
{
  QFrame *topFrame = plainPage();
  QGridLayout *topLayout = new QGridLayout(topFrame);
  topLayout->setSpacing(spacingHint());
  
  QLabel *dateLabel = new QLabel(i18n("Appointments older than:"),topFrame);
  topLayout->addWidget(dateLabel,0,0);

  mDateEdit = new KDateEdit(topFrame);
  topLayout->addWidget(mDateEdit,0,1);

  QHBox *fileBox = new QHBox(topFrame);
  fileBox->setSpacing(spacingHint());
  topLayout->addMultiCellWidget(fileBox,1,1,0,1);
  (void)new QLabel(i18n("Archive file:"),fileBox);
  mArchiveFile = new KURLRequester (fileBox);
  mArchiveFile->fileDialog()->setMode(KFile::File);
  mArchiveFile->fileDialog()->setFilter("*vcs|vCalendar Files");

/*  
  QButtonGroup *deleteGroup;
  deleteGroup = new QButtonGroup(this);
  //deleteGroup->setFrameStyle(QFrame::NoFrame);
  deleteGroup->move(10, dateLabel->geometry().bottomLeft().y()+10);

  QRadioButton *deleteRB, *archiveRB;
  deleteRB = new QRadioButton(deleteGroup);
  deleteRB->setText(i18n(i18n("Delete Events")));
  deleteRB->adjustSize();
  deleteRB->move(10, 10);
  
  archiveRB = new QRadioButton(deleteGroup);
  archiveRB->setText(i18n(i18n("Archive Events")));
  archiveRB->adjustSize();
  archiveRB->move(10, deleteRB->geometry().bottomLeft().y());

  deleteGroup->adjustSize();

  QLabel *archivePL;
  archivePL = new QLabel(this);
  archivePL->setText(i18n("Archive File:"));
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
  browse->setText(i18n("Browse"));
  browse->adjustSize();
  browse->move(archiveFnEdit->geometry().topRight().x()+10,
	       archivePL->geometry().topRight().y());

  QFrame *sepFrame;
  sepFrame = new QFrame(this);
  sepFrame->setFrameStyle(QFrame::Sunken|QFrame::HLine);
  sepFrame->setFixedWidth(this->width()-20);
  sepFrame->move(10, archiveFnEdit->geometry().bottomLeft().y()+10);

  QPushButton *ok, *cancel; 

  cancel = new QPushButton( i18n("Cancel"), this ); 
  cancel->setDefault(TRUE);
  cancel->setGeometry( this->width()-180, this->height()-40, 80,30 );
  connect( cancel, SIGNAL(clicked()), SLOT(reject()) );

  ok = new QPushButton( i18n("OK"), this );  
  ok->setGeometry( this->width()-90, this->height()-40, 80,30 ); 
  connect( ok, SIGNAL(clicked()), SLOT(accept()) );
*/
}

ArchiveDialog::~ArchiveDialog()
{
}

// Archive old events
void ArchiveDialog::slotUser1()
{
  qDebug("ArchiveDialog::slotUser1()");
  accept();
}

// Delete old events
void ArchiveDialog::slotUser2()
{
  int result = KMessageBox::warningContinueCancel(this,
      i18n("Delete all appointments older than %1?")
      .arg(KGlobal::locale()->formatDate(mDateEdit->getDate())),
      i18n("Delete old appointments"),i18n("Delete"));
  if (result == KMessageBox::Continue) {
    qDebug("ArchiveDialog::slotUser2(): Delete");
    accept();
  }
}
