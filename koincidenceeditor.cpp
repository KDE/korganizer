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

#include <libkdepim/categoryselectdialog.h>
#include <libkdepim/kinputdialog.h>

#include <libkcal/calendarlocal.h>
#include <libkcal/icalformat.h>

#include "koprefs.h"
#include "koglobals.h"

#include "koincidenceeditor.h"
#include "koincidenceeditor.moc"

KOIncidenceEditor::KOIncidenceEditor( const QString &caption, 
                                      Calendar *calendar, QWidget *parent ) :
  KDialogBase( Tabbed, caption, Ok | Apply | Cancel | Default | User1, Ok,
               parent, 0, false, false ),
  mSaveTemplateDialog( 0 )
{
  mCalendar = calendar;

  setButtonText( Default, i18n("Template...") );

  QString saveTemplateText;
  if ( KOPrefs::instance()->mCompactDialogs ) {
    showButton( User1, false );
    showButton( Apply, false );
  } else {
    saveTemplateText = i18n("Save as Template...");
  }
  setButtonText( User1, saveTemplateText );

  mCategoryDialog = new KPIM::CategorySelectDialog( KOPrefs::instance(), this );
  KOGlobals::fitDialogToScreen( mCategoryDialog );

  connect(mCategoryDialog,SIGNAL(editCategories()),SIGNAL(editCategories()));

  connect( this, SIGNAL( defaultClicked() ), SLOT( slotLoadTemplate() ) );
  connect( this, SIGNAL( user1Clicked() ), SLOT( slotSaveTemplate() ) );
}

KOIncidenceEditor::~KOIncidenceEditor()
{
  delete mCategoryDialog;
}

void KOIncidenceEditor::setupAttendeesTab()
{
  QFrame *topFrame = addPage(i18n("Attendees"));

  QBoxLayout *topLayout = new QVBoxLayout(topFrame);

  mDetails = new KOEditorDetails(spacingHint(),topFrame);
  topLayout->addWidget(mDetails);
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
  reject();
}

void KOIncidenceEditor::slotLoadTemplate()
{
  kdDebug(5850) << "KOIncidenceEditor::loadTemplate()" << endl;
}

void KOIncidenceEditor::slotSaveTemplate()
{
  kdDebug(5850) << "KOIncidenceEditor::saveTemplate()" << endl;
}

void KOIncidenceEditor::createSaveTemplateDialog( SaveTemplateDialog::IncidenceType type )
{
  if ( !mSaveTemplateDialog ) {
    mSaveTemplateDialog = new SaveTemplateDialog( type, this );
    connect( mSaveTemplateDialog, SIGNAL( templateSelected( const QString & ) ),
             SLOT( saveTemplate( const QString & ) ) );
  }
  mSaveTemplateDialog->show();
  mSaveTemplateDialog->raise();
}

void KOIncidenceEditor::saveAsTemplate( Incidence *incidence,
                                        const QString &templateName )
{
  if ( !incidence || templateName.isEmpty() ) return;

  QString fileName = "templates/" + incidence->type();
  fileName.append( "/" + templateName );
  fileName = locateLocal( "data", "korganizer/" + fileName );

  CalendarLocal cal;
  cal.addIncidence( incidence );
  ICalFormat format;
  format.save( &cal, fileName );
}

QString KOIncidenceEditor::loadTemplate( Calendar *cal, const QString &type,
                                         const QStringList &templates )
{
  bool ok = false;
  QString templateName = KInputDialog::getItem( i18n("Load Template"),
      i18n("Select a template to load:"), templates, 0, &ok );
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
