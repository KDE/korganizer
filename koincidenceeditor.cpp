/*
    This file is part of KOrganizer.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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

#include <qtooltip.h>
#include <qframe.h>
#include <qpixmap.h>
#include <qlayout.h>
#include <qwidgetstack.h>
#include <qdatetime.h>

#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <kinputdialog.h>

#include <libkdepim/categoryselectdialog.h>

#include <libkcal/calendarlocal.h>
#include <libkcal/incidence.h>
#include <libkcal/icalformat.h>

#include "koprefs.h"
#include "koglobals.h"
#include "koeditordetails.h"
#include "koeditorattachments.h"

#include "koincidenceeditor.h"
#include "templatemanagementdialog.h"

KOIncidenceEditor::KOIncidenceEditor( const QString &caption,
                                      Calendar *calendar, QWidget *parent )
  : KDialogBase( Tabbed, caption, Ok | Apply | Cancel | Default, Ok,
                 parent, 0, false, false ),
    mDetails( 0 ), mAttachments( 0 )
{
  // Set this to be the group leader for all subdialogs - this means
  // modal subdialogs will only affect this dialog, not the other windows
  setWFlags( getWFlags() | WGroupLeader );

  mCalendar = calendar;

  if ( KOPrefs::instance()->mCompactDialogs ) {
    showButton( Apply, false );
    showButton( Default, false );
  } else {
    setButtonText( Default, "&Templates..." );
  }

  mCategoryDialog = new KPIM::CategorySelectDialog( KOPrefs::instance(), this );
  KOGlobals::fitDialogToScreen( mCategoryDialog );

  connect( mCategoryDialog, SIGNAL( editCategories() ),
           SIGNAL( editCategories() ) );

  connect( this, SIGNAL( defaultClicked() ), SLOT( slotManageTemplates() ) );
  connect( this, SIGNAL( finished() ), SLOT( delayedDestruct() ) );
}

KOIncidenceEditor::~KOIncidenceEditor()
{
  delete mCategoryDialog;
}

void KOIncidenceEditor::setupAttendeesTab()
{
  QFrame *topFrame = addPage( i18n("Atte&ndees") );

  QBoxLayout *topLayout = new QVBoxLayout( topFrame );

  mDetails = new KOEditorDetails( spacingHint(), topFrame );
  topLayout->addWidget( mDetails );
}

void KOIncidenceEditor::setupAttachmentsTab()
{
  QFrame *topFrame = addPage( i18n("Attachments") );

  QBoxLayout *topLayout = new QVBoxLayout( topFrame );

  mAttachments = new KOEditorAttachments( spacingHint(), topFrame );
  topLayout->addWidget( mAttachments );
}

void KOIncidenceEditor::slotApply()
{
  processInput();
}

void KOIncidenceEditor::slotOk()
{
  if ( processInput() ) accept();
}

void KOIncidenceEditor::updateCategoryConfig()
{
  mCategoryDialog->updateCategoryConfig();
}

void KOIncidenceEditor::slotCancel()
{
  processCancel();
  reject();
}

void KOIncidenceEditor::cancelRemovedAttendees( Incidence *incidence )
{
  if ( !incidence ) return;

  // cancelAttendeeEvent removes all attendees from the incidence,
  // and then only adds those that need to be cancelled (i.e. a mail needs to be sent to them).
  if ( KOPrefs::instance()->thatIsMe( incidence->organizer().email() ) ) {
    Incidence *ev = incidence->clone();
    ev->registerObserver( 0 );
    mDetails->cancelAttendeeEvent( ev );
    if ( ev->attendeeCount() > 0 ) {
      emit deleteAttendee( ev );
    }
    delete( ev );
  }

}

void KOIncidenceEditor::slotManageTemplates()
{
  kdDebug(5850) << "KOIncidenceEditor::manageTemplates()" << endl;

  QString tp = type();

  TemplateManagementDialog * const d = new TemplateManagementDialog( this, templates() );
  connect( d, SIGNAL( loadTemplate( const QString& ) ),
           this, SLOT( slotLoadTemplate( const QString& ) ) );
  connect( d, SIGNAL( templatesChanged( const QStringList& ) ),
           this, SLOT( slotTemplatesChanged( const QStringList& ) ) );
  connect( d, SIGNAL( saveTemplate( const QString& ) ),
           this, SLOT( slotSaveTemplate( const QString& ) ) );
  d->exec();
  return;
}

void KOIncidenceEditor::saveAsTemplate( Incidence *incidence,
                                        const QString &templateName )
{
  if ( !incidence || templateName.isEmpty() ) return;

  QString fileName = "templates/" + incidence->type();
  fileName.append( "/" + templateName );
  fileName = locateLocal( "data", "korganizer/" + fileName );

  CalendarLocal cal( KOPrefs::instance()->mTimeZoneId );
  cal.addIncidence( incidence );
  ICalFormat format;
  format.save( &cal, fileName );
}

void KOIncidenceEditor::slotLoadTemplate( const QString& templateName )
{
  CalendarLocal cal( KOPrefs::instance()->mTimeZoneId );
  QString fileName = locateLocal( "data", "korganizer/templates/" + type() + "/" +
      templateName );

  if ( fileName.isEmpty() ) {
    KMessageBox::error( this, i18n("Unable to find template '%1'.")
                              .arg( fileName ) );
  } else {
    ICalFormat format;
    if ( !format.load( &cal, fileName ) ) {
      KMessageBox::error( this, i18n("Error loading template file '%1'.")
          .arg( fileName ) );
      return;
    }
  }
  loadTemplate( cal );
}

void KOIncidenceEditor::slotTemplatesChanged( const QStringList& newTemplates )
{
  templates() = newTemplates;
}

#include "koincidenceeditor.moc"
