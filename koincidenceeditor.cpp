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

KOIncidenceEditor::KOIncidenceEditor( const QString &caption,
                                      Calendar *calendar, QWidget *parent )
  : KDialogBase( Tabbed, caption, Ok | Apply | Cancel | Default | User1, Ok,
                 parent, 0, false, false ),
    mDetails( 0 ), mAttachments( 0 )
{
  mCalendar = calendar;

  setButtonText( Default, i18n("Load &Template...") );

  QString saveTemplateText;
  if ( KOPrefs::instance()->mCompactDialogs ) {
    showButton( User1, false );
    showButton( Apply, false );
  } else {
    saveTemplateText = i18n("&Save as Template...");
  }
  setButtonText( User1, saveTemplateText );

  mCategoryDialog = new KPIM::CategorySelectDialog( KOPrefs::instance(), this );
  KOGlobals::fitDialogToScreen( mCategoryDialog );

  connect( mCategoryDialog, SIGNAL( editCategories() ),
           SIGNAL( editCategories() ) );

  connect( this, SIGNAL( defaultClicked() ), SLOT( slotLoadTemplate() ) );
  connect( this, SIGNAL( user1Clicked() ), SLOT( slotSaveTemplate() ) );
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

void KOIncidenceEditor::slotLoadTemplate()
{
  kdDebug(5850) << "KOIncidenceEditor::loadTemplate()" << endl;
}

void KOIncidenceEditor::slotSaveTemplate()
{
  kdDebug(5850) << "KOIncidenceEditor::saveTemplate()" << endl;
  QString tp = type();
  QStringList templates;
  if ( tp == "Event" ) {
    templates = KOPrefs::instance()->mEventTemplates;
  } else if( tp == "ToDo" ) {
    templates = KOPrefs::instance()->mTodoTemplates;
  }
  bool ok = false;
  QString templateName = KInputDialog::getItem( i18n("Save Template"),
      i18n("Please enter a name for the template:"), templates, 
      -1, true, &ok, this );
  if ( ok && templateName.isEmpty() ) {
    KMessageBox::error( this, i18n("You did not give a valid template name, "
                                   "no template will be saved") );
    ok = false;
  }
  
  if ( ok && templates.contains( templateName ) ) {
    int res = KMessageBox::warningYesNo( this, 
                                         i18n("The selected template "
                                              "already exists. Overwrite it?"),
                                         i18n("Template already exists") );
    if ( res == KMessageBox::No ) {
      ok = false;
    }
  }
  
  if ( ok ) {
    saveTemplate( templateName );
    
    // Add template to list of existing templates
    if ( !templates.contains( templateName ) ) {
      templates.append( templateName );
      if ( tp == "Event" ) {
        KOPrefs::instance()->mEventTemplates = templates;
      } else if( tp == "ToDo" ) {
        KOPrefs::instance()->mTodoTemplates = templates;
      }
    }
    
  }
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

QString KOIncidenceEditor::loadTemplate( Calendar *cal, const QString &type,
                                         const QStringList &templates )
{
  bool ok = false;
  QString templateName = KInputDialog::getItem( i18n("Load Template"),
      i18n("Select a template to load:"), templates, 0, false, &ok, this );

  if ( !ok || templateName.isEmpty() ) return QString::null;

  QString fileName = locateLocal( "data", "korganizer/templates/" + type + "/" +
                                  templateName );

  if ( fileName.isEmpty() ) {
    KMessageBox::error( this, i18n("Unable to find template '%1'.")
                              .arg( fileName ) );
    return QString::null;
  } else {
    ICalFormat format;
    if ( !format.load( cal, fileName ) ) {
      KMessageBox::error( this, i18n("Error loading template file '%1'.")
                                .arg( fileName ) );
      return QString::null;
    }
  }

  return templateName;
}

#include "koincidenceeditor.moc"
