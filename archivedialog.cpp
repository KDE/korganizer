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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

// ArchiveDialog -- archive/delete past appointments.

#include <qlabel.h>
#include <qlayout.h>
#include <qdatetime.h>
#include <qcheckbox.h>
#include <qwhatsthis.h>

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
#include <kactivelabel.h>

#include <libkcal/event.h>
#include <libkcal/calendar.h>
#include <libkcal/calendarlocal.h>
#include <libkcal/filestorage.h>

#include <libkdepim/kdateedit.h>

#include "koprefs.h"

#include "archivedialog.h"
#include "archivedialog.moc"

ArchiveDialog::ArchiveDialog(Calendar *cal,QWidget *parent, const char *name)
  : KDialogBase (Plain,i18n("Archive/Delete Past Appointments"),
                 User1|Cancel,User1,parent,name,false,true,
                 i18n("&Archive"))
{
  mCalendar = cal;

  QFrame *topFrame = plainPage();
  QVBoxLayout *topLayout = new QVBoxLayout(topFrame);
  topLayout->setSpacing(spacingHint());

  KActiveLabel *descLabel = new KActiveLabel(
    i18n("Archiving saves old appointments into the given file and "
         "then deletes them in the current calendar. If the archive file "
         "already exists they will be added. "
         "(<a href=\"whatsthis:In order to add an archive "
         "to your calendar, use the &quot;Merge Calendar&quot; function. "
         "You can view an archive by opening it in KOrganizer like any "
         "other calendar. It is not saved in a special format, but as "
         "vCalendar.\">How to restore</a>)"),
    topFrame);
  topLayout->addWidget(descLabel);

  QHBoxLayout *dateLayout = new QHBoxLayout(0);
  QLabel *dateLabel = new QLabel(i18n("A&ppointments older than:"),topFrame);
  dateLayout->addWidget(dateLabel);
  mDateEdit = new KDateEdit(topFrame);
  QWhatsThis::add(mDateEdit, 
    i18n("The age of the appointments to archive. All older appointments "
         "will be saved and deleted, the newer will be kept."));
  dateLabel->setBuddy(mDateEdit);
  dateLayout->addWidget(mDateEdit);  
  topLayout->addLayout(dateLayout);

  QHBoxLayout *fileLayout = new QHBoxLayout(0);
  fileLayout->setSpacing(spacingHint());
  QLabel *l = new QLabel(i18n("Archive &file:"),topFrame);
  fileLayout->addWidget(l);
  mArchiveFile = new KURLRequester(KOPrefs::instance()->mArchiveFile,topFrame);
  mArchiveFile->setMode(KFile::File);
  mArchiveFile->setFilter(i18n("*.vcs|vCalendar Files"));
  QWhatsThis::add(mArchiveFile, 
    i18n("The path of the archive. The appointments will be added to the "
         "archive file, so any appointments that are already in the file "
         "will not be modified or deleted. You can later load or merge the "
         "file like any other calendar. It is not saved in a special "
         "format, it uses the vCalendar format. "));
  l->setBuddy(mArchiveFile->lineEdit());
  fileLayout->addWidget(mArchiveFile);
  topLayout->addLayout(fileLayout);
  
  mDeleteCb = new QCheckBox(i18n("&Delete only, do not save"),
                            topFrame);
  QWhatsThis::add(mDeleteCb,  
    i18n("Select this option to delete old appointments without saving them."
         "It is not possible to recover the appointments later."));
  topLayout->addWidget(mDeleteCb);
  connect(mDeleteCb, SIGNAL(toggled(bool)), mArchiveFile, SLOT(setDisabled(bool)));
  connect(mDeleteCb, SIGNAL(toggled(bool)), this, SLOT(slotEnableUser1()));
  connect(mArchiveFile->lineEdit(),SIGNAL(textChanged ( const QString & )),
          this,SLOT(slotEnableUser1()));
  enableButton(KDialogBase::User1,!mArchiveFile->lineEdit()->text().isEmpty());
}

ArchiveDialog::~ArchiveDialog()
{
}

void ArchiveDialog::slotEnableUser1()
{
  bool state = ( mDeleteCb->isChecked() ||
                 !mArchiveFile->lineEdit()->text().isEmpty() );
  enableButton(KDialogBase::User1,state);
}

// Archive old events
void ArchiveDialog::slotUser1()
{
  if (mDeleteCb->isChecked()) {
    deleteOldEvents();
    return;
  }

  // Get destination URL
  KURL destUrl = mArchiveFile->url();
  if ( !destUrl.isValid() ) {
    KMessageBox::sorry(this,i18n("The archive file name is not valid.\n"));
    return;
  }
  // Force filename to be ending with vCalendar extension
  QString filename = destUrl.fileName();
  if (filename.right(4) != ".vcs" && filename.right(4) != ".ics") {
    filename.append(".ics");
    destUrl.setFileName(filename);
  }

  // Get events to be archived
  Event::List events = mCalendar->events( QDate( 1800, 1, 1 ),
                                          mDateEdit->date().addDays( -1 ),
                                          true );
  if ( events.count() == 0 ) {
    KMessageBox::sorry(this,i18n("There are no events before %1")
        .arg(KGlobal::locale()->formatDate(mDateEdit->date())));
    return;
  }

  FileStorage storage( mCalendar );

  // Save current calendar to disk
  KTempFile tmpFile;
  tmpFile.setAutoDelete(true);
  storage.setFileName( tmpFile.name() );
  if ( !storage.save() ) {
    kdDebug(5850) << "ArchiveDialog::slotUser1(): Can't save calendar to temp file" << endl;
    return;
  }

  // Duplicate current calendar by loading in new calendar object
  CalendarLocal archiveCalendar( KOPrefs::instance()->mTimeZoneId );

  FileStorage archiveStore( &archiveCalendar );
  archiveStore.setFileName( tmpFile.name() );
  if (!archiveStore.load()) {
    kdDebug(5850) << "ArchiveDialog::slotUser1(): Can't load calendar from temp file" << endl;
    return;
  }

  // Strip active events from calendar so that only events to be archived
  // remain.
  Event::List activeEvents = archiveCalendar.events( mDateEdit->date(),
                                                     QDate( 3000, 1, 1 ),
                                                     false );
  Event::List::ConstIterator it;
  for( it = activeEvents.begin(); it != activeEvents.end(); ++it ) {
    archiveCalendar.deleteEvent( *it );
  }

  // Get or create the archive file
  QString archiveFile;

  if ( KIO::NetAccess::exists( destUrl, true, this ) ) {
    if( !KIO::NetAccess::download( destUrl, archiveFile, this ) ) {
      kdDebug(5850) << "ArchiveDialog::slotUser1(): Can't download archive file" << endl;
      return;
    }
    // Merge with events to be archived.
    archiveStore.setFileName( archiveFile );
    if ( !archiveStore.load() ) {
      kdDebug(5850) << "ArchiveDialog::slotUser1(): Can't merge with archive file" << endl;
      return;
    }
/*
    QPtrList<Event> es = archiveCalendar.events(QDate(1800,1,1),
                                                QDate(3000,1,1),
                                                false);
    kdDebug(5850) << "--Following events in archive calendar:" << endl;
    Event *e;
    for(e=es.first();e;e=es.next()) {
      kdDebug(5850) << "-----Event: " << e->getSummary() << endl;
    }
*/
  } else {
    archiveFile = tmpFile.name();
  }

  // Save archive calendar
  if ( !archiveStore.save() ) {
    KMessageBox::error(this,i18n("Cannot write archive file."));
    return;
  }

  // Upload if necessary
  KURL srcUrl;
  srcUrl.setPath(archiveFile);
  if (srcUrl != destUrl) {
    if ( !KIO::NetAccess::upload( archiveFile, destUrl, this ) ) {
      KMessageBox::error(this,i18n("Cannot write archive to final destination."));
      return;
    }
  }

  KOPrefs::instance()->mArchiveFile = destUrl.url();

  KIO::NetAccess::removeTempFile(archiveFile);

  // Delete archived events from calendar
  for( it = events.begin(); it != events.end(); ++it ) {
    mCalendar->deleteEvent( *it );
  }
  emit eventsDeleted();

  accept();
}

// Delete old events
void ArchiveDialog::deleteOldEvents()
{
  Event::List events = mCalendar->events( QDate( 1769, 12, 1 ),
                                          mDateEdit->date().addDays( -1 ),
                                          true );

  if ( events.count() == 0 ) {
    KMessageBox::sorry(this,i18n("There are no events before %1")
        .arg(KGlobal::locale()->formatDate(mDateEdit->date())));
    return;
  }

  QStringList eventStrs;
  Event::List::ConstIterator it;
  for( it = events.begin(); it != events.end(); ++it ) {
    eventStrs.append( (*it)->summary() );
  }

  int result = KMessageBox::warningContinueCancelList(this,
      i18n("Delete all events before %1 without saving?\n"
           "The following events will be deleted:")
      .arg(KGlobal::locale()->formatDate(mDateEdit->date())),eventStrs,
      i18n("Delete old events"),i18n("&Delete"));
  if (result == KMessageBox::Continue) {
    for( it = events.begin(); it != events.end(); ++it ) {
      mCalendar->deleteEvent( *it );
    }
    emit eventsDeleted();
    accept();
  }
}
