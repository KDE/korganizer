/*
    This file is part of KOrganizer.
    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

// ArchiveDialog -- archive/delete past events.

#include <qlabel.h>
#include <qlayout.h>
#include <qdatetime.h>
#include <qcheckbox.h>
#include <qwhatsthis.h>
#include <qhgroupbox.h>

#include <kdebug.h>
#include <klocale.h>
#include <kurlrequester.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kurl.h>
#include <klineedit.h>
#include <kactivelabel.h>

#include <libkdepim/kdateedit.h>

#include "koprefs.h"

#include "archivedialog.h"
#include "eventarchiver.h"
#include <knuminput.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include "archivedialog.moc"

ArchiveDialog::ArchiveDialog(Calendar *cal,QWidget *parent, const char *name)
  : KDialogBase (Plain,i18n("Archive/Delete Past Events"),
                 User1|Cancel,User1,parent,name,false,true,
                 i18n("&Archive"))
{
  mCalendar = cal;

  QFrame *topFrame = plainPage();
  QVBoxLayout *topLayout = new QVBoxLayout(topFrame);
  topLayout->setSpacing(spacingHint());

  KActiveLabel *descLabel = new KActiveLabel(
    i18n("Archiving saves old events into the given file and "
         "then deletes them in the current calendar. If the archive file "
         "already exists they will be added. "
         "(<a href=\"whatsthis:In order to add an archive "
         "to your calendar, use the &quot;Merge Calendar&quot; function. "
         "You can view an archive by opening it in KOrganizer like any "
         "other calendar. It is not saved in a special format, but as "
         "vCalendar.\">How to restore</a>)"),
    topFrame);
  topLayout->addWidget(descLabel);

  QButtonGroup* radioBG = new QButtonGroup( this );
  radioBG->hide(); // just for the exclusive behavior
  connect( radioBG, SIGNAL( clicked( int ) ), SLOT( slotActionChanged() ) );

  QHBoxLayout *dateLayout = new QHBoxLayout(0);
  mArchiveOnceRB = new QRadioButton(i18n("Archive now events older than:"),topFrame);
  dateLayout->addWidget(mArchiveOnceRB);
  radioBG->insert(mArchiveOnceRB);
  mDateEdit = new KDateEdit(topFrame);
  QWhatsThis::add(mDateEdit,
    i18n("The date before which events should be archived. All older events will "
         "be saved and deleted, the newer (and events exactly on that date) will be kept."));
  dateLayout->addWidget(mDateEdit);
  topLayout->addLayout(dateLayout);

  // Checkbox, numinput and combo for auto-archiving
  // (similar to kmail's mExpireFolderCheckBox/mReadExpiryTimeNumInput in kmfolderdia.cpp)
  QHBox* autoArchiveHBox = new QHBox(topFrame);
  topLayout->addWidget(autoArchiveHBox);
  mAutoArchiveRB = new QRadioButton(i18n("Automaticall&y archive events older than:"), autoArchiveHBox);
  radioBG->insert(mAutoArchiveRB);
  QWhatsThis::add(mAutoArchiveRB,
    i18n("If this feature is enabled, KOrganizer will regularly check if events have to be archived; "
         "this means you will not need to use this dialog box again, except to change the settings."));

  mExpiryTimeNumInput = new KIntNumInput(autoArchiveHBox);
  mExpiryTimeNumInput->setRange(1, 500, 1, false);
  mExpiryTimeNumInput->setEnabled(false);
  mExpiryTimeNumInput->setValue(7);
  QWhatsThis::add(mExpiryTimeNumInput,
    i18n("The age of the events to archive. All older events "
         "will be saved and deleted, the newer will be kept."));

  mExpiryUnitsComboBox = new QComboBox(autoArchiveHBox);
  // Those items must match the "Expiry Unit" enum in the kcfg file!
  mExpiryUnitsComboBox->insertItem(i18n("Day(s)"));
  mExpiryUnitsComboBox->insertItem(i18n("Week(s)"));
  mExpiryUnitsComboBox->insertItem(i18n("Month(s)"));
  mExpiryUnitsComboBox->setEnabled(false);

  QHBoxLayout *fileLayout = new QHBoxLayout(0);
  fileLayout->setSpacing(spacingHint());
  QLabel *l = new QLabel(i18n("Archive &file:"),topFrame);
  fileLayout->addWidget(l);
  mArchiveFile = new KURLRequester(KOPrefs::instance()->mArchiveFile,topFrame);
  mArchiveFile->setMode(KFile::File);
  mArchiveFile->setFilter(i18n("*.ics|iCalendar Files"));
  QWhatsThis::add(mArchiveFile,
    i18n("The path of the archive. The events will be added to the "
         "archive file, so any events that are already in the file "
         "will not be modified or deleted. You can later load or merge the "
         "file like any other calendar. It is not saved in a special "
         "format, it uses the vCalendar format. "));
  l->setBuddy(mArchiveFile->lineEdit());
  fileLayout->addWidget(mArchiveFile);
  topLayout->addLayout(fileLayout);
  
  QHGroupBox *typeBox = new QHGroupBox( i18n("Type of incidences to archive"), 
                                        topFrame);
  mEvents = new QCheckBox( i18n("&Events"), typeBox );
  mTodos = new QCheckBox( i18n("&To-dos"), typeBox );
  topLayout->addWidget( typeBox );
  QWhatsThis::add( typeBox, i18n("Here you can select which incidences (events "
                   "or to-do items) should be archived. Events are archived if they "
                   "ended before the date given above; to-do items are archived if "
                   "they were finished before the date.") );

  mDeleteCb = new QCheckBox(i18n("&Delete only, do not save"),
                            topFrame);
  QWhatsThis::add(mDeleteCb,
    i18n("Select this option to delete old events without saving them. "
         "It is not possible to recover the events later."));
  topLayout->addWidget(mDeleteCb);
  connect(mDeleteCb, SIGNAL(toggled(bool)), mArchiveFile, SLOT(setDisabled(bool)));
  connect(mDeleteCb, SIGNAL(toggled(bool)), this, SLOT(slotEnableUser1()));
  connect(mArchiveFile->lineEdit(),SIGNAL(textChanged ( const QString & )),
          this,SLOT(slotEnableUser1()));

  // Load settings from KOPrefs
  mExpiryTimeNumInput->setValue( KOPrefs::instance()->mExpiryTime );
  mExpiryUnitsComboBox->setCurrentItem( KOPrefs::instance()->mExpiryUnit );
  mDeleteCb->setChecked( KOPrefs::instance()->mArchiveAction == KOPrefs::actionDelete );
  mEvents->setChecked( KOPrefs::instance()->mArchiveEvents );
  mTodos->setChecked( KOPrefs::instance()->mArchiveTodos );

  slotEnableUser1();

  // The focus should go to a useful field by default, not to the top richtext-label
  if ( KOPrefs::instance()->mAutoArchive ) {
    mAutoArchiveRB->setChecked( true );
    mAutoArchiveRB->setFocus();
  } else {
    mArchiveOnceRB->setChecked( true );
    mArchiveOnceRB->setFocus();
  }
  slotActionChanged();
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

void ArchiveDialog::slotActionChanged()
{
  mDateEdit->setEnabled( mArchiveOnceRB->isChecked() );
  mExpiryTimeNumInput->setEnabled( mAutoArchiveRB->isChecked() );
  mExpiryUnitsComboBox->setEnabled( mAutoArchiveRB->isChecked() );
}

// Archive old events
void ArchiveDialog::slotUser1()
{
  EventArchiver archiver;
  connect( &archiver, SIGNAL( eventsDeleted() ), this, SLOT( slotEventsDeleted() ) );

  KOPrefs::instance()->mAutoArchive = mAutoArchiveRB->isChecked();
  KOPrefs::instance()->mExpiryTime = mExpiryTimeNumInput->value();
  KOPrefs::instance()->mExpiryUnit = mExpiryUnitsComboBox->currentItem();

  if (mDeleteCb->isChecked()) {
    KOPrefs::instance()->mArchiveAction = KOPrefs::actionDelete;
  } else {
    KOPrefs::instance()->mArchiveAction = KOPrefs::actionArchive;

    // Get destination URL
    KURL destUrl( mArchiveFile->url() );
    if ( !destUrl.isValid() ) {
      KMessageBox::sorry(this,i18n("The archive file name is not valid.\n"));
      return;
    }
    // Force filename to be ending with vCalendar extension
    QString filename = destUrl.fileName();
    if (!filename.endsWith(".vcs") && !filename.endsWith(".ics")) {
      filename.append(".ics");
      destUrl.setFileName(filename);
    }

    KOPrefs::instance()->mArchiveFile = destUrl.url();
  }
  if ( KOPrefs::instance()->mAutoArchive ) {
    archiver.runAuto( mCalendar, this, true /*with gui*/ );
    emit autoArchivingSettingsModified();
    accept();
  }
  else
    archiver.runOnce( mCalendar, mDateEdit->date(), this );
}

void ArchiveDialog::slotEventsDeleted()
{
  emit eventsDeleted();
  if ( !KOPrefs::instance()->mAutoArchive )
    accept();
}
