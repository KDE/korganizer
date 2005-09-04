/*
    This file is part of KOrganizer.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2005 Reinhold Kainhofer <reinhold@kainhofer.com>
    Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>

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

#include "koeditorattachments.h"

#include <libkcal/incidence.h>
#include <libkdepim/kdepimprotocols.h>
#include <libkdepim/kpimurlrequesterdlg.h>
#include <libkdepim/kvcarddrag.h>

#include <kio/job.h>
#include <kio/jobclasses.h>

#include <kapplication.h>
#include <klocale.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kiconview.h>
#include <klineedit.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <kiconview.h>
#include <kpopupmenu.h>
#include <kprotocolinfo.h>
#include <krecentdocument.h>
#include <krun.h>
#include <kseparator.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kurldrag.h>
#include <kurlrequester.h>

#include <qcursor.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <q3dragobject.h>
#include <qregexp.h>
#include <qstringlist.h>
#include <qstyle.h>
#include <q3whatsthis.h>
//Added by qt3to4:
#include <QPixmap>
#include <QGridLayout>
#include <QBoxLayout>
#include <QDropEvent>
#include <QHBoxLayout>
#include <Q3ValueList>
#include <QVBoxLayout>
#include <QDragEnterEvent>

class AttachmentIconItem : public KIconViewItem
{
  public:
    AttachmentIconItem( KCal::Attachment*att, Q3IconView *parent ) :
        KIconViewItem( parent )
    {
      if ( att ) {
        mAttachment = new KCal::Attachment( *att );
      } else {
        mAttachment = new KCal::Attachment( QString::null );
      }
      readAttachment();
    }
    ~AttachmentIconItem() { delete mAttachment; }
    KCal::Attachment *attachment() const { return mAttachment; }

    const QString uri() const
    {
      return mAttachment->uri();
    }
    void setUri( const QString &uri )
    {
      mAttachment->setUri( uri );
      readAttachment();
    }
    void setData( const QByteArray &data )
    {
      mAttachment->setDecodedData( data );
      readAttachment();
    }
    const QString mimeType() const
    {
      return mAttachment->mimeType();
    }
    void setMimeType( const QString &mime )
    {
      mAttachment->setMimeType( mime );
      readAttachment();
    }
    const QString label() const
    {
      return mAttachment->label();
    }
    void setLabel( const QString &description )
    {
      mAttachment->setLabel( description );
      readAttachment();
    }
    const bool isLocal() const
    {
      return mAttachment->isLocal();
    }
    void setLocal( bool local )
    {
      mAttachment->setLocal( local );
      readAttachment();
    }
    QPixmap icon() const
    {
      return icon( KMimeType::mimeType( mAttachment->mimeType() ), 
                   mAttachment->uri(), mAttachment->isLocal() );
    }
    static QPixmap icon( KMimeType::Ptr mimeType, const QString &uri, 
                         bool local = false )
    {
      QString iconStr = mimeType->icon( uri, false );
      return KGlobal::iconLoader()->loadIcon( iconStr, KIcon::Desktop, 0, 
                                              KIcon::DefaultState | ( 
                                                ( uri.isNull() || local ) ? 0 : 
                                                  KIcon::LinkOverlay) );
    }

    void readAttachment()
    {
      if ( mAttachment->label().isEmpty() )
        setText( mAttachment->uri() );
      else
        setText( mAttachment->label() );
      
      setRenameEnabled( true );
      
      KMimeType::Ptr mimeType;
      if ( !mAttachment->mimeType().isEmpty() )
        mimeType = KMimeType::mimeType( mAttachment->mimeType() );
      else {
        if ( mAttachment->isUri() )
          mimeType = KMimeType::findByURL( mAttachment->uri() );
        else
          mimeType = KMimeType::findByContent( mAttachment->decodedData() );
        mAttachment->setMimeType( mimeType->name() );
      }
      
      setPixmap( icon() );
    }
    
  private:
    KCal::Attachment *mAttachment;
};

AttachmentEditDialog::AttachmentEditDialog( AttachmentIconItem *item,
                                            QWidget *parent, const char *name,
                                            bool modal )
  : KDialogBase ( KDialogBase::Plain,
                  i18n( "Properties for %1" ).arg( item->label().isEmpty()
                      ? item->uri() : item->label() ),
                  KDialogBase::Ok | KDialogBase::Cancel | KDialogBase::Apply,
                  KDialogBase::Ok, parent, name, modal ), mItem( item ),
mURLRequester( 0 )
{
  // based loosely on KPropertiesDialog code
  QWidget *page = plainPage();
  QVBoxLayout *vbl = new QVBoxLayout( page, 0, KDialog::spacingHint() );
  QGridLayout *grid = new QGridLayout(0, 3);
  grid->setColStretch(0, 0);
  grid->setColStretch(1, 0);
  grid->setColStretch(2, 1);
  grid->addColSpacing(1, KDialog::spacingHint());
  vbl->addLayout(grid);
  
  mIcon = new QLabel( page );
  int bsize = 66 + 2 * 
                  mIcon->style().pixelMetric( QStyle::PM_ButtonMargin );
  mIcon->setFixedSize(bsize, bsize);
  mIcon->setPixmap( item->icon() );
  grid->addWidget( mIcon, 0, 0, AlignLeft );
  
  mLabelEdit = new KLineEdit( page );
  mLabelEdit->setText( item->label().isEmpty() ? item->uri() :
                                                  item->label() );
  grid->addWidget( mLabelEdit, 0, 2 );
  
  KSeparator* sep = new KSeparator( KSeparator::HLine, page );
  grid->addMultiCellWidget(sep, 1, 1, 0, 2);
  
  QLabel *label = new QLabel( i18n( "Type:" ), page );
  grid->addWidget( label, 2, 0 );
  QString typecomment = item->mimeType().isEmpty() ? i18n( "Unknown" ) : 
                             KMimeType::mimeType( item->mimeType() )->comment();
  mTypeLabel = new QLabel( typecomment, page );
  grid->addWidget( mTypeLabel, 2, 2 );
  mMimeType = KMimeType::mimeType( item->mimeType() );
  
  if ( item->attachment()->isUri() && !item->isLocal() ) {
    label = new QLabel( i18n( "Location:" ), page );
    grid->addWidget( label, 3, 0 );
    mURLRequester = new KURLRequester( item->uri(), page );
    grid->addWidget( mURLRequester, 3, 2 );
    connect( mURLRequester, SIGNAL( urlSelected( const QString & ) ), 
              SLOT( urlChanged( const QString & ) ) );
  } else {
    if ( item->isLocal() ) {
      grid->addWidget( new QLabel( i18n( "Size:" ), page ), 3, 0 );
      uint size = QFileInfo( KURL( item->uri() ).path() ).size();
      grid->addWidget( new QLabel( QString::fromLatin1( "%1 (%2)" )
                         .arg( KIO::convertSize( size ) ) 
                         .arg( KGlobal::locale()->formatNumber( 
                                                    size, 0 ) ), page ), 3, 2 );
    } else {
      grid->addMultiCellWidget( new QLabel( i18n( 
                    "Binary attachment, not supported." ), page ), 3, 3, 0, 2 );
    }
#if 0
    grid->addWidget( new QLabel( QString::fromLatin1( "%1 (%2)" )
                         .arg( KIO::convertSize( item->attachment()->size() ) ) 
                         .arg( KGlobal::locale()->formatNumber( 
                              item->attachment()->size(), 0 ) ), page ), 3, 2 );
#endif
  }
  vbl->addStretch( 10 );
}
    
void AttachmentEditDialog::slotApply()
{
  mItem->setLabel( mLabelEdit->text() );
  mItem->setMimeType( mMimeType->name() );
  if ( mURLRequester )
    mItem->setUri( mURLRequester->url() );
}

void AttachmentEditDialog::accept()
{
  slotApply();
  KDialogBase::accept();
}
  
void AttachmentEditDialog::urlChanged( const QString &url )
{
  mMimeType = KMimeType::findByURL( url );
  mTypeLabel->setText( mMimeType->comment() );
  mIcon->setPixmap( AttachmentIconItem::icon( mMimeType, url ) );
}

class AttachmentIconView : public KIconView
{
public:
  AttachmentIconView( QWidget *parent ) : KIconView( parent ) {}
  
protected:
  virtual Q3DragObject * dragObject ()
  {
    // create a list of the URL:s that we want to drag
    KURL::List urls;
    QStringList labels;
    for ( Q3IconViewItem *it = firstItem() ; it; it = it->nextItem() ) {
      if ( it->isSelected() ) {
        AttachmentIconItem *item = static_cast<AttachmentIconItem *>( it );
        urls.append( item->uri() );
        labels.append( KURL::encode_string( item->label() ) );
      }
    }
    if ( selectionMode() == Q3IconView::NoSelection ) {
      AttachmentIconItem *item =
                             static_cast<AttachmentIconItem *>( currentItem() );
      if ( item ) {
        urls.append( item->uri() );
        labels.append( KURL::encode_string( item->label() ) );
      }
    }
    QPixmap pixmap;
    if( urls.count() > 1 )
        pixmap = KGlobal::iconLoader()->loadIcon( "kmultiple", KIcon::Desktop );
    if( pixmap.isNull() )
        pixmap = static_cast<AttachmentIconItem *>( currentItem() )->icon();
    
    QPoint hotspot;
    hotspot.setX( pixmap.width() / 2 );
    hotspot.setY( pixmap.height() / 2 );
    QMap<QString, QString> metadata;
    metadata["labels"] = labels.join(":");
    Q3DragObject* myDragObject = new KURLDrag( urls, metadata, this );
    myDragObject->setPixmap( pixmap, hotspot );
    return myDragObject;
  }
};

KOEditorAttachments::KOEditorAttachments( int spacing, QWidget *parent,
                                          const char *name )
  : QWidget( parent, name )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setSpacing( spacing );

  mAttachments = new AttachmentIconView( this );
  Q3WhatsThis::add( mAttachments,
                   i18n("Displays items (files, mail, etc.) "
                        "that have been associated with this event or to-do.") 
                        );
  mAttachments->setItemsMovable( false );
  mAttachments->setSelectionMode( Q3IconView::Extended );
  topLayout->addWidget( mAttachments );
  connect( mAttachments, SIGNAL( executed( Q3IconViewItem * ) ),
           SLOT( showAttachment( Q3IconViewItem * ) ) );
  connect( mAttachments, SIGNAL( itemRenamed( Q3IconViewItem *, 
                                const QString & ) ),
           SLOT( slotItemRenamed( Q3IconViewItem *, const QString & ) ) );
  connect( mAttachments, SIGNAL( contextMenuRequested( Q3IconViewItem *, 
                                                       const QPoint & ) ),
           SLOT( showAttachmentContextMenu( Q3IconViewItem *, 
                                            const QPoint & ) ) );
  connect( mAttachments, SIGNAL( dropped( QDropEvent *, 
                                          const Q3ValueList<Q3IconDragItem> & ) ),
           SLOT( dropped( QDropEvent *, const Q3ValueList<Q3IconDragItem> & ) ) );
  
  // FIXME for some reason it doesn't work
  connect( mAttachments, SIGNAL( moved() ), SLOT( slotRemove() ) );

  QBoxLayout *buttonLayout = new QHBoxLayout( topLayout );

  QPushButton *button = new QPushButton( i18n("&Add..."), this );
  Q3WhatsThis::add( button,
                   i18n("Shows a dialog used to select an attachment "
                        "to add to this event or to-do.") );
  buttonLayout->addWidget( button );
  connect( button, SIGNAL( clicked() ), SLOT( slotAdd() ) );

  button = new QPushButton( i18n("&Properties..."), this );
  Q3WhatsThis::add( button,
                   i18n("Shows a dialog used to edit the attachment "
                        "currently selected in the list above.") );
  buttonLayout->addWidget( button );
  connect( button, SIGNAL( clicked() ), SLOT( slotEdit() ) );

  button = new QPushButton( i18n("&Remove"), this );
  Q3WhatsThis::add( button,
                   i18n("Removes the attachment selected in the list above "
                        "from this event or to-do.") );
  buttonLayout->addWidget( button );
  connect( button, SIGNAL( clicked() ), SLOT( slotRemove() ) );

  button = new QPushButton( i18n("&Show"), this );
  Q3WhatsThis::add( button,
                   i18n("Opens the attachment selected in the list above "
                        "in the viewer that is associated with it in your "
                        "KDE preferences.") );
  buttonLayout->addWidget( button );
  connect( button, SIGNAL( clicked() ), SLOT( slotShow() ) );
  
  mPopupMenu = new KPopupMenu( this );
  mPopupMenu->insertItem( i18n( "&Open" ), this, SLOT( slotShow() ) );
  mPopupMenu->insertItem( i18n( "&Delete" ), this, SLOT( slotRemove() ) );
  mPopupMenu->insertItem( i18n( "&Properties..." ), this, SLOT( slotEdit() ) );
  
  mPopupNew = new KPopupMenu( this );
  mPopupNew->insertItem( i18n( "&New..." ), this, SLOT( slotAdd() ) );

  setAcceptDrops( TRUE );
}

KOEditorAttachments::~KOEditorAttachments()
{
  // delete any previously copied files that weren't accepted
  for ( KURL::List::ConstIterator it = mDeferredCopy.constBegin();
        it != mDeferredCopy.constEnd(); ++it ) {
    Q_ASSERT( ( *it ).isLocalFile() );
    QFile::remove( ( *it ).path() );
  }
}

bool KOEditorAttachments::hasAttachments()
{
  return mAttachments->count() > 0;
}

void KOEditorAttachments::dragEnterEvent( QDragEnterEvent* event ) {
  event->accept( KURLDrag::canDecode( event ) | Q3TextDrag::canDecode( event ) );
}

QString KOEditorAttachments::generateLocalAttachmentPath( 
                              QString filename, const KMimeType::Ptr mimeType )
{
  QString pathBegin = "korganizer/attachments/";
  if ( mUid.isEmpty() )
    pathBegin += KApplication::randomString( 10 ); // arbitrary
  else
    pathBegin += mUid;
  pathBegin += "/";
  
  if ( filename.isEmpty() )
    filename = KApplication::randomString( 10 ) + 
                     QString( mimeType->patterns().first() ).replace( "*", "" );
  else {
    // we need to determine if there is a correct extension
    bool correctExtension = false;
    for ( QStringList::ConstIterator it = mimeType->patterns().constBegin();
          it != mimeType->patterns().constEnd(); ++it ) {
      QRegExp re( *it );
      re.setWildcard( true );
      if ( ( correctExtension = re.exactMatch( filename ) ) )
        break;
    }
    if ( !correctExtension )
      // we take the first one
      filename += QString( mimeType->patterns().first() ).replace( "*", "" );
  }
  
  QString path = locateLocal( "data", pathBegin + filename );
  
  while ( QFile::exists( path ) )
    // no need to worry much about races here, I guess
    path = locateLocal( "data", 
                       pathBegin + KApplication::randomString( 6 ) + filename );
  
  return path;
}

void KOEditorAttachments::dropEvent( QDropEvent* event ) {
  KURL::List urls;
  QString text;
  bool probablyWeHaveUris = false;
  bool weCanCopy = true;
  KABC::Addressee::List addressees;
  QStringList labels;
  QMap<QString,QString> metadata;
  if ( KVCardDrag::canDecode( event ) ) {
    KVCardDrag::decode( event, addressees );
    for ( KABC::Addressee::List::ConstIterator it = addressees.constBegin();
          it != addressees.constEnd(); ++it ) {
      urls.append( KDEPIMPROTOCOL_CONTACT + ( *it ).uid() );
      // there is some weirdness about realName(), hence fromUtf8
      labels.append( QString::fromUtf8( ( *it ).realName().latin1() ) );
    }
    probablyWeHaveUris = true;
  } else if ( KURLDrag::decode( event, urls, metadata ) ) {
    probablyWeHaveUris = true;
    labels = QStringList::split( ":", metadata["labels"] );
    for ( QStringList::Iterator it = labels.begin(); it != labels.end(); ++it )
      *it = KURL::decode_string( *it );
  } else if ( Q3TextDrag::decode( event, text ) ) {
    QStringList lst = QStringList::split( '\n', text );
    for ( QStringList::ConstIterator it = lst.begin(); it != lst.end(); ++it )
      urls.append( *it );
    probablyWeHaveUris = true;
  }
  
  KPopupMenu menu( this );
  if ( probablyWeHaveUris ) {
    menu.insertItem( i18n( "&Link here" ), 1 );
    // we need to check if we can reasonably expect to copy the objects
    for ( KURL::List::ConstIterator it = urls.constBegin(); 
          it != urls.constEnd(); ++it )
      if ( !( weCanCopy = KProtocolInfo::supportsReading( *it ) ) )
        break; // either we can copy them all, or no copying at all
    if ( weCanCopy )
      menu.insertItem( i18n( "&Copy here" ), 0 );
  } else {
    menu.insertItem( i18n( "&Copy here" ), 0 );
  }
  
  menu.insertSeparator();
  menu.insertItem( i18n( "C&ancel" ), 4 );
  
  int ret = menu.exec( QCursor::pos() );
  if ( 1 == ret ) {
    QStringList::ConstIterator jt = labels.constBegin();
    for ( KURL::List::ConstIterator it = urls.constBegin(); 
          it != urls.constEnd(); ++it )
      addAttachment( (*it).url(), QString::null, 
                     ( jt == labels.constEnd() ? QString::null : *(jt++) ) );
  } else if ( 0 == ret ) {
    if ( probablyWeHaveUris )
      for ( KURL::List::ConstIterator it = urls.constBegin();
            it != urls.constEnd(); ++it ) {
#if 0 // binary attachments are unimplemented yet
        KIO::Job *job = KIO::storedGet( *it );
        connect( job, SIGNAL( result( KIO::Job * ) ), 
                SLOT( downloadComplete( KIO::Job * ) ) );
#endif
        KIO::Job *job = KIO::copy( *it, generateLocalAttachmentPath( 
                                                   ( *it ).fileName(), 
                                                   KMimeType::findByURL( *it ) )
                                 );
        connect( job, SIGNAL( result( KIO::Job * ) ), 
                 SLOT( copyComplete( KIO::Job * ) ) );
      }
    else { // we take anything
      KMimeType::Ptr mimeType = KMimeType::mimeType( event->format() );
      QString path = generateLocalAttachmentPath( QString::null, mimeType );
      QFile file( path );
      file.open( QIODevice::WriteOnly );
      QDataStream( &file ) << event->encodedData( event->format() );
      file.close();
      addAttachment( path, mimeType->name(), mimeType->comment(), true );
      mDeferredCopy.append( path );
    }
#if 0 // binary attachments are unimplemented yet
      addAttachment( event->encodedData( event->format() ), event->format(), 
                     KMimeType::mimeType( event->format() )->name() );
#endif
  }
}

#if 0 // binary attachments are unimplemented yet
void KOEditorAttachments::downloadComplete( KIO::Job *job )
{
  if ( job->error() )
    job->showErrorDialog( this );
  else
    addAttachment( static_cast<KIO::StoredTransferJob *>( job )->data(), 
                   QString::null, 
                   static_cast<KIO::SimpleJob *>( job )->url().fileName() );
}
#endif

void KOEditorAttachments::copyComplete( KIO::Job *job )
{
  if ( job->error() )
    job->showErrorDialog( this );
  else {
    addAttachment( static_cast<KIO::CopyJob *>( job )->destURL().url(), 
                   QString::null, 
                   static_cast<KIO::CopyJob *>( job )->
                                                   srcURLs().first().fileName(),
                   true );
    mDeferredCopy.append( static_cast<KIO::CopyJob *>( job )->destURL() );
  }
}

void KOEditorAttachments::dropped ( QDropEvent * e, 
                                    const Q3ValueList<Q3IconDragItem> & /*lst*/ )
{
  dropEvent( e );
}

void KOEditorAttachments::showAttachment( Q3IconViewItem *item )
{
  AttachmentIconItem *attitem = static_cast<AttachmentIconItem*>(item);
  if ( !attitem || !attitem->attachment() ) return;

  KCal::Attachment *att = attitem->attachment();
  if ( att->isUri() ) {
    emit openURL( att->uri() );
  } else {
    KMessageBox::sorry( this, i18n( "Binary attachment, not supported." ) );
#if 0
    // read-only not to give the idea that it could be written to
    KTempFile file( QString::null, QString( KMimeType::mimeType( att->mimeType()
                                            )->patterns().first() )
                                   .replace( "*", "" ), 0400 );
    *file.dataStream() << att->decodedData();
    file.close();
    KRun::runURL( file.name(), att->mimeType(), true );
#endif
  }
}



void KOEditorAttachments::slotAdd()
{
  AttachmentIconItem *item = new AttachmentIconItem( 0, mAttachments );
  item->setLabel( i18n( "New attachment" ) );
  AttachmentEditDialog *dlg = new AttachmentEditDialog( item, mAttachments );

  dlg->setCaption( i18n( "Add Attachment" ) );

  dlg->exec();
  
  if ( dlg->result() == KDialog::Rejected ) {
    delete dlg; // delete it first, as it hold item
    delete item;
  }
}

void KOEditorAttachments::slotEdit()
{
  for ( Q3IconViewItem *item = mAttachments->firstItem(); item;
        item = item->nextItem() )
    if ( item->isSelected() ) {
      AttachmentIconItem *attitem = static_cast<AttachmentIconItem*>(item);
      if ( !attitem || !attitem->attachment() ) return;
    
      AttachmentEditDialog *dialog = new AttachmentEditDialog( attitem,
          mAttachments, false );
      dialog->setModal( false );
      connect( dialog, SIGNAL( hidden() ), dialog,
               SLOT( slotDelayedDestruct() ) );
      dialog->show();
    }
}

void KOEditorAttachments::slotRemove()
{
  Q3ValueList<Q3IconViewItem *> toDelete;
  for ( Q3IconViewItem *it = mAttachments->firstItem(); it; it = it->nextItem() )
    if ( it->isSelected() ) {
      AttachmentIconItem *item =
          static_cast<AttachmentIconItem *>( it );
      
      if ( !item ) continue;
    
      if ( KMessageBox::warningContinueCancel(this,
           i18n("The item labeled \"%1\" will be permanently deleted.")
               .arg( item->label() ),
           i18n("KOrganizer Confirmation"),
           KStdGuiItem::del()) == KMessageBox::Continue )
      {
        if ( item->isLocal() )
          mDeferredDelete.append( item->uri() );
        toDelete.append( it );
      }
    }

  for ( Q3ValueList<Q3IconViewItem *>::ConstIterator it = toDelete.constBegin();
        it != toDelete.constEnd(); ++it )
    delete *it;
}

void KOEditorAttachments::slotShow()
{
  for ( Q3IconViewItem *item = mAttachments->firstItem(); item;
        item = item->nextItem() )
    if ( item->isSelected() )
      showAttachment( item );
}

void KOEditorAttachments::setDefaults()
{
  mAttachments->clear();
}

void KOEditorAttachments::addAttachment( const QString &uri,
                                         const QString &mimeType,
                                         const QString &label,
                                         bool local )
{
  AttachmentIconItem *item = new AttachmentIconItem( 0, mAttachments );
  item->setUri( uri );
  item->setLabel( label );
  if ( mimeType.isEmpty() ) {
    if ( uri.startsWith( KDEPIMPROTOCOL_CONTACT ) )
      item->setMimeType( "text/x-vcard" );
    else if ( uri.startsWith( KDEPIMPROTOCOL_EMAIL ) )
      item->setMimeType( "message/rfc822" );
    else if ( uri.startsWith( KDEPIMPROTOCOL_INCIDENCE ) )
      item->setMimeType( "text/calendar" );
    else if ( uri.startsWith( KDEPIMPROTOCOL_NEWSARTICLE ) )
      item->setMimeType( "message/news" );
    else
      item->setMimeType( KMimeType::findByURL( uri )->name() );
  } else
    item->setMimeType( mimeType );
  item->setLocal( local );
}

void KOEditorAttachments::addAttachment( const QByteArray &data, 
                                         const QString &mimeType, 
                                         const QString &label )
{
  AttachmentIconItem *item = new AttachmentIconItem( 0, mAttachments );
  item->setData( data );
  item->setLabel( label );
  if ( mimeType.isEmpty() )
    item->setMimeType( KMimeType::findByContent( data )->name() );
  else
    item->setMimeType( mimeType );
}

void KOEditorAttachments::addAttachment( KCal::Attachment *attachment )
{
  new AttachmentIconItem( attachment, mAttachments );
}

void KOEditorAttachments::readIncidence( KCal::Incidence *i )
{
  mAttachments->clear();

  KCal::Attachment::List attachments = i->attachments();
  KCal::Attachment::List::ConstIterator it;
  for( it = attachments.begin(); it != attachments.end(); ++it ) {
    addAttachment( (*it) );
  }
  mUid = i->uid();
}

void KOEditorAttachments::writeIncidence( KCal::Incidence *i )
{
  i->clearAttachments();

  Q3IconViewItem *item;
  AttachmentIconItem *attitem;
  for( item = mAttachments->firstItem(); item; item = item->nextItem() ) {
    attitem = static_cast<AttachmentIconItem*>(item);
    if ( attitem )
      i->addAttachment( new KCal::Attachment( *(attitem->attachment() ) ) );
  }
}

void KOEditorAttachments::slotItemRenamed ( Q3IconViewItem * item, 
                                            const QString & text )
{
  static_cast<AttachmentIconItem *>( item )->setLabel( text );
}

void KOEditorAttachments::showAttachmentContextMenu( Q3IconViewItem *item, 
                                                     const QPoint &pos )
{
  if ( item )
    mPopupMenu->popup( pos );
  else
    mPopupNew->popup( pos );
}

void KOEditorAttachments::applyChanges()
{
  for ( KURL::List::ConstIterator it = mDeferredDelete.constBegin(); 
        it != mDeferredDelete.constEnd(); ++it ) {
    Q_ASSERT( ( *it ).isLocalFile() );
    QFile::remove( ( *it ).path() );
  }
  mDeferredDelete.clear();
  
  mDeferredCopy.clear(); // files are already copied
}

#include "koeditorattachments.moc"
