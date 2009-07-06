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
    void setMimeType( const QString &mime )
    {
      mAttachment->setMimeType( mime );
      readAttachment();
    }
    void setLabel( const QString &label )
    {
      mAttachment->setLabel( label );
      readAttachment();
    }

    void readAttachment()
    {
      if ( mAttachment->isUri() )
        setText( mAttachment->uri() );
      else {
        if ( mAttachment->label().isEmpty() )
          setText( i18n("[Binary data]") );
        else
          setText( mAttachment->label() );
      }
      KMimeType::Ptr mt = KMimeType::mimeType( mAttachment->mimeType() );
      if ( mt ) {
          const QString iconName( mt->icon( QString(), false ) );
          QPixmap pix = KGlobal::iconLoader( )->loadIcon( iconName, KIcon::Small );
          if ( pix.isNull() )
            pix = KGlobal::iconLoader( )->loadIcon( "unknown", KIcon::Small );
            if ( !pix.isNull() )
              setPixmap( pix );
      }
    }

  private:
    KCal::Attachment *mAttachment;
};

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

  mAddMenu = new KPopupMenu( this );
  mContextMenu = new KPopupMenu( this );

  KActionCollection* ac = new KActionCollection( this, this );

  mOpenAction = new KAction( i18n("View"), 0, this, SLOT(slotShow()), ac );
  mOpenAction->plug( mContextMenu );
  mContextMenu->insertSeparator();

  mCopyAction = KStdAction::copy(this, SLOT(slotCopy( ) ), ac );
  mCopyAction->plug( mContextMenu );
  mCutAction = KStdAction::cut(this, SLOT(slotCut( ) ), ac );
  mCutAction->plug( mContextMenu );
  KAction *action = KStdAction::paste(this, SLOT(slotPaste( ) ), ac );
  action->plug( mContextMenu );

  action = new KAction( i18n("&Attach File..."), 0, this, SLOT(slotAddData()), ac );
  action->setWhatsThis( i18n("Shows a dialog used to select an attachment "
                        "to add to this event or to-do as link as inline data.") );
  action->plug( mAddMenu );
  action = new KAction( i18n("Attach &Link..."), 0, this, SLOT(slotAdd()), ac );
  action->setWhatsThis( i18n("Shows a dialog used to select an attachment "
                        "to add to this event or to-do as link.") );
  action->plug( mAddMenu );

  QPushButton *addButton = new QPushButton( this );
  addButton->setIconSet( SmallIconSet( "add" ) );
  addButton->setPopup( mAddMenu );
  topLayout->addWidget( addButton );

  mRemoveBtn = new QPushButton( this );
  mRemoveBtn->setIconSet( SmallIconSet( "remove" ) );
  QToolTip::add( mRemoveBtn, i18n("&Remove") );
  QWhatsThis::add( mRemoveBtn,
                   i18n("Removes the attachment selected in the list above "
                        "from this event or to-do.") );
  topLayout->addWidget( mRemoveBtn );
  connect( mRemoveBtn, SIGNAL( clicked() ), SLOT( slotRemove() ) );

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
  event->accept( KURLDrag::canDecode( event ) || QTextDrag::canDecode( event ) );
}

void KOEditorAttachments::dragEnterEvent( QDragEnterEvent* event )
{
  dragMoveEvent( event );
}

void KOEditorAttachments::handlePasteOrDrop( QMimeSource* source )
{
  KURL::List urls;
  QString text;
  if ( KURLDrag::decode( source, urls ) ) {
    const bool asUri = KMessageBox::questionYesNo( this,
            i18n("Do you want to link to the attachments, or include them in the event?"),
            i18n("Attach as link?"), i18n("As Link"), i18n("As File") ) == KMessageBox::Yes;
    for ( KURL::List::ConstIterator it = urls.begin(); it != urls.end(); ++it ) {
      addAttachment( (*it).url(), QString::null, asUri );
    }
  } else if ( QTextDrag::decode( source, text ) ) {
    QStringList lst = QStringList::split( '\n', text );
    for ( QStringList::ConstIterator it = lst.begin(); it != lst.end(); ++it ) {
      addAttachment( (*it)  );
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
    KTempFile f;
    if ( !f.file() )
      return;
    QByteArray encoded;
    encoded.duplicate( att->data(), strlen(att->data()) );
    QByteArray decoded;
    KCodecs::base64Decode( encoded, decoded );
    f.file()->writeBlock( decoded );
    f.file()->close();
    KRun::runURL( f.name(), att->mimeType(), true, false );
  }
}

void KOEditorAttachments::slotAdd()
{
  KURL uri = KPimURLRequesterDlg::getURL( QString::null, i18n(
         "URL (e.g. a web page) or file to be attached (only "
         "the link will be attached, not the file itself):"), this,
                                       i18n("Add Attachment") );
  if ( !uri.isEmpty() ) {
    addAttachment( uri );
  }
}

void KOEditorAttachments::slotAddData()
{
  KURL uri = KFileDialog::getOpenFileName( QString(), QString(), this, i18n("Add Attachment") );
  if ( !uri.isEmpty() ) {
    addAttachment( uri, QString::null, false );
  }
}

void KOEditorAttachments::slotEdit()
{
  QIconViewItem *item = mAttachments->currentItem();
  AttachmentListItem *attitem = static_cast<AttachmentListItem*>(item);
  if ( !attitem || !attitem->attachment() ) return;

  KCal::Attachment *att = attitem->attachment();
  if ( att->isUri() ) {
    KURL uri = KPimURLRequesterDlg::getURL( att->uri(), i18n(
         "URL (e.g. a web page) or file to be attached (only "
         "the link will be attached, not the file itself):"), this,
                                         i18n("Edit Attachment") );

    if ( !uri.isEmpty() )
      attitem->setUri( uri.url() );
  } else {
    KURL uri = KPimURLRequesterDlg::getURL( QString::null, i18n(
         "File to be attached:"), this, i18n("Add Attachment") );
    if ( !uri.isEmpty() ) {
          QString tmpFile;
      if ( KIO::NetAccess::download( uri, tmpFile, this ) ) {
        QFile f( tmpFile );
        if ( !f.open( IO_ReadOnly ) )
          return;
        QByteArray data = f.readAll();
        f.close();
        attitem->setData( KCodecs::base64Encode( data ) );
        attitem->setMimeType( KIO::NetAccess::mimetype( uri, this ) );
        QString label = uri.fileName();
        if ( label.isEmpty() )
          label = uri.prettyURL();
        attitem->setLabel( label );
        KIO::NetAccess::removeTempFile( tmpFile );
      }
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

void KOEditorAttachments::addAttachment( const KURL &uri,
                                         const QString &mimeType, bool asUri )
{
  AttachmentListItem *item = new AttachmentListItem( 0, mAttachments );
  if ( asUri ) {
    item->setUri( uri.url() );
    if ( !mimeType.isEmpty() ) item->setMimeType( mimeType );
  } else {
    QString tmpFile;
    if ( KIO::NetAccess::download( uri, tmpFile, this ) ) {
      QFile f( tmpFile );
      if ( !f.open( IO_ReadOnly ) )
        return;
      QByteArray data = f.readAll();
      f.close();
      item->setData( KCodecs::base64Encode( data ) );
      if ( !mimeType.isEmpty() )
        item->setMimeType( mimeType );
      else
        item->setMimeType( KIO::NetAccess::mimetype( uri, this ) );
      QString label = uri.fileName();
      if ( label.isEmpty() )
        label = uri.prettyURL();
      item->setLabel( label );
      KIO::NetAccess::removeTempFile( tmpFile );
    }
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
    QApplication::clipboard()->setData( mAttachments->dragObject(), QClipboard::Clipboard );
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
  mContextMenu->exec( pos );
}

#include "koeditorattachments.moc"
