/*
  This file is part of KOrganizer.

  Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2005 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>

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

#include "koeditorattachments.h"

#include <kcal/incidence.h>

#include <libkdepim/kdepimprotocols.h>
#include <libkdepim/kvcarddrag.h>

#include <kio/job.h>
#include <kio/copyjob.h>
#include <kio/jobclasses.h>
#include <kio/jobuidelegate.h>
#include <kio/netaccess.h>

#include <k3iconview.h>
#include <klocale.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <kmenu.h>
#include <kprotocolmanager.h>
#include <krecentdocument.h>
#include <krun.h>
#include <kseparator.h>
#include <kstandarddirs.h>
#include <ktemporaryfile.h>
#include <kurlrequester.h>
#include <krandom.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kactioncollection.h>

#include <QCheckBox>
#include <QCursor>
#include <QFile>
#include <QFileInfo>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QRegExp>
#include <QString>
#include <QStringList>
#include <QStyle>
#include <QPixmap>
#include <QGridLayout>
#include <QBoxLayout>
#include <QDropEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDragEnterEvent>
#include <qapplication.h>
#include <qclipboard.h>

class AttachmentIconItem : public K3IconViewItem
{
  public:
    AttachmentIconItem( KCal::Attachment *att, Q3IconView *parent )
      : K3IconViewItem( parent )
    {
      if ( att ) {
        mAttachment = new KCal::Attachment( *att );
      } else {
        mAttachment = new KCal::Attachment( QString() );
      }
      readAttachment();
    }
    ~AttachmentIconItem() { delete mAttachment; }
    KCal::Attachment *attachment() const
    {
      return mAttachment;
    }
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
    bool isBinary() const
    {
      return mAttachment->isBinary();
    }
    QPixmap icon() const
    {
      return icon( KMimeType::mimeType( mAttachment->mimeType() ),
                   mAttachment->uri(), mAttachment->isBinary() );
    }
    static QPixmap icon( KMimeType::Ptr mimeType, const QString &uri,
                         bool binary = false )
    {
      QString iconStr = mimeType->iconName( uri );
      QStringList overlays;
      if ( !uri.isEmpty() && !binary ) {
        overlays << "emblem-link";
      }

      return KIconLoader::global()->loadIcon( iconStr, KIconLoader::Desktop, 0,
                                              KIconLoader::DefaultState,
                                              overlays );
    }

    void readAttachment()
    {
      if ( mAttachment->label().isEmpty() ) {
        if ( mAttachment->isUri() ) {
          setText( mAttachment->uri() );
        } else {
          setText( i18nc( "@label attachment contains binary data", "[Binary data]" ) );
        }
      } else {
        setText( mAttachment->label() );
      }

      setRenameEnabled( true );

      if ( mAttachment->mimeType().isEmpty() ||
           !( KMimeType::mimeType( mAttachment->mimeType() ) ) ) {
        KMimeType::Ptr mimeType;
        if ( mAttachment->isUri() ) {
          mimeType = KMimeType::findByUrl( mAttachment->uri() );
        } else {
          mimeType = KMimeType::findByContent( mAttachment->decodedData() );
        }
        mAttachment->setMimeType( mimeType->name() );
      }

      setPixmap( icon() );
    }

  private:
    KCal::Attachment *mAttachment;
};

AttachmentEditDialog::AttachmentEditDialog( AttachmentIconItem *item,
                                            QWidget *parent, bool modal )
  : KDialog ( parent ), mItem( item ), mURLRequester( 0 )
{
  // based loosely on KPropertiesDialog code
  QWidget *page = new QWidget(this);
  setMainWidget( page );
  setCaption( i18nc( "@title", "Properties for %1",
                     item->label().isEmpty() ? item->uri() : item->label() ) );
  setButtons( KDialog::Ok | KDialog::Cancel );
  setDefaultButton( KDialog::Ok );
  setModal( modal );
  QVBoxLayout *vbl = new QVBoxLayout( page );
  vbl->setSpacing( KDialog::spacingHint() );
  vbl->setMargin( 0 );
  QGridLayout *grid = new QGridLayout();
  grid->setColumnStretch( 0, 0 );
  grid->setColumnStretch( 1, 0 );
  grid->setColumnStretch( 2, 1 );
  grid->addItem( new QSpacerItem( KDialog::spacingHint(), 0 ), 0, 1 );
  vbl->addLayout( grid );

  mIcon = new QLabel( page );
  int bsize = 66 + 2 * mIcon->style()->pixelMetric( QStyle::PM_ButtonMargin );
  mIcon->setFixedSize( bsize, bsize );
  mIcon->setPixmap( item->icon() );
  grid->addWidget( mIcon, 0, 0, Qt::AlignLeft );

  mLabelEdit = new KLineEdit( page );
  mLabelEdit->setText( item->label().isEmpty() ? item->uri() : item->label() );
  mLabelEdit->setClickMessage( i18nc( "@label", "Attachment name" ) );
  mLabelEdit->setToolTip(
    i18nc( "@info", "Give the attachment a name" ) );
  mLabelEdit->setWhatsThis(
    i18nc( "@info", "Type any string you desire here for the name of the attachment" ) );
  grid->addWidget( mLabelEdit, 0, 2 );

  KSeparator *sep = new KSeparator( Qt::Horizontal, page );
  grid->addWidget( sep, 1, 0, 1, 3 );

  QLabel *label = new QLabel( i18nc( "@label", "Type:" ), page );
  grid->addWidget( label, 2, 0 );
  QString typecomment = item->mimeType().isEmpty() ?
                        i18nc( "@label unknown mimetype", "Unknown" ) :
                        KMimeType::mimeType( item->mimeType() )->comment();
  mTypeLabel = new QLabel( typecomment, page );
  grid->addWidget( mTypeLabel, 2, 2 );
  mMimeType = KMimeType::mimeType( item->mimeType() );

  mInline = new QCheckBox( i18nc( "@option:check", "Store attachment inline" ), page );
  grid->addWidget( mInline, 3, 0, 1, 3 );
  mInline->setChecked( item->isBinary() );
  mInline->setToolTip(
    i18nc( "@info", "Store the attachment file inside the calendar" ) );
  mInline->setWhatsThis(
    i18nc( "@info",
           "Checking this option will cause the attachment to be stored inside "
           "your calendar, which can take a lot of space depending on the size "
           "of the attachment. If this option is not checked, then only a link "
           "pointing to the attachment will be stored.  Do not use a link for "
           "attachments that change often or may be moved (or removed) from "
           "its current location." ) );

  if ( item->attachment()->isUri() ) {
    label = new QLabel( i18nc( "@label", "Location:" ), page );
    grid->addWidget( label, 4, 0 );
    mURLRequester = new KUrlRequester( item->uri(), page );
    mURLRequester->setToolTip(
      i18nc( "@info", "Provide a location for the attachment file" ) );
    mURLRequester->setWhatsThis(
      i18nc( "@info",
             "Enter the path to the attachment file or use the file browser "
             "by pressing the adjacent button" ) );
    grid->addWidget( mURLRequester, 4, 2 );
    connect( mURLRequester, SIGNAL(urlSelected(const KUrl &)),
             SLOT(urlChanged(const KUrl &)) );
    connect( mURLRequester, SIGNAL( textChanged( const QString& ) ),
             SLOT( urlChanged( const QString& ) ) );
    urlChanged(  item->uri() );
  } else {
    grid->addWidget( new QLabel( i18nc( "@label", "Size:" ), page ), 4, 0 );
    grid->addWidget( new QLabel( QString::fromLatin1( "%1 (%2)" ).
                                 arg( KIO::convertSize( item->attachment()->size() ) ).
                                 arg( KGlobal::locale()->formatNumber(
                                        item->attachment()->size(), 0 ) ), page ), 4, 2 );
  }
  vbl->addStretch( 10 );
}

void AttachmentEditDialog::slotApply()
{
  if ( mLabelEdit->text().isEmpty() ) {
    if ( mURLRequester->url().isLocalFile() ) {
      mItem->setLabel( mURLRequester->url().fileName() );
    } else {
      mItem->setLabel( mURLRequester->url().url() );
    }
  } else {
    mItem->setLabel( mLabelEdit->text() );
  }
  if ( mItem->label().isEmpty() ) {
    mItem->setLabel( i18nc( "@label", "New attachment" ) );
  }
  mItem->setMimeType( mMimeType->name() );
  if ( mURLRequester ) {
    if ( mInline->isChecked() ) {
      QString tmpFile;
      if ( KIO::NetAccess::download( mURLRequester->url(), tmpFile, this ) ) {
        QFile f( tmpFile );
        if ( !f.open( QIODevice::ReadOnly ) ) {
          return;
        }
        QByteArray data = f.readAll();
        f.close();
        mItem->setData( data );
      }
      KIO::NetAccess::removeTempFile( tmpFile );
    } else {
      mItem->setUri( mURLRequester->url().url() );
    }
  }
}

void AttachmentEditDialog::accept()
{
  slotApply();
  KDialog::accept();
}

void AttachmentEditDialog::urlChanged( const QString& url )
{
  enableButtonOk( !url.isEmpty() );
}

void AttachmentEditDialog::urlChanged( const KUrl &url )
{
  mMimeType = KMimeType::findByUrl( url );
  mTypeLabel->setText( mMimeType->comment() );
  mIcon->setPixmap( AttachmentIconItem::icon( mMimeType, url.path() ) );
}

class AttachmentIconView : public K3IconView
{
  friend class KOEditorAttachments;
  public:
    AttachmentIconView( QWidget *parent ) : K3IconView( parent )
    {
      setAcceptDrops( true );
      setSelectionMode( Q3IconView::Extended );
      setMode( K3IconView::Select );
      setItemTextPos( Q3IconView::Right );
      setArrangement( Q3IconView::LeftToRight );
      setMaxItemWidth( qMax( maxItemWidth(), 250 ) );
    }

    KUrl tempFileForAttachment( KCal::Attachment *attachment )
    {
      if ( mTempFiles.contains( attachment ) ) {
        return mTempFiles.value( attachment );
      }
      KTemporaryFile *file = new KTemporaryFile();
      file->setParent( this );

      QStringList patterns = KMimeType::mimeType( attachment->mimeType() )->patterns();
      if ( !patterns.empty() ) {
        file->setSuffix( QString( patterns.first() ).remove( '*' ) );
      }
      file->setAutoRemove( true );
      file->open();
      // read-only not to give the idea that it could be written to
      file->setPermissions( QFile::ReadUser );
      file->write( QByteArray::fromBase64( attachment->data() ) );
      mTempFiles.insert( attachment, file->fileName() );
      file->close();
      return mTempFiles.value( attachment );
    }

  protected:

    QMimeData *mimeData()
    {
      // create a list of the URL:s that we want to drag
      KUrl::List urls;
      QStringList labels;
      for ( Q3IconViewItem *it = firstItem(); it; it = it->nextItem() ) {
        if ( it->isSelected() ) {
          AttachmentIconItem *item = static_cast<AttachmentIconItem *>( it );
          if ( item->isBinary() ) {
            urls.append( tempFileForAttachment( item->attachment() ) );
          } else {
            urls.append( item->uri() );
          }
          labels.append( KUrl::toPercentEncoding( item->label() ) );
        }
      }
      if ( selectionMode() == Q3IconView::NoSelection ) {
        AttachmentIconItem *item = static_cast<AttachmentIconItem *>( currentItem() );
        if ( item ) {
          urls.append( item->uri() );
          labels.append( KUrl::toPercentEncoding( item->label() ) );
        }
      }

      QMap<QString, QString> metadata;
      metadata["labels"] = labels.join( ":" );

      QMimeData *mimeData = new QMimeData;
      urls.populateMimeData( mimeData, metadata );
      return mimeData;
    }

#ifdef __GNUC__
#warning Port to QDrag instead of Q3DragObject once we port the view from K3IconView
#endif
    virtual Q3DragObject *dragObject ()
    {
      int count = 0;
      for ( Q3IconViewItem *it = firstItem(); it; it = it->nextItem() ) {
        if ( it->isSelected() ) {
          ++count;
        }
      }

      QPixmap pixmap;
      if ( count > 1 ) {
        pixmap = KIconLoader::global()->loadIcon( "mail-attachment", KIconLoader::Desktop );
      }
      if ( pixmap.isNull() ) {
        pixmap = static_cast<AttachmentIconItem *>( currentItem() )->icon();
      }

      QPoint hotspot( pixmap.width() / 2, pixmap.height() / 2 );

      QDrag *drag = new QDrag( this );
      drag->setMimeData( mimeData() );

      drag->setPixmap( pixmap );
      drag->setHotSpot( hotspot );
      drag->exec( Qt::CopyAction );
      return 0;
    }

  private:
    QHash<KCal::Attachment*, KUrl> mTempFiles;
};

KOEditorAttachments::KOEditorAttachments( int spacing, QWidget *parent )
  : QWidget( parent )
{
  QBoxLayout *topLayout = new QHBoxLayout( this );
  topLayout->setSpacing( spacing );

  QLabel *label = new QLabel( i18nc( "@label", "Attachments:" ), this );
  topLayout->addWidget( label );

  mAttachments = new AttachmentIconView( this );
  mAttachments->setWhatsThis( i18nc( "@info",
                                     "Displays items (files, mail, etc.) that "
                                     "have been associated with this event or to-do." ) );
  mAttachments->setItemsMovable( false );
  mAttachments->setSelectionMode( Q3IconView::Extended );
  topLayout->addWidget( mAttachments );
  connect( mAttachments, SIGNAL(returnPressed(Q3IconViewItem *)),
           SLOT(showAttachment(Q3IconViewItem *)) );
  connect( mAttachments, SIGNAL(doubleClicked(Q3IconViewItem *)),
           SLOT(showAttachment(Q3IconViewItem *)) );
  connect( mAttachments, SIGNAL(itemRenamed(Q3IconViewItem *,const QString &)),
           SLOT(slotItemRenamed(Q3IconViewItem *,const QString &)) );
  connect( mAttachments, SIGNAL(dropped(QDropEvent *,const Q3ValueList<Q3IconDragItem> &)),
           SLOT(dropped(QDropEvent *,const Q3ValueList<Q3IconDragItem> &)) );
  connect( mAttachments, SIGNAL(selectionChanged()),
           SLOT(selectionChanged()) );
  connect( mAttachments, SIGNAL(contextMenuRequested(Q3IconViewItem *,const QPoint &)),
           SLOT(contextMenu(Q3IconViewItem *,const QPoint &)) );

  QPushButton *addButton = new QPushButton( this );
  addButton->setIcon( KIcon( "list-add" ) );
  addButton->setToolTip( i18nc( "@info", "Add an attachment" ) );
  addButton->setWhatsThis( i18nc( "@info",
                                  "Shows a dialog used to select an attachment "
                                  "to add to this event or to-do as link or as "
                                  "inline data." ) );
  topLayout->addWidget( addButton );
  connect( addButton, SIGNAL(clicked()), SLOT(slotAdd()) );

  mRemoveBtn = new QPushButton( this );
  mRemoveBtn->setIcon( KIcon( "list-remove" ) );
  mRemoveBtn->setToolTip( i18nc( "@info", "Remove the selected attachment" ) );
  mRemoveBtn->setWhatsThis( i18nc( "@info",
                                   "Removes the attachment selected in the "
                                   "list above from this event or to-do." ) );
  topLayout->addWidget( mRemoveBtn );
  connect( mRemoveBtn, SIGNAL(clicked()), SLOT(slotRemove()) );

  KActionCollection *ac = new KActionCollection( this );
  ac->addAssociatedWidget( this );

  mPopupMenu = new KMenu( this );

  mOpenAction = new KAction( i18nc( "@action:inmenu open the attachment in a viewer",
                                    "&Open" ), this );
  connect( mOpenAction, SIGNAL(triggered(bool)), this, SLOT(slotShow()) );
  ac->addAction( "view", mOpenAction );
  mPopupMenu->addAction( mOpenAction );
  mPopupMenu->addSeparator();

  mCopyAction = KStandardAction::copy( this, SLOT(slotCopy()), ac );
  mPopupMenu->addAction( mCopyAction );
  mCutAction = KStandardAction::cut( this, SLOT(slotCut()), ac );
  mPopupMenu->addAction( mCutAction );
  KAction *action = KStandardAction::paste( this, SLOT(slotPaste()), ac );
  mPopupMenu->addAction( action );
  mPopupMenu->addSeparator();

  mDeleteAction = new KAction( i18nc( "@action:inmenu remove the attachment",
                                      "&Delete" ), this );
  connect( mDeleteAction, SIGNAL(triggered(bool)), this, SLOT(slotRemove()) );
  ac->addAction( "remove", mDeleteAction );
  mPopupMenu->addAction( mDeleteAction );
  mPopupMenu->addSeparator();

  mEditAction = new KAction( i18nc( "@action:inmenu show a dialog used to edit the attachment",
                                    "&Properties..." ), this );
  connect( mEditAction, SIGNAL(triggered(bool)), this, SLOT(slotEdit()) );
  ac->addAction( "edit", mEditAction );
  mPopupMenu->addAction( mEditAction );

  selectionChanged();
  setAcceptDrops( true );
}

KOEditorAttachments::~KOEditorAttachments()
{
}

bool KOEditorAttachments::hasAttachments()
{
  return mAttachments->count() > 0;
}

void KOEditorAttachments::dragEnterEvent( QDragEnterEvent *event )
{
  const QMimeData *md = event->mimeData();
  event->setAccepted( KUrl::List::canDecode( md ) || md->hasText() );
}

void KOEditorAttachments::handlePasteOrDrop( const QMimeData *mimeData )
{
  KUrl::List urls;
  bool probablyWeHaveUris = false;
  bool weCanCopy = true;
  QStringList labels;

  if ( KPIM::KVCardDrag::canDecode( mimeData ) ) {
    KABC::Addressee::List addressees;
    KPIM::KVCardDrag::fromMimeData( mimeData, addressees );
    for ( KABC::Addressee::List::ConstIterator it = addressees.constBegin();
          it != addressees.constEnd(); ++it ) {
      urls.append( KDEPIMPROTOCOL_CONTACT + ( *it ).uid() );
      // there is some weirdness about realName(), hence fromUtf8
      labels.append( QString::fromUtf8( ( *it ).realName().toLatin1() ) );
    }
    probablyWeHaveUris = true;
  } else if ( KUrl::List::canDecode( mimeData ) ) {
    QMap<QString,QString> metadata;

    urls = KUrl::List::fromMimeData( mimeData, &metadata );
    probablyWeHaveUris = true;
    labels = metadata["labels"].split( ":", QString::SkipEmptyParts );
    for ( QStringList::Iterator it = labels.begin(); it != labels.end(); ++it ) {
      *it = KUrl::fromPercentEncoding( (*it).toLatin1() );
    }
  } else if ( mimeData->hasText() ) {
    QString text = mimeData->text();
    QStringList lst = text.split( '\n', QString::SkipEmptyParts );
    for ( QStringList::ConstIterator it = lst.begin(); it != lst.end(); ++it ) {
      urls.append( *it );
    }
    probablyWeHaveUris = true;
  }

  KMenu menu( this );
  QAction *linkAction = 0, *cancelAction;
  if ( probablyWeHaveUris ) {
    linkAction = menu.addAction( i18nc( "@action:inmenu", "&Link here" ) );
    // we need to check if we can reasonably expect to copy the objects
    for ( KUrl::List::ConstIterator it = urls.constBegin(); it != urls.constEnd(); ++it ) {
      if ( !( weCanCopy = KProtocolManager::supportsReading( *it ) ) ) {
        break; // either we can copy them all, or no copying at all
      }
    }
    if ( weCanCopy ) {
      menu.addAction( i18nc( "@action:inmenu", "&Copy here" ) );
    }
  } else {
    menu.addAction( i18nc( "@action:inmenu", "&Copy here" ) );
  }

  menu.addSeparator();
  cancelAction = menu.addAction( i18nc( "@action:inmenu", "C&ancel" ) );

  QAction *ret = menu.exec( QCursor::pos() );
  if ( linkAction == ret ) {
    QStringList::ConstIterator jt = labels.constBegin();
    for ( KUrl::List::ConstIterator it = urls.constBegin();
          it != urls.constEnd(); ++it ) {
      addAttachment( (*it).url(), QString(), ( jt == labels.constEnd() ?
                                               QString() : *( jt++ ) ) );
    }
  } else if ( cancelAction != ret ) {
    if ( probablyWeHaveUris ) {
      for ( KUrl::List::ConstIterator it = urls.constBegin();
            it != urls.constEnd(); ++it ) {
        KIO::Job *job = KIO::storedGet( *it );
        connect( job, SIGNAL(result(KJob *)), SLOT(downloadComplete(KJob *)) );
      }
    } else { // we take anything
      addAttachment( mimeData->data( mimeData->formats().first() ), mimeData->formats().first(),
                     KMimeType::mimeType( mimeData->formats().first() )->name() );
    }
  }
}

void KOEditorAttachments::dropEvent( QDropEvent *event )
{
  handlePasteOrDrop( event->mimeData() );
}

void KOEditorAttachments::downloadComplete( KJob *job )
{
  if ( job->error() ) {
    static_cast<KIO::Job*>(job)->ui()->setWindow( this );
    static_cast<KIO::Job*>(job)->ui()->showErrorMessage();
  } else {
    addAttachment( static_cast<KIO::StoredTransferJob *>( job )->data(),
                   QString(),
                   static_cast<KIO::SimpleJob *>( job )->url().fileName() );
  }
}

void KOEditorAttachments::dropped ( QDropEvent *e, const Q3ValueList<Q3IconDragItem> &lst )
{
  Q_UNUSED( lst );
  dropEvent( e );
}

void KOEditorAttachments::showAttachment( Q3IconViewItem *item )
{
  AttachmentIconItem *attitem = static_cast<AttachmentIconItem*>( item );
  if ( !attitem || !attitem->attachment() ) {
    return;
  }

  KCal::Attachment *att = attitem->attachment();
  if ( att->isUri() ) {
    emit openURL( att->uri() );
  } else {
    KRun::runUrl( mAttachments->tempFileForAttachment( att ), att->mimeType(), 0, true );
  }
}

void KOEditorAttachments::slotAdd()
{
  AttachmentIconItem *item = new AttachmentIconItem( 0, mAttachments );
  AttachmentEditDialog *dlg = new AttachmentEditDialog( item, mAttachments );

  dlg->setCaption( i18nc( "@title", "Add Attachment" ) );
  dlg->exec();

  if ( dlg->result() == KDialog::Rejected ) {
    delete dlg; // delete it first, as it hold item
    delete item;
  }
}

void KOEditorAttachments::slotEdit()
{
  for ( Q3IconViewItem *item = mAttachments->firstItem(); item; item = item->nextItem() ) {
    if ( item->isSelected() ) {
      AttachmentIconItem *attitem = static_cast<AttachmentIconItem*>( item );
      if ( !attitem || !attitem->attachment() ) {
        return;
      }

      AttachmentEditDialog *dialog = new AttachmentEditDialog( attitem, mAttachments, false );
      dialog->setModal( false );
      connect( dialog, SIGNAL(hidden()), dialog, SLOT(delayedDestruct()) );
      dialog->show();
    }
  }
}

void KOEditorAttachments::slotRemove()
{
  QList<Q3IconViewItem *> toDelete;
  for ( Q3IconViewItem *it = mAttachments->firstItem(); it; it = it->nextItem() ) {
    if ( it->isSelected() ) {
      AttachmentIconItem *item = static_cast<AttachmentIconItem *>( it );

      if ( !item ) {
        continue;
      }

      if ( KMessageBox::warningContinueCancel(
             this,
             i18nc( "@info",
                    "The item labeled \"%1\" will be permanently deleted.", item->label() ),
             i18nc( "@title:window", "KOrganizer Confirmation" ),
             KStandardGuiItem::del() ) == KMessageBox::Continue ) {
        toDelete.append( it );
      }
    }
  }

  for ( QList<Q3IconViewItem *>::ConstIterator it = toDelete.constBegin();
        it != toDelete.constEnd(); ++it ) {
    delete *it;
  }
}

void KOEditorAttachments::slotShow()
{
  for ( Q3IconViewItem *item = mAttachments->firstItem(); item; item = item->nextItem() ) {
    if ( item->isSelected() ) {
      showAttachment( item );
    }
  }
}

void KOEditorAttachments::setDefaults()
{
  mAttachments->clear();
}

void KOEditorAttachments::addAttachment( const QString &uri,
                                         const QString &mimeType,
                                         const QString &label,
                                         bool binary )
{
  if ( !binary ) {
    AttachmentIconItem *item = new AttachmentIconItem( 0, mAttachments );
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
        item->setMimeType( KMimeType::findByUrl( uri )->name() );
      }
    } else {
      QString tmpFile;
      if ( KIO::NetAccess::download( uri, tmpFile, this ) ) {
        QFile f( tmpFile );
        if ( !f.open( QIODevice::ReadOnly ) ) {
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
  AttachmentIconItem *item = new AttachmentIconItem( 0, mAttachments );
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
  new AttachmentIconItem( attachment, mAttachments );
}

void KOEditorAttachments::readIncidence( KCal::Incidence *i )
{
  mAttachments->clear();

  KCal::Attachment::List attachments = i->attachments();
  KCal::Attachment::List::ConstIterator it;
  for ( it = attachments.begin(); it != attachments.end(); ++it ) {
    addAttachment( (*it) );
  }

  mUid = i->uid();

  if ( mAttachments->count() > 0 ) {
    QTimer::singleShot( 0, mAttachments, SLOT(arrangeItemsInGrid()) );
  }
}

void KOEditorAttachments::writeIncidence( KCal::Incidence *i )
{
  i->clearAttachments();

  Q3IconViewItem *item;
  AttachmentIconItem *attitem;
  for ( item = mAttachments->firstItem(); item; item = item->nextItem() ) {
    attitem = static_cast<AttachmentIconItem*>(item);
    if ( attitem ) {
      i->addAttachment( new KCal::Attachment( *( attitem->attachment() ) ) );
    }
  }
}

void KOEditorAttachments::slotItemRenamed ( Q3IconViewItem *item, const QString &text )
{
  static_cast<AttachmentIconItem *>( item )->setLabel( text );
}

void KOEditorAttachments::applyChanges()
{
}

void KOEditorAttachments::slotCopy()
{
  QApplication::clipboard()->setMimeData( mAttachments->mimeData(), QClipboard::Clipboard );
}

void KOEditorAttachments::slotCut()
{
  slotCopy();
  slotRemove();
}

void KOEditorAttachments::slotPaste()
{
  handlePasteOrDrop( QApplication::clipboard()->mimeData() );
}

void KOEditorAttachments::selectionChanged()
{
  bool selected = false;
  for ( Q3IconViewItem *item = mAttachments->firstItem(); item; item = item->nextItem() ) {
    if ( item->isSelected() ) {
      selected = true;
      break;
    }
  }
  mRemoveBtn->setEnabled( selected );
}

void KOEditorAttachments::contextMenu( Q3IconViewItem *item, const QPoint &pos )
{
  const bool enable = item != 0;
  mOpenAction->setEnabled( enable );
  mCopyAction->setEnabled( enable );
  mCutAction->setEnabled( enable );
  mDeleteAction->setEnabled( enable );
  mEditAction->setEnabled( enable );
  mPopupMenu->exec( pos );
}

#include "koeditorattachments.moc"
