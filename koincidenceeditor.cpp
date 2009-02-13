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

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "koincidenceeditor.h"
#include "koprefs.h"
#include "koglobals.h"
#include "koeditordetails.h"
#include "koeditoralarms.h"
#include "urihandler.h"
#include "templatemanagementdialog.h"

#include <libkdepim/designerfields.h>
#include <libkdepim/embeddedurlpage.h>

#include <kabc/addressee.h>
#include <kcal/calendarlocal.h>
#include <kcal/incidence.h>
#include <kcal/icalformat.h>

#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <kinputdialog.h>
#include <kio/netaccess.h>

#include <QPixmap>
#include <QPointer>
#include <QLayout>
#include <QDateTime>
#include <QVBoxLayout>
#include <QBoxLayout>
#include <QList>

KOIncidenceEditor::KOIncidenceEditor( const QString &caption,
                                      Calendar *calendar, QWidget *parent )
  : KPageDialog( parent ),
    mAttendeeEditor( 0 ), mIsCounter( false )
{
  setFaceType( KPageDialog::Tabbed );
  setCaption( caption );
  setButtons( Ok | Apply | Cancel | Default );
  setDefaultButton( Ok );
  setModal( false );
  showButtonSeparator( false );

  // Set this to be the group leader for all subdialogs - this means
  // modal subdialogs will only affect this dialog, not the other windows
  setAttribute( Qt::WA_GroupLeader );

  mCalendar = calendar;

  if ( KOPrefs::instance()->mCompactDialogs ) {
    showButton( Apply, false );
    showButton( Default, false );
  } else {
    setButtonText( Default, i18n( "Manage &Templates..." ) );
  }

  connect( this, SIGNAL(defaultClicked()), SLOT(slotManageTemplates()) );
  connect( this, SIGNAL(finished()), SLOT(delayedDestruct()) );
}

KOIncidenceEditor::~KOIncidenceEditor()
{
}

void KOIncidenceEditor::slotButtonClicked( int button )
{
  switch( button ) {
  case KDialog::Ok:
  {
    // "this" can be deleted before processInput() returns (processInput()
    // opens a non-modal dialog when Kolab is used). So accept should only
    // be executed when "this" is still valid
    QPointer<QWidget> ptr( this );
    if ( processInput() && ptr ) {
      KDialog::accept();
    }
    break;
  }
  case KDialog::Apply:
    processInput();
    break;
  case KDialog::Cancel:
    if ( KMessageBox::questionYesNo(
           this,
           i18nc( "@info", "Do you really want to cancel?" ),
           i18nc( "@title:window", "KOrganizer Confirmation" ) ) == KMessageBox::Yes ) {
      processCancel();
      KDialog::reject();
    }
    break;
  default:
    KPageDialog::slotButtonClicked( button );
    break;
  }
}

void KOIncidenceEditor::setupAttendeesTab()
{
  QFrame *topFrame = new QFrame( this );
  addPage( topFrame, i18n( "Atte&ndees" ) );
  topFrame->setWhatsThis(
    i18n( "The Attendees tab allows you to Add or Remove "
          "Attendees to/from this event or to-do." ) );

  QBoxLayout *topLayout = new QVBoxLayout( topFrame );

  mAttendeeEditor = mDetails = new KOEditorDetails( spacingHint(), topFrame );
  topLayout->addWidget( mDetails );
}

void KOIncidenceEditor::accept()
{
}

void KOIncidenceEditor::reject()
{
}

void KOIncidenceEditor::closeEvent( QCloseEvent *event )
{
  event->ignore();
  slotButtonClicked( KDialog::Cancel );
}

void KOIncidenceEditor::cancelRemovedAttendees( Incidence *incidence )
{
  if ( !incidence ) {
    return;
  }

  // cancelAttendeeIncidence removes all attendees from the incidence,
  // and then only adds those that need to be canceled (i.e. a mail needs to be sent to them).
  if ( KOPrefs::instance()->thatIsMe( incidence->organizer().email() ) ) {
    Incidence *inc = incidence->clone();
    inc->registerObserver( 0 );
    mAttendeeEditor->cancelAttendeeIncidence( inc );
    if ( inc->attendeeCount() > 0 ) {
      emit deleteAttendee( inc );
    }
    delete inc;
  }

}

void KOIncidenceEditor::slotManageTemplates()
{
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

void KOIncidenceEditor::saveAsTemplate( Incidence *incidence, const QString &templateName )
{
  if ( !incidence || templateName.isEmpty() ) {
    return;
  }

  QString fileName = "templates/" + incidence->type();
  fileName.append( '/' + templateName );
  fileName = KStandardDirs::locateLocal( "data", "korganizer/" + fileName );

  CalendarLocal cal( KOPrefs::instance()->timeSpec() );
  cal.addIncidence( incidence );
  ICalFormat format;
  format.save( &cal, fileName );
}

void KOIncidenceEditor::slotLoadTemplate( const QString &templateName )
{
  CalendarLocal cal( KOPrefs::instance()->timeSpec() );
  QString fileName = KStandardDirs::locateLocal( "data", "korganizer/templates/" + type() + '/' +
      templateName );

  if ( fileName.isEmpty() ) {
    KMessageBox::error( this, i18n( "Unable to find template '%1'.", fileName ) );
  } else {
    ICalFormat format;
    if ( !format.load( &cal, fileName ) ) {
      KMessageBox::error( this, i18n( "Error loading template file '%1'.", fileName ) );
      return;
    }
  }
  loadTemplate( cal );
}

void KOIncidenceEditor::slotTemplatesChanged( const QStringList &newTemplates )
{
  templates() = newTemplates;
}

void KOIncidenceEditor::setupDesignerTabs( const QString &type )
{
  QStringList activePages = KOPrefs::instance()->activeDesignerFields();

  QStringList list = KGlobal::dirs()->findAllResources(
    "data",
    "korganizer/designer/" + type + "/*.ui",
    KStandardDirs::Recursive |KStandardDirs::NoDuplicates );

  for ( QStringList::iterator it = list.begin(); it != list.end(); ++it ) {
    const QString &fn = (*it).mid( (*it).lastIndexOf('/') + 1 );
    if ( activePages.contains( fn )  ) {
      addDesignerTab( *it );
    }
  }
}

QWidget *KOIncidenceEditor::addDesignerTab( const QString &uifile )
{
  KPIM::DesignerFields *wid = new KPIM::DesignerFields( uifile, 0 );
  mDesignerFields.append( wid );

  QFrame *topFrame = new QFrame();
  addPage( topFrame, wid->title() );

  QBoxLayout *topLayout = new QVBoxLayout( topFrame );

  wid->setParent( topFrame );
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
      for( it = props.constBegin(); it != props.constEnd(); ++it ) {
        QString customKey = it.key();
        QStringList parts = customKey.split( "-", QString::SkipEmptyParts );
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
  KCalStorage storage( i );
  foreach ( KPIM::DesignerFields *fields, mDesignerFields ) {
    if ( fields ) {
      fields->save( &storage );
    }
  }
}

void KOIncidenceEditor::setupEmbeddedURLPage( const QString &label,
                                 const QString &url, const QString &mimetype )
{
  QFrame *topFrame = new QFrame();
  addPage( topFrame, label );
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
  if ( !i ) return;
  if ( !mEmbeddedURLPages.isEmpty() ) {
    qDeleteAll( mEmbeddedURLPages );
    mEmbeddedURLPages.clear();
  }
  if ( !mAttachedDesignerFields.isEmpty() ) {
    for ( QList<QWidget*>::Iterator it = mAttachedDesignerFields.begin();
          it != mAttachedDesignerFields.end(); ++it ) {
      if ( mDesignerFieldForWidget.contains( *it ) ) {
        mDesignerFields.removeAll( mDesignerFieldForWidget[ *it ] );
      }
    }
    qDeleteAll( mAttachedDesignerFields );
    mAttachedDesignerFields.clear();
  }

  Attachment::List att = i->attachments();
  for ( Attachment::List::Iterator it = att.begin(); it != att.end(); ++it ) {
    Attachment *a = (*it);
    if ( a->showInline() && a->isUri() ) {
      // TODO: Allow more mime-types, but add security checks!
/*      if ( a->mimeType() == QLatin1String("application/x-designer") ) {
        QString tmpFile;
        if ( KIO::NetAccess::download( a->uri(), tmpFile, this ) ) {
          mAttachedDesignerFields.append( addDesignerTab( tmpFile ) );
          KIO::NetAccess::removeTempFile( tmpFile );
        }
      } else*/
      // TODO: Enable that check again!
      if ( a->mimeType() == QLatin1String( "text/html" ) ) {
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

void KOIncidenceEditor::addAttachments( const QStringList &attachments,
                                        const QStringList &mimeTypes,
                                        bool inlineAttachments )
{
  emit signalAddAttachments( attachments, mimeTypes, inlineAttachments );
}

void KOIncidenceEditor::addAttendees( const QStringList &attendees )
{
  QStringList::ConstIterator it;
  for ( it = attendees.begin(); it != attendees.end(); ++it ) {
    QString name, email;
    KABC::Addressee::parseEmailAddress( *it, name, email );
    mAttendeeEditor->insertAttendee( new Attendee( name, email ) );
  }
}

void KOIncidenceEditor::selectInvitationCounterProposal( bool enable )
{
  mIsCounter = enable;
  if ( mIsCounter ) {
    setCaption( i18n( "Counter proposal" ) );
    setButtonText( KDialog::Ok, i18n( "Counter proposal" ) );
    enableButtonApply( false );
  }
}

#include "koincidenceeditor.moc"
