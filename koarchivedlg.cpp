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
#include <kurl.h>
#include <ktempfile.h>
#include <kio/netaccess.h>
#include <klineedit.h>

#include "kdateedit.h"
#include "koevent.h"
#include "calobject.h"

#include "koarchivedlg.h"
#include "koarchivedlg.moc"

ArchiveDialog::ArchiveDialog(CalObject *cal,QWidget *parent, const char *name)
  : KDialogBase (Plain,i18n("Archive / Delete Past Appointments"),
                 User1|User2|Cancel,User1,parent,name,false,true,
                 i18n("Archive"),i18n("Delete"))
{
  mCalendar = cal;

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
}

ArchiveDialog::~ArchiveDialog()
{
}

// Archive old events
void ArchiveDialog::slotUser1()
{
  QString str = mArchiveFile->lineEdit()->text();
  if (str.isEmpty()) {
    KMessageBox::sorry(this,i18n("The archive file name is not valid.\n"));
    return;
  }

  qDebug("str: %s",str.latin1());
  KURL destUrl;
  destUrl.setPath(str);
  qDebug("url: %s",destUrl.prettyURL().latin1());
  QString filename = destUrl.fileName();
  if (filename.right(4) != ".vcs") {
    filename.append(".vcs");
    destUrl.setFileName(filename);
  }
  qDebug("url: %s",destUrl.prettyURL().latin1());

  QList<KOEvent> events = mCalendar->getEvents(QDate(1800,1,1),
                                               mDateEdit->getDate(),true);
  if (events.count() == 0) {
    KMessageBox::sorry(this,i18n("There are no events before %1")
        .arg(KGlobal::locale()->formatDate(mDateEdit->getDate())));
    return;
  }

  CalObject archiveCal;
  
  KOEvent *ev;
  for(ev=events.first();ev;ev=events.next()) {
    archiveCal.addEvent(ev);
  }

  KTempFile tmpFile;
  if (!archiveCal.save(tmpFile.name())) {
    KMessageBox::error(this,i18n("Cannot write archive file."));
    return;
  }

  if (!KIO::NetAccess::upload(tmpFile.name(),destUrl)) {
    KMessageBox::error(this,i18n("Cannot write archive to final destination."));
    return;
  }

  accept();
}

// Delete old events
void ArchiveDialog::slotUser2()
{
  QList<KOEvent> events = mCalendar->getEvents(QDate(1769,12,1),
                                               mDateEdit->getDate(),true);

  if (events.count() == 0) {
    KMessageBox::sorry(this,i18n("There are no events before %1")
        .arg(KGlobal::locale()->formatDate(mDateEdit->getDate())));
    return;
  }

  QStringList eventStrs;
  KOEvent *ev;
  for(ev=events.first();ev;ev=events.next()) {
    eventStrs.append(ev->getSummary());
  }
  
  int result = KMessageBox::questionYesNoList(this,
      i18n("Delete all events before %1?\nThe following events will be deleted:")
      .arg(KGlobal::locale()->formatDate(mDateEdit->getDate())),eventStrs,
      i18n("Delete old events"),i18n("Delete"));
  if (result == KMessageBox::Yes) {
    for(ev=events.first();ev;ev=events.next()) {
      mCalendar->deleteEvent(ev);
    }
    accept();
  }
}
