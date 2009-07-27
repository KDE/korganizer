/*
    This file is part of KOrganizer.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2005 Reinhold Kainhofer <reinhold@kainhofer.com>

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
#include <libkdepim/kpimurlrequesterdlg.h>
#include <libkdepim/kfileio.h>
#include <libkdepim/kdepimprotocols.h>
#include <libkdepim/maillistdrag.h>
#include <libkdepim/kvcarddrag.h>
#include <libkdepim/kdepimprotocols.h>

#include <klocale.h>
#include <kdebug.h>
#include <kmdcodec.h>
#include <kmessagebox.h>
#include <krun.h>
#include <kurldrag.h>
#include <ktempfile.h>
#include <ktempdir.h>
#include <kio/netaccess.h>
#include <kmimetype.h>
#include <kiconloader.h>
#include <kfiledialog.h>
#include <kstdaction.h>
#include <kactioncollection.h>
#include <kpopupmenu.h>
#include <kprotocolinfo.h>
#include <klineedit.h>
#include <kseparator.h>
#include <kurlrequester.h>

#include <qcheckbox.h>
#include <qfile.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qdragobject.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qapplication.h>
#include <qclipboard.h>

#include <cassert>
#include <cstdlib>

class AttachmentListItem : public KIconViewItem
{
  public:
    AttachmentListItem( KCal::Attachment*att, QIconView *parent ) :
        KIconViewItem( parent )
    {
      if ( att ) {
        mAttachment = new KCal::Attachment( *att );
      } else {
        mAttachment = new KCal::Attachment( QString::null );
      }
      readAttachment();
      setDragEnabled( true );
    }
    ~AttachmentListItem() { delete mAttachment; }
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
    void setData( const char *base64 )
    {
      mAttachment->setData( base64 );
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
    void setLabel( const QString &label )
    {
      mAttachment->setLabel( label );
      readAttachment();
    }
    bool isBinary() const
    {
      return mAttachment->isBinary();
    }
    QPixmap icon() const
    {
      return icon( KMimeType::mimeType( mAttachment->mimeType() ),
                   mAttachment->uri() );
    }
    static QPixmap icon( KMimeType::Ptr mimeType, const QString &uri )
    {
      QString iconStr = mimeType->icon( uri, false );
      return KGlobal::iconLoader()->loadIcon( iconStr, KIcon::Small );
    }
    void readAttachment()
    {
      if ( mAttachment->label().isEmpty() ) {
        if ( mAttachment->isUri() ) {
          setText( mAttachment->uri() );
        } else {
          setText( i18n( "[Binary data]" ) );
        }
      } else {
        setText( mAttachment->label() );
      }
      if ( mAttachment->mimeType().isEmpty() ||
           !( KMimeType::mimeType( mAttachment->mimeType() ) ) ) {
        KMimeType::Ptr mimeType;
        if ( mAttachment->isUri() ) {
          mimeType = KMimeType::findByURL( mAttachment->uri() );
        } else {
          mimeType = KMimeType::findByContent( QCString( mAttachment->data() ) );
        }
        mAttachment->setMimeType( mimeType->name() );
      }

      setPixmap( icon() );
    }

  private:
    KCal::Attachment *mAttachment;
};

AttachmentEditDialog::AttachmentEditDialog( AttachmentListItem *item,
                                            QWidget *parent )
  : KDialogBase ( Plain, i18n( "Add Attachment" ), Ok|Cancel, Ok, parent, 0, false, false ),
    mItem( item ), mURLRequester( 0 )
{
  QFrame *topFrame = plainPage();
  QVBoxLayout *vbl = new QVBoxLayout( topFrame, 0, spacingHint() );

  QGridLayout *grid = new QGridLayout();
  grid->setColStretch( 0, 0 );
  grid->setColStretch( 1, 0 );
  grid->setColStretch( 2, 1 );
  vbl->addLayout( grid );

  mIcon = new QLabel( topFrame );
  mIcon->setPixmap( item->icon() );
  grid->addWidget( mIcon, 0, 0 );

  mLabelEdit = new KLineEdit( topFrame );
  mLabelEdit->setClickMessage( i18n( "Attachment name" ) );
  if ( !item->uri().isEmpty() ) {
    KURL url( item->uri() );
    if ( url.isLocalFile() ) {
      mLabelEdit->setText( url.fileName() );
    } else {
      mLabelEdit->setText( url.url() );
    }
  }
  QToolTip::add( mLabelEdit, i18n( "Give the attachment a name" ) );
  QWhatsThis::add( mLabelEdit,
                   i18n( "Type any string you desire here for the name of the attachment" ) );
  grid->addMultiCellWidget( mLabelEdit, 0, 0, 1, 2 );

  KSeparator *sep = new KSeparator( Qt::Horizontal, topFrame );
  grid->addMultiCellWidget( sep, 1, 1, 0, 2 );

  QLabel *label = new QLabel( i18n( "Type:" ), topFrame );
  grid->addWidget( label, 2, 0 );
  QString typecomment = item->mimeType().isEmpty() ?
                        i18n( "Unknown" ) :
                        KMimeType::mimeType( item->mimeType() )->comment();
  mTypeLabel = new QLabel( typecomment, topFrame );
  grid->addWidget( mTypeLabel, 2, 1 );
  mMimeType = KMimeType::mimeType( item->mimeType() );

  mInline = new QCheckBox( i18n( "Store attachment inline" ), topFrame );
  grid->addMultiCellWidget( mInline, 3, 3, 0, 2 );
  mInline->setChecked( item->isBinary() );
  QToolTip::add( mInline, i18n( "Store the attachment file inside the calendar" ) );
  QWhatsThis::add(
    mInline,
    i18n( "Checking this option will cause the attachment to be stored inside "
          "your calendar, which can take a lot of space depending on the size "
          "of the attachment. If this option is not checked, then only a link "
          "pointing to the attachment will be stored.  Do not use a link for "
          "attachments that change often or may be moved (or removed) from "
          "their current location." ) );

  if ( item->attachment()->isUri() ) {
    label = new QLabel( i18n( "Location:" ), topFrame );
    grid->addWidget( label, 4, 0 );
    mURLRequester = new KURLRequester( item->uri(), topFrame );
    QToolTip::add( mURLRequester, i18n( "Provide a location for the attachment file" ) );
    QWhatsThis::add(
      mURLRequester,
      i18n( "Enter the path to the attachment file or use the "
            "file browser by pressing the adjacent button" ) );
    grid->addMultiCellWidget( mURLRequester, 4, 4, 1, 2 );
    connect( mURLRequester, SIGNAL(urlSelected(const QString &)),
             SLOT(urlSelected(const QString &)) );
    connect( mURLRequester, SIGNAL( textChanged( const QString& ) ),
             SLOT( urlChanged( const QString& ) ) );
    urlChanged( item->uri() );
  } else {
    uint size = QCString( item->attachment()->data() ).size();
    grid->addWidget( new QLabel( i18n( "Size:" ), topFrame ), 4, 0 );
    grid->addWidget( new QLabel( QString::fromLatin1( "%1 (%2)" ).
                                 arg( KIO::convertSize( size ) ).
                                 arg( KGlobal::locale()->formatNumber(
                                        size, 0 ) ), topFrame ), 4, 2 );
  }
  vbl->addStretch( 10 );
}

void AttachmentEditDialog::slotApply()
{
  KURL url( mURLRequester->url() );
  if ( mLabelEdit->text().isEmpty() ) {
    if ( url.isLocalFile() ) {
      mItem->setLabel( url.fileName() );
    } else {
      mItem->setLabel( url.url() );
    }
  } else {
    mItem->setLabel( mLabelEdit->text() );
  }
  if ( mItem->label().isEmpty() ) {
    mItem->setLabel( i18n( "New attachment" ) );
  }
  mItem->setMimeType( mMimeType->name() );
  if ( mURLRequester ) {
    if ( mInline->isChecked() ) {
      QString tmpFile;
      if ( KIO::NetAccess::download( mURLRequester->url(), tmpFile, this ) ) {
        QFile f( tmpFile );
        if ( !f.open( IO_ReadOnly ) ) {
          return;
        }
        QByteArray data = f.readAll();
        f.close();
        mItem->setData( data );
      }
      KIO::NetAccess::removeTempFile( tmpFile );
    } else {
      mItem->setUri( url.url() );
    }
  }
}

void AttachmentEditDialog::accept()
{
  slotApply();
  KDialog::accept();
}

void AttachmentEditDialog::urlChanged( const QString &url )
{
  enableButton( Ok, !url.isEmpty() );
}

void AttachmentEditDialog::urlSelected( const QString &url )
{
  KURL kurl( url );
  mMimeType = KMimeType::findByURL( kurl );
  mTypeLabel->setText( mMimeType->comment() );
  mIcon->setPixmap( AttachmentListItem::icon( mMimeType, kurl.path() ) );
}

AttachmentIconView::AttachmentIconView( KOEditorAttachments* parent )
  : KIconView( parent ),
    mParent( parent )
{
  setSelectionMode( QIconView::Extended );
  setMode( KIconView::Select );
  setItemTextPos( QIconView::Right );
  setArrangement( QIconView::LeftToRight );
  setMaxItemWidth( QMAX(maxItemWidth(), 250) );
  setMinimumHeight( QMAX(fontMetrics().height(), 16) + 12 );

  connect( this, SIGNAL( dropped ( QDropEvent *, const QValueList<QIconDragItem> & ) ),
           this, SLOT( handleDrop( QDropEvent *, const QValueList<QIconDragItem> & ) ) );
}

KURL AttachmentIconView::tempFileForAttachment( KCal::Attachment *attachment )
{
  if ( mTempFiles.contains( attachment ) ) {
    return mTempFiles[attachment];
  }
  QStringList patterns = KMimeType::mimeType( attachment->mimeType() )->patterns();

  KTempFile *file;
  if ( !patterns.empty() ) {
    file = new KTempFile( QString::null,
                          QString( patterns.first() ).remove( '*' ),0600 );
  } else {
    file = new KTempFile( QString::null, QString::null, 0600 );
  }
  file->setAutoDelete( true );
  file->file()->open( IO_WriteOnly );
  QTextStream stream( file->file() );
  stream << attachment->data();
  KURL url( file->name() );
  mTempFiles.insert( attachment, url );
  file->close();
  return mTempFiles[attachment];
}

QDragObject *AttachmentIconView::mimeData()
{
  // create a list of the URL:s that we want to drag
  KURL::List urls;
  QStringList labels;
  for ( QIconViewItem *it = firstItem(); it; it = it->nextItem() ) {
    if ( it->isSelected() ) {
      AttachmentListItem *item = static_cast<AttachmentListItem *>( it );
      if ( item->isBinary() ) {
        urls.append( tempFileForAttachment( item->attachment() ) );
      } else {
        urls.append( item->uri() );
      }
      labels.append( KURL::encode_string( item->label() ) );
    }
  }
  if ( selectionMode() == QIconView::NoSelection ) {
    AttachmentListItem *item = static_cast<AttachmentListItem *>( currentItem() );
    if ( item ) {
      urls.append( item->uri() );
      labels.append( KURL::encode_string( item->label() ) );
    }
  }

  QMap<QString, QString> metadata;
  metadata["labels"] = labels.join( ":" );

  KURLDrag *drag = new KURLDrag( urls, metadata );
  return drag;
}

AttachmentIconView::~AttachmentIconView()
{
  for ( std::set<KTempDir*>::iterator it = mTempDirs.begin() ; it != mTempDirs.end() ; ++it ) {
    delete *it;
  }
}

QDragObject * AttachmentIconView::dragObject()
{
  KURL::List urls;
  for ( QIconViewItem *it = firstItem( ); it; it = it->nextItem( ) ) {
    if ( !it->isSelected() ) continue;
    AttachmentListItem * item = dynamic_cast<AttachmentListItem*>( it );
    if ( !item ) return 0;
    KCal::Attachment * att = item->attachment();
    assert( att );
    KURL url;
    if ( att->isUri() ) {
      url.setPath( att->uri() );
    } else {
      KTempDir * tempDir = new KTempDir(); // will be deleted on editor close
      tempDir->setAutoDelete( true );
      mTempDirs.insert( tempDir );
      QByteArray encoded;
      encoded.duplicate( att->data(), strlen(att->data()) );
      QByteArray decoded;
      KCodecs::base64Decode( encoded, decoded );
      const QString fileName = tempDir->name( ) + "/" + att->label();
      KPIM::kByteArrayToFile( decoded, fileName, false, false, false );
      url.setPath( fileName );
    }
    urls << url;
  }
  KURLDrag *drag  = new KURLDrag( urls, this );
  return drag;
}

void AttachmentIconView::handleDrop( QDropEvent *event, const QValueList<QIconDragItem> & list )
{
  Q_UNUSED( list );
  mParent->handlePasteOrDrop( event );
}


void AttachmentIconView::dragMoveEvent( QDragMoveEvent *event )
{
  mParent->dragMoveEvent( event );
}

void AttachmentIconView::contentsDragMoveEvent( QDragMoveEvent *event )
{
  mParent->dragMoveEvent( event );
}

void AttachmentIconView::contentsDragEnterEvent( QDragEnterEvent *event )
{
  mParent->dragMoveEvent( event );
}

void AttachmentIconView::dragEnterEvent( QDragEnterEvent *event )
{
  mParent->dragEnterEvent( event );
}

KOEditorAttachments::KOEditorAttachments( int spacing, QWidget *parent,
                                          const char *name )
  : QWidget( parent, name )
{
  QBoxLayout *topLayout = new QHBoxLayout( this );
  topLayout->setSpacing( spacing );

  QLabel *label = new QLabel( i18n("Attachments:"), this );
  topLayout->addWidget( label );

  mAttachments = new AttachmentIconView( this );
  QWhatsThis::add( mAttachments,
                   i18n("Displays a list of current items (files, mail, etc.) "
                        "that have been associated with this event or to-do. ") );
  topLayout->addWidget( mAttachments );
  connect( mAttachments, SIGNAL( doubleClicked( QIconViewItem * ) ),
           SLOT( showAttachment( QIconViewItem * ) ) );
  connect( mAttachments, SIGNAL(selectionChanged()),
           SLOT(selectionChanged()) );
  connect( mAttachments, SIGNAL(contextMenuRequested(QIconViewItem*,const QPoint&)),
           SLOT(contextMenu(QIconViewItem*,const QPoint&)) );

    QPushButton *addButton = new QPushButton( this );
  addButton->setIconSet( SmallIconSet( "add" ) );
  QToolTip::add( addButton, i18n( "Add an attachment" ) );
  QWhatsThis::add( addButton,
                   i18n( "Shows a dialog used to select an attachment "
                         "to add to this event or to-do as link or as "
                         "inline data." ) );
  topLayout->addWidget( addButton );
  connect( addButton, SIGNAL(clicked()), SLOT(slotAdd()) );

  mRemoveBtn = new QPushButton( this );
  mRemoveBtn->setIconSet( SmallIconSet( "remove" ) );
  QToolTip::add( mRemoveBtn, i18n("&Remove") );
  QWhatsThis::add( mRemoveBtn,
                   i18n("Removes the attachment selected in the list above "
                        "from this event or to-do.") );
  topLayout->addWidget( mRemoveBtn );
  connect( mRemoveBtn, SIGNAL(clicked()), SLOT(slotRemove()) );

  mContextMenu = new KPopupMenu( this );

  KActionCollection* ac = new KActionCollection( this, this );

  mOpenAction = new KAction( i18n("Open"), 0, this, SLOT(slotShow()), ac );
  mOpenAction->plug( mContextMenu );
  mContextMenu->insertSeparator();

  mCopyAction = KStdAction::copy(this, SLOT(slotCopy()), ac );
  mCopyAction->plug( mContextMenu );
  mCutAction = KStdAction::cut(this, SLOT(slotCut()), ac );
  mCutAction->plug( mContextMenu );
  KAction *action = KStdAction::paste(this, SLOT(slotPaste()), ac );
  action->plug( mContextMenu );
  mContextMenu->insertSeparator();

  mDeleteAction = new KAction( i18n( "&Remove" ), 0, this, SLOT(slotRemove()),  ac );
  mDeleteAction->plug( mContextMenu );
  mContextMenu->insertSeparator();

  mEditAction = new KAction( i18n( "&Properties..." ), 0, this, SLOT(slotEdit()), ac );
  mEditAction->plug( mContextMenu );

  selectionChanged();
  setAcceptDrops( true );
}

KOEditorAttachments::~KOEditorAttachments()
{
}

bool KOEditorAttachments::hasAttachments()
{
  return mAttachments->count() != 0;
}

void KOEditorAttachments::dragMoveEvent( QDragMoveEvent *event )
{
  event->accept( KURLDrag::canDecode( event ) ||
                 QTextDrag::canDecode( event ) ||
                 KPIM::MailListDrag::canDecode( event ) ||
                 KVCardDrag::canDecode( event ) );
}

void KOEditorAttachments::dragEnterEvent( QDragEnterEvent* event )
{
  dragMoveEvent( event );
}

void KOEditorAttachments::handlePasteOrDrop( QMimeSource* source )
{
  KURL::List urls;
  bool probablyWeHaveUris = false;
  bool weCanCopy = true;
  QStringList labels;

  if ( KVCardDrag::canDecode( source ) ) {
    KABC::Addressee::List addressees;
    KVCardDrag::decode( source, addressees );
    for ( KABC::Addressee::List::ConstIterator it = addressees.constBegin();
          it != addressees.constEnd(); ++it ) {
      urls.append( KDEPIMPROTOCOL_CONTACT + ( *it ).uid() );
      // there is some weirdness about realName(), hence fromUtf8
      labels.append( QString::fromUtf8( ( *it ).realName().latin1() ) );
    }
    probablyWeHaveUris = true;
  } else if ( KURLDrag::canDecode( source ) ) {
    QMap<QString,QString> metadata;
    if ( KURLDrag::decode( source, urls, metadata ) ) {
      probablyWeHaveUris = true;
      labels = QStringList::split( ':', metadata["labels"], FALSE );
      for ( QStringList::Iterator it = labels.begin(); it != labels.end(); ++it ) {
        *it = KURL::decode_string( (*it).latin1() );
      }

    }
  } else if ( QTextDrag::canDecode( source ) ) {
    QString text;
    QTextDrag::decode( source, text );
    QStringList lst = QStringList::split( '\n', text, FALSE );
    for ( QStringList::ConstIterator it = lst.constBegin(); it != lst.constEnd(); ++it ) {
      urls.append( *it );
      labels.append( QString::null );
    }
    probablyWeHaveUris = true;
  }

  KPopupMenu menu;
  int items=0;
  if ( probablyWeHaveUris ) {
    menu.insertItem( i18n( "&Link here" ), DRAG_LINK, items++ );
    // we need to check if we can reasonably expect to copy the objects
    for ( KURL::List::ConstIterator it = urls.constBegin(); it != urls.constEnd(); ++it ) {
      if ( !( weCanCopy = KProtocolInfo::supportsReading( *it ) ) ) {
        break; // either we can copy them all, or no copying at all
      }
    }
    if ( weCanCopy ) {
      menu.insertItem( SmallIcon( "editcopy" ), i18n( "&Copy Here" ), DRAG_COPY, items++ );
    }
  } else {
      menu.insertItem( SmallIcon( "editcopy" ), i18n( "&Copy Here" ), DRAG_COPY, items++ );
  }

  menu.insertSeparator();
  items++;
  menu.insertItem( SmallIcon( "cancel" ), i18n( "C&ancel" ), DRAG_CANCEL, items );
  int action = menu.exec( QCursor::pos(), 0 );

  if ( action == DRAG_LINK ) {
    QStringList::ConstIterator jt = labels.constBegin();
    for ( KURL::List::ConstIterator it = urls.constBegin();
          it != urls.constEnd(); ++it ) {
      QString label = (*jt++);
      if ( mAttachments->findItem( label ) ) {
        label += '~' + randomString( 3 );
      }
      addAttachment( (*it).url(), QString::null, label );
    }
  } else if ( action != DRAG_CANCEL ) {
    if ( probablyWeHaveUris ) {
      for ( KURL::List::ConstIterator it = urls.constBegin();
            it != urls.constEnd(); ++it ) {
        QString label = (*it).fileName();
        if ( label.isEmpty() ) {
          label = (*it).prettyURL();
        }
        if ( mAttachments->findItem( label ) ) {
          label += '~' + randomString( 3 );
        }
        addAttachment( (*it).url(), QString::null, label );
      }
    } else { // we take anything
      addAttachment( source->encodedData( source->format() ),
                     source->format(),
                     KMimeType::mimeType( source->format() )->name() );
    }
  }
}

void KOEditorAttachments::dropEvent( QDropEvent* event )
{
    handlePasteOrDrop( event );
}

void KOEditorAttachments::showAttachment( QIconViewItem *item )
{
  AttachmentListItem *attitem = static_cast<AttachmentListItem*>(item);
  if ( !attitem || !attitem->attachment() ) return;

  KCal::Attachment *att = attitem->attachment();
  if ( att->isUri() ) {
    emit openURL( att->uri() );
  } else {
    KRun::runURL( mAttachments->tempFileForAttachment( att ), att->mimeType(), 0, true );
  }
}

void KOEditorAttachments::slotAdd()
{
  AttachmentListItem *item = new AttachmentListItem( 0, mAttachments );

  AttachmentEditDialog *dlg = new AttachmentEditDialog( item, mAttachments )
;
  if ( dlg->exec() == KDialog::Rejected ) {
    delete item;
  }
  delete dlg;
}

void KOEditorAttachments::slotAddData()
{
  KURL uri = KFileDialog::getOpenFileName( QString(), QString(), this, i18n("Add Attachment") );
  if ( !uri.isEmpty() ) {
    QString label = uri.fileName();
    if ( label.isEmpty() ) {
      label = uri.prettyURL();
    }
    addAttachment( uri.url(), QString::null, label, true );
  }
}

void KOEditorAttachments::slotEdit()
{
  for ( QIconViewItem *item = mAttachments->firstItem(); item; item = item->nextItem() ) {
    if ( item->isSelected() ) {
      AttachmentListItem *attitem = static_cast<AttachmentListItem*>( item );
      if ( !attitem || !attitem->attachment() ) {
        return;
      }

      AttachmentEditDialog *dialog = new AttachmentEditDialog( attitem, mAttachments );
      dialog->mInline->setEnabled( false );
      dialog->setModal( false );
      connect( dialog, SIGNAL(hidden()), dialog, SLOT(delayedDestruct()) );
      dialog->show();
    }
  }
}

void KOEditorAttachments::slotRemove()
{
    QValueList<QIconViewItem*> selected;
    for ( QIconViewItem *it = mAttachments->firstItem( ); it; it = it->nextItem( ) ) {
        if ( !it->isSelected() ) continue;
        selected << it;
    }
    if ( selected.isEmpty() || KMessageBox::warningContinueCancel(this,
                    selected.count() == 1?i18n("This item will be permanently deleted."):
                    i18n("The selected items will be permanently deleted."),
                    i18n("KOrganizer Confirmation"),KStdGuiItem::del()) != KMessageBox::Continue )
        return;

    for ( QValueList<QIconViewItem*>::iterator it( selected.begin() ), end( selected.end() ); it != end ; ++it ) {
        delete *it;
    }
}

void KOEditorAttachments::slotShow()
{
  for ( QIconViewItem *it = mAttachments->firstItem(); it; it = it->nextItem() ) {
    if ( !it->isSelected() )
      continue;
    showAttachment( it );
  }
}

void KOEditorAttachments::setDefaults()
{
  mAttachments->clear();
}

QString KOEditorAttachments::randomString(int length) const
{
   if (length <=0 ) return QString();

   QString str; str.setLength( length );
   int i = 0;
   while (length--)
   {
      int r=random() % 62;
      r+=48;
      if (r>57) r+=7;
      if (r>90) r+=6;
      str[i++] =  char(r);
      // so what if I work backwards?
   }
   return str;
}

void KOEditorAttachments::addAttachment( const QString &uri,
                                         const QString &mimeType,
                                         const QString &label,
                                         bool binary )
{
  if ( !binary ) {
    AttachmentListItem *item = new AttachmentListItem( 0, mAttachments );
    item->setUri( uri );
    item->setLabel( label );
    if ( mimeType.isEmpty() ) {
      if ( uri.startsWith( KDEPIMPROTOCOL_CONTACT ) ) {
        item->setMimeType( "text/directory" );
      } else if ( uri.startsWith( KDEPIMPROTOCOL_EMAIL ) ) {
        item->setMimeType( "message/rfc822" );
      } else if ( uri.startsWith( KDEPIMPROTOCOL_INCIDENCE ) ) {
        item->setMimeType( "text/calendar" );
      } else if ( uri.startsWith( KDEPIMPROTOCOL_NEWSARTICLE ) ) {
        item->setMimeType( "message/news" );
      } else {
        item->setMimeType( KMimeType::findByURL( uri )->name() );
      }
    } else {
      QString tmpFile;
      if ( KIO::NetAccess::download( uri, tmpFile, this ) ) {
        QFile f( tmpFile );
        if ( !f.open( IO_ReadOnly ) ) {
          return;
        }
        const QByteArray data = f.readAll();
        f.close();
        addAttachment( data, mimeType, label );
      }
      KIO::NetAccess::removeTempFile( tmpFile );
    }
  } else {
  }
}

void KOEditorAttachments::addAttachment( const QByteArray &data,
                                         const QString &mimeType,
                                         const QString &label )
{
  AttachmentListItem *item = new AttachmentListItem( 0, mAttachments );
  item->setData( data );
  item->setLabel( label );
  if ( mimeType.isEmpty() ) {
    item->setMimeType( KMimeType::findByContent( data )->name() );
  } else {
    item->setMimeType( mimeType );
  }
}

void KOEditorAttachments::addAttachment( KCal::Attachment *attachment )
{
  new AttachmentListItem( attachment, mAttachments );
}

void KOEditorAttachments::readIncidence( KCal::Incidence *i )
{
  mAttachments->clear();

  KCal::Attachment::List attachments = i->attachments();
  KCal::Attachment::List::ConstIterator it;
  for( it = attachments.begin(); it != attachments.end(); ++it ) {
    addAttachment( (*it) );
  }
  if ( mAttachments->count() > 0 ) {
    QTimer::singleShot( 0, mAttachments, SLOT(arrangeItemsInGrid()) );
  }
}

void KOEditorAttachments::writeIncidence( KCal::Incidence *i )
{
  i->clearAttachments();

  QIconViewItem *item;
  AttachmentListItem *attitem;
  for( item = mAttachments->firstItem(); item; item = item->nextItem() ) {
    attitem = static_cast<AttachmentListItem*>(item);
    if ( attitem )
      i->addAttachment( new KCal::Attachment( *(attitem->attachment() ) ) );
  }
}


void KOEditorAttachments::slotCopy()
{
    QApplication::clipboard()->setData( mAttachments->mimeData(), QClipboard::Clipboard );
}

void KOEditorAttachments::slotCut()
{
    slotCopy();
    slotRemove();
}

void KOEditorAttachments::slotPaste()
{
    handlePasteOrDrop( QApplication::clipboard()->data() );
}

void KOEditorAttachments::selectionChanged()
{
  bool selected = false;
  for ( QIconViewItem *item = mAttachments->firstItem(); item; item = item->nextItem() ) {
    if ( item->isSelected() ) {
      selected = true;
      break;
    }
  }
  mRemoveBtn->setEnabled( selected );
}

void KOEditorAttachments::contextMenu(QIconViewItem * item, const QPoint & pos)
{
  const bool enable = item != 0;
  mOpenAction->setEnabled( enable );
  mCopyAction->setEnabled( enable );
  mCutAction->setEnabled( enable );
  mDeleteAction->setEnabled( enable );
  mEditAction->setEnabled( enable );
  mContextMenu->exec( pos );
}

#include "koeditorattachments.moc"
