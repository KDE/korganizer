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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "koeditorattachments.h"

#include <libkcal/incidence.h>

#include <klocale.h>
#include <kdebug.h>
#include <kurlrequesterdlg.h>
#include <kmessagebox.h>
#include <klistview.h>

#include <qlayout.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qdragobject.h>
#include <qwhatsthis.h>

class AttachmentListItem : public KListViewItem
{
  public:
    AttachmentListItem( KCal::Attachment*att, QListView *parent ) :
        KListViewItem( parent )
    {
      if ( att ) {
        mAttachment = new KCal::Attachment( *att );
      } else {
        mAttachment = new KCal::Attachment( QString::null );
      }
      readAttachment();
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

    void readAttachment()
    {
      if ( mAttachment->isUri() )
        setText( 0, mAttachment->uri() );
      else
        setText( 0, i18n("[Binary data]") );
      setText( 1, mAttachment->mimeType() );
    }

  private:
    KCal::Attachment *mAttachment;
};

KOEditorAttachments::KOEditorAttachments( int spacing, QWidget *parent,
                                          const char *name )
  : QWidget( parent, name )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setSpacing( spacing );

  mAttachments = new KListView( this );
  QWhatsThis::add( mAttachments,
                   i18n("Displays a list of current items (files, mail, etc.) "
                        "that have been associated with this event or to-do. "
                        "The URI column displays the location of the file.") );
  mAttachments->addColumn( i18n("URI") );
  mAttachments->addColumn( i18n("MIME Type") );
  topLayout->addWidget( mAttachments );
  connect( mAttachments, SIGNAL( doubleClicked( QListViewItem * ) ),
           SLOT( showAttachment( QListViewItem * ) ) );

  QBoxLayout *buttonLayout = new QHBoxLayout( topLayout );

  QPushButton *button = new QPushButton( i18n("&Add..."), this );
  QWhatsThis::add( button,
                   i18n("Shows a dialog used to select an attachment "
                        "to add to this event or to-do.") );
  buttonLayout->addWidget( button );
  connect( button, SIGNAL( clicked() ), SLOT( slotAdd() ) );

  button = new QPushButton( i18n("&Edit..."), this );
  QWhatsThis::add( button,
                   i18n("Shows a dialog used to edit the attachment "
                        "currently selected in the list above.") );
  buttonLayout->addWidget( button );
  connect( button, SIGNAL( clicked() ), SLOT( slotEdit() ) );

  button = new QPushButton( i18n("&Remove"), this );
  QWhatsThis::add( button,
                   i18n("Removes the attachment selected in the list above "
                        "from this event or to-do.") );
  buttonLayout->addWidget( button );
  connect( button, SIGNAL( clicked() ), SLOT( slotRemove() ) );

  button = new QPushButton( i18n("&Show"), this );
  QWhatsThis::add( button,
                   i18n("Opens the attachment selected in the list above "
                        "in the viewer that is associated with it in your "
                        "KDE preferences.") );
  buttonLayout->addWidget( button );
  connect( button, SIGNAL( clicked() ), SLOT( slotShow() ) );

  setAcceptDrops( TRUE );
}

KOEditorAttachments::~KOEditorAttachments()
{
}

void KOEditorAttachments::dragEnterEvent( QDragEnterEvent* event ) {
  event->accept( QTextDrag::canDecode( event ) );
}

void KOEditorAttachments::dropEvent( QDropEvent* event ) {
  QString text;
  int index;

  if ( QTextDrag::decode( event, text ) ) {
    if ( ( index = text.contains( '\n', FALSE ) ) <= 1 ) {
      addAttachment( text );
    } else {
      QString section;
      for ( int num = 0; num < index; num++ ) {
        section = text.section('\n', num, num );
        addAttachment( section );
      }
    }
  }
}

void KOEditorAttachments::showAttachment( QListViewItem *item )
{
  AttachmentListItem *attitem = static_cast<AttachmentListItem*>(item);
  if ( !attitem || !attitem->attachment() ) return;

  KCal::Attachment *att = attitem->attachment();
  if ( att->isUri() ) {
    emit openURL( att->uri() );
  } else {
    // FIXME: Handle binary attachments
  }
}

void KOEditorAttachments::slotAdd()
{
  KURL uri = KURLRequesterDlg::getURL( QString::null, 0,
                                       i18n("Add Attachment") );
  // TODO: Implement adding binary attachments
  if ( !uri.isEmpty() ) {
    addAttachment( uri.url() );
  }
}

void KOEditorAttachments::slotEdit()
{
  QListViewItem *item = mAttachments->currentItem();
  AttachmentListItem *attitem = static_cast<AttachmentListItem*>(item);
  if ( !attitem || !attitem->attachment() ) return;

  KCal::Attachment *att = attitem->attachment();
  if ( att->isUri() ) {
    KURL uri = KURLRequesterDlg::getURL( att->uri(), 0,
                                         i18n("Edit Attachment") );

    if ( !uri.isEmpty() )
      attitem->setUri( uri.url() );
  } else {
    // FIXME: Handle binary attachments
  }
}

void KOEditorAttachments::slotRemove()
{
  QListViewItem *item = mAttachments->currentItem();
  if ( !item ) return;

  if ( KMessageBox::warningContinueCancel(this,
        i18n("This item will be permanently deleted."),
  i18n("KOrganizer Confirmation"),KStdGuiItem::del()) == KMessageBox::Continue )
    delete item;
}

void KOEditorAttachments::slotShow()
{
  showAttachment( mAttachments->currentItem() );
}

void KOEditorAttachments::setDefaults()
{
  mAttachments->clear();
}

void KOEditorAttachments::addAttachment( const QString &uri,
                                         const QString &mimeType )
{
  AttachmentListItem *item = new AttachmentListItem( 0, mAttachments );
  item->setUri( uri );
  if ( !mimeType.isEmpty() ) item->setMimeType( mimeType );
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
}

void KOEditorAttachments::writeIncidence( KCal::Incidence *i )
{
  i->clearAttachments();

  QListViewItem *item;
  AttachmentListItem *attitem;
  for( item = mAttachments->firstChild(); item; item = item->nextSibling() ) {
    attitem = static_cast<AttachmentListItem*>(item);
    if ( attitem )
      i->addAttachment( new KCal::Attachment( *(attitem->attachment() ) ) );
  }
}

#include "koeditorattachments.moc"
