/*
    This file is part of KOrganizer.
    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

// ArchiveDialog -- archive/delete past appointments.

#include <qlabel.h>
#include <qlayout.h>
#include <qdatetime.h>

#include <kdebug.h>
#include <klocale.h>
#include <kurlrequester.h>
#include <kmessagebox.h>
#include <kglobal.h>
#include <kfiledialog.h>
#include <kurl.h>
#include <ktempfile.h>
#include <kio/netaccess.h>
#include <klineedit.h>

#include <libkcal/event.h>
#include <libkcal/calendar.h>
#include <libkcal/calendarlocal.h>

#include <libkdepim/kdateedit.h>

#include "koprefs.h"

#include "archivedialog.h"
#include "archivedialog.moc"

ArchiveDialog::ArchiveDialog(Calendar *cal,QWidget *parent, const char *name)
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
  mArchiveFile = new KURLRequester (KOPrefs::instance()->mArchiveFile,fileBox);
  mArchiveFile->fileDialog()->setMode(KFile::File);
  mArchiveFile->fileDialog()->setFilter(i18n("*.vcs|vCalendar Files"));
  connect(mArchiveFile->lineEdit(),SIGNAL(textChanged ( const QString & )),this,SLOT(slotArchiveFileChanged(const QString &)));
  enableButton(KDialogBase::User1,!mArchiveFile->lineEdit()->text().isEmpty());
}

ArchiveDialog::~ArchiveDialog()
{
}

void ArchiveDialog::slotArchiveFileChanged(const QString &text)
{
    enableButton(KDialogBase::User1,!text.isEmpty());
}

// Archive old events
void ArchiveDialog::slotUser1()
{
  // Get destination URL
  KURL destUrl = mArchiveFile->url();
  if (destUrl.isMalformed()) {
    KMessageBox::sorry(this,i18n("The archive file name is not valid.\n"));
    return;
  }
  // Force filename to be ending with vCalendar extension
  QString filename = destUrl.fileName();
  if (filename.right(4) != ".vcs") {
    filename.append(".vcs");
    destUrl.setFileName(filename);
  }

  // Get events to be archived
  QPtrList<Event> events = mCalendar->getEvents(QDate(1800,1,1),
                                             mDateEdit->date().addDays(-1),true);
  if (events.count() == 0) {
    KMessageBox::sorry(this,i18n("There are no events before %1")
        .arg(KGlobal::locale()->formatDate(mDateEdit->date())));
    return;
  }

  // Save current calendar to disk
  KTempFile tmpFile;
  tmpFile.setAutoDelete(true);
  if (!mCalendar->save(tmpFile.name())) {
    kdDebug() << "ArchiveDialog::slotUser1(): Can't save calendar to temp file" << endl;
    return;
  }

  // Duplicate current calendar by loading in new calendar object
  CalendarLocal archiveCalendar(KOPrefs::instance()->mTimeZoneId.local8Bit());
  if (!archiveCalendar.load(tmpFile.name())) {
    kdDebug() << "ArchiveDialog::slotUser1(): Can't load calendar from temp file" << endl;
    return;
  }

  // Strip active events from calendar so that only events to be archived
  // remain.
  QPtrList<Event> activeEvents = archiveCalendar.getEvents(mDateEdit->date(),
                                                          QDate(3000,1,1),
                                                          false);
  Event *ev;
  for(ev=activeEvents.first();ev;ev=activeEvents.next()) {
    archiveCalendar.deleteEvent(ev);
  }

  // Get or create the archive file
  QString archiveFile;

  if (KIO::NetAccess::exists(destUrl)) {
    if(!KIO::NetAccess::download(destUrl,archiveFile)) {
      kdDebug() << "ArchiveDialog::slotUser1(): Can't download archive file" << endl;
      return;
    }
    // Merge with events to be archived.
    if (!archiveCalendar.load(archiveFile)) {
      kdDebug() << "ArchiveDialog::slotUser1(): Can't merge with archive file" << endl;
      return;
    }
/*
    QPtrList<Event> es = archiveCalendar.getEvents(QDate(1800,1,1),
                                                  QDate(3000,1,1),
                                                  false);
    kdDebug() << "--Following events in archive calendar:" << endl;
    Event *e;
    for(e=es.first();e;e=es.next()) {
      kdDebug() << "-----Event: " << e->getSummary() << endl;
    }
*/
  } else {
    archiveFile = tmpFile.name();
  }

  // Save archive calendar
  if (!archiveCalendar.save(archiveFile)) {
    KMessageBox::error(this,i18n("Cannot write archive file."));
    return;
  }

  // Upload if necessary
  KURL srcUrl;
  srcUrl.setPath(archiveFile);
  if (srcUrl != destUrl) {
    if (!KIO::NetAccess::upload(archiveFile,destUrl)) {
      KMessageBox::error(this,i18n("Cannot write archive to final destination."));
      return;
    }
  }

  KOPrefs::instance()->mArchiveFile = destUrl.url();

  KIO::NetAccess::removeTempFile(archiveFile);

  // Delete archived events from calendar
  for(ev=events.first();ev;ev=events.next()) {
    mCalendar->deleteEvent(ev);
  }
  emit eventsDeleted();

  accept();
}

// Delete old events
void ArchiveDialog::slotUser2()
{
  QPtrList<Event> events = mCalendar->getEvents(QDate(1769,12,1),
                                             mDateEdit->date().addDays(-1),true);

  if (events.count() == 0) {
    KMessageBox::sorry(this,i18n("There are no events before %1")
        .arg(KGlobal::locale()->formatDate(mDateEdit->date())));
    return;
  }

  QStringList eventStrs;
  Event *ev;
  for(ev=events.first();ev;ev=events.next()) {
    eventStrs.append(ev->summary());
  }

  int result = KMessageBox::questionYesNoList(this,
      i18n("Delete all events before %1?\nThe following events will be deleted:")
      .arg(KGlobal::locale()->formatDate(mDateEdit->date())),eventStrs,
      i18n("Delete old events"),i18n("Delete"));
  if (result == KMessageBox::Yes) {
    for(ev=events.first();ev;ev=events.next()) {
      mCalendar->deleteEvent(ev);
    }
    emit eventsDeleted();
    accept();
  }
}
