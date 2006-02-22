/*
    This file is part of KOrganizer.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qtooltip.h>
#include <q3frame.h>
#include <qpixmap.h>
#include <qlayout.h>
#include <q3widgetstack.h>
#include <qdatetime.h>

#include <QVBoxLayout>
#include <QBoxLayout>
#include <QList>

#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <kinputdialog.h>
#include <kio/netaccess.h>

#include <libkdepim/categoryselectdialog.h>
#include <libkdepim/designerfields.h>
#include <libkdepim/embeddedurlpage.h>

#include <libkcal/calendarlocal.h>
#include <libkcal/incidence.h>
#include <libkcal/icalformat.h>

#include "koprefs.h"
#include "koglobals.h"
#include "koeditordetails.h"
#include "koeditorattachments.h"
#include "koeditoralarms.h"
#include "urihandler.h"
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
  setAttribute( Qt::WA_GroupLeader ); 

  mCalendar = calendar;

  if ( KOPrefs::instance()->mCompactDialogs ) {
    showButton( Apply, false );
    showButton( Default, false );
  } else {
    setButtonText( Default, i18n("&Templates...") );
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
  topFrame->setWhatsThis(
                   i18n("The Attendees tab allows you to Add or Remove "
                        "Attendees to/from this event or to-do.") );

  QBoxLayout *topLayout = new QVBoxLayout( topFrame );

  mDetails = new KOEditorDetails( spacingHint(), topFrame );
  topLayout->addWidget( mDetails );
}

void KOIncidenceEditor::setupAttachmentsTab()
{
  QFrame *topFrame = addPage( i18n("Attach&ments") );
  topFrame->setWhatsThis(
                   i18n("The Attachments tab allows you to add or remove "
                        "files, emails, contacts, and other items "
                        "associated with this event or to-do.") );

  QBoxLayout *topLayout = new QVBoxLayout( topFrame );

  mAttachments = new KOEditorAttachments( spacingHint(), topFrame );
  connect( mAttachments, SIGNAL( openURL( const KUrl & ) ) ,
           this, SLOT( openURL( const KUrl & ) ) );
  topLayout->addWidget( mAttachments );
}

void KOIncidenceEditor::slotApply()
{
  if ( mAttachments )
    mAttachments->applyChanges();
  processInput();
}

void KOIncidenceEditor::slotOk()
{
  if ( mAttachments )
    mAttachments->applyChanges();
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
  kDebug(5850) << "KOIncidenceEditor::manageTemplates()" << endl;

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

void KOIncidenceEditor::setupDesignerTabs( const QString &type )
{
  QStringList activePages = KOPrefs::instance()->activeDesignerFields();

  QStringList list = KGlobal::dirs()->findAllResources( "data",
    "korganizer/designer/" + type + "/*.ui", true, true );
  for ( QStringList::iterator it = list.begin(); it != list.end(); ++it ) {
    const QString &fn = (*it).mid( (*it).findRev('/') + 1 );
    if ( activePages.find( fn ) != activePages.end() ) {
      addDesignerTab( *it );
    }
  }
}

QWidget *KOIncidenceEditor::addDesignerTab( const QString &uifile )
{
  kDebug() << "Designer tab: " << uifile << endl;

  KPIM::DesignerFields *wid = new KPIM::DesignerFields( uifile, 0 );
  mDesignerFields.append( wid );

  QFrame *topFrame = addPage( wid->title() );

  QBoxLayout *topLayout = new QVBoxLayout( topFrame );

  wid->reparent( topFrame, 0, QPoint() );
  topLayout->addWidget( wid );
  mDesignerFieldForWidget[ topFrame ] = wid;

  return topFrame;
}

class KCalStorage : public KPIM::DesignerFields::Storage
{
  public:
    KCalStorage( Incidence *incidence )
      : mIncidence( incidence )
    {
    }

    QStringList keys()
    {
      QStringList keys;

      QMap<QByteArray, QString> props = mIncidence->customProperties();
      QMap<QByteArray, QString>::ConstIterator it;
      for( it = props.begin(); it != props.end(); ++it ) {
        QString customKey = it.key();
        QStringList parts = QStringList::split( "-", customKey );
        if ( parts.count() != 4 ) continue;
        if ( parts[ 2 ] != "KORGANIZER" ) continue;
        keys.append( parts[ 3 ] );
      }

      return keys;
    }

    QString read( const QString &key )
    {
      return mIncidence->customProperty( "KORGANIZER", key.toUtf8() );
    }

    void write( const QString &key, const QString &value )
    {
      mIncidence->setCustomProperty( "KORGANIZER", key.toUtf8(), value );
    }

  private:
    Incidence *mIncidence;
};

void KOIncidenceEditor::readDesignerFields( Incidence *i )
{
  KCalStorage storage( i );
  foreach ( KPIM::DesignerFields *fields, mDesignerFields ) {
    if ( fields )
      fields->load( &storage );
  }
}

void KOIncidenceEditor::writeDesignerFields( Incidence *i )
{
  kDebug() << "KOIncidenceEditor::writeDesignerFields()" << endl;

  KCalStorage storage( i );
  foreach ( KPIM::DesignerFields *fields, mDesignerFields ) {
    if ( fields ) {
      kDebug() << "Write Field " << fields->title() << endl;
      fields->save( &storage );
    }
  }
}


void KOIncidenceEditor::setupEmbeddedURLPage( const QString &label,
                                 const QString &url, const QString &mimetype )
{
  kDebug() << "KOIncidenceEditor::setupEmbeddedURLPage()" << endl;
  kDebug() << "label=" << label << ", url=" << url << ", mimetype=" << mimetype << endl;
  QFrame *topFrame = addPage( label );
  QBoxLayout *topLayout = new QVBoxLayout( topFrame );

  KPIM::EmbeddedURLPage *wid = new KPIM::EmbeddedURLPage( url, mimetype,
                                                          topFrame );
  topLayout->addWidget( wid );
  mEmbeddedURLPages.append( topFrame );
  connect( wid, SIGNAL( openURL( const KUrl & ) ) ,
           this, SLOT( openURL( const KUrl & ) ) );
  // TODO: Call this method only when the tab is actually activated!
  wid->loadContents();
}

void KOIncidenceEditor::createEmbeddedURLPages( Incidence *i )
{
  kDebug() << "KOIncidenceEditor::createEmbeddedURLPages()" << endl;

  if ( !i ) return;
  if ( !mEmbeddedURLPages.isEmpty() ) {
kDebug() << "mEmbeddedURLPages are not empty, clearing it!" << endl;
    qDeleteAll( mEmbeddedURLPages );
    mEmbeddedURLPages.clear();
  }
  if ( !mAttachedDesignerFields.isEmpty() ) {
    for ( QList<QWidget*>::Iterator it = mAttachedDesignerFields.begin();
          it != mAttachedDesignerFields.end(); ++it ) {
      if ( mDesignerFieldForWidget.contains( *it ) ) {
        mDesignerFields.remove( mDesignerFieldForWidget[ *it ] );
      }
    }
    qDeleteAll( mAttachedDesignerFields );
    mAttachedDesignerFields.clear();
  }

  Attachment::List att = i->attachments();
  for ( Attachment::List::Iterator it = att.begin(); it != att.end(); ++it ) {
    Attachment *a = (*it);
    kDebug() << "Iterating over the attachments " << endl;
    kDebug() << "label=" << a->label() << ", url=" << a->uri() << ", mimetype=" << a->mimeType() << endl;
    if ( a && a->showInline() && a->isUri() ) {
      // TODO: Allow more mime-types, but add security checks!
/*      if ( a->mimeType() == "application/x-designer" ) {
        QString tmpFile;
        if ( KIO::NetAccess::download( a->uri(), tmpFile, this ) ) {
          mAttachedDesignerFields.append( addDesignerTab( tmpFile ) );
          KIO::NetAccess::removeTempFile( tmpFile );
        }
      } else*/
      // TODO: Enable that check again!
      if ( a->mimeType() == "text/html" )
      {
        setupEmbeddedURLPage( a->label(), a->uri(), a->mimeType() );
      }
    }
  }
}

void KOIncidenceEditor::openURL( const KUrl &url )
{
  QString uri = url.url();
  UriHandler::process( uri );
}

#include "koincidenceeditor.moc"
