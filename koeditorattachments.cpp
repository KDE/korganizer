/*
    This file is part of KOrganizer.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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

#include "urihandler.h"

#include <klocale.h>
#include <kdebug.h>
#include <kurlrequesterdlg.h>
#include <kmessagebox.h>
#include <libkcal/incidence.h>

#include <qlayout.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qwhatsthis.h>

KOEditorAttachments::KOEditorAttachments( int spacing, QWidget *parent,
                                          const char *name )
  : QWidget( parent, name )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setSpacing( spacing );

  mAttachments = new QListView( this );
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
}

KOEditorAttachments::~KOEditorAttachments()
{
}

void KOEditorAttachments::showAttachment( QListViewItem *item )
{
  if ( !item ) return;

  QString uri = item->text( 0 );

  UriHandler::process( uri );
}

void KOEditorAttachments::slotAdd()
{
  KURL uri = KURLRequesterDlg::getURL( QString::null, 0,
                                       i18n("Add Attachment") );
  if ( !uri.isEmpty() ) {
    new QListViewItem( mAttachments, uri.url() );
  }
}

void KOEditorAttachments::slotEdit()
{
  QListViewItem *item = mAttachments->currentItem();
  if ( !item ) return;

  KURL uri = KURLRequesterDlg::getURL( item->text( 0 ), 0,
                                       i18n("Edit Attachment") );

  if ( !uri.isEmpty() ) item->setText( 0, uri.url() );
}

void KOEditorAttachments::slotRemove()
{
  QListViewItem *item = mAttachments->currentItem();
  if ( !item ) return;

  if ( KMessageBox::warningContinueCancel(this,
        i18n("This item will be permanently deleted."),
        i18n("KOrganizer Confirmation"),KGuiItem(i18n("&Delete"),"editdelete")) == KMessageBox::Continue )
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
  new QListViewItem( mAttachments, uri, mimeType );
}

void KOEditorAttachments::readIncidence( Incidence *i )
{
  mAttachments->clear();

  Attachment::List attachments = i->attachments();
  Attachment::List::ConstIterator it;
  for( it = attachments.begin(); it != attachments.end(); ++it ) {
    QString uri;
    if ( (*it)->isUri() ) uri = (*it)->uri();
    else uri = i18n("[Binary data]");
    addAttachment( uri, (*it)->mimeType() );
  }
}

void KOEditorAttachments::writeIncidence( Incidence *i )
{
  i->clearAttachments();

  QListViewItem *item;
  for( item = mAttachments->firstChild(); item; item = item->nextSibling() ) {
    i->addAttachment( new Attachment( item->text( 0 ), item->text( 1 ) ) );
  }
}

#include "koeditorattachments.moc"
