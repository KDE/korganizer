/*
    This file is part of KOrganizer.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include "koeditoralarms.h"

//#include "urihandler.h"

#include <klocale.h>
#include <kdebug.h>
// #include <kurlrequesterdlg.h>
// #include <kmessagebox.h>*/

#include <libkcal/alarm.h>
#include <libkcal/incidence.h>

#include <qlayout.h>
#include <qlistview.h>
//#include <qpushbutton.h>

class AlarmListViewItem : public QListViewItem
{
  public:
    AlarmListViewItem( QListView *parent, KCal::Alarm *alarm );
    virtual ~AlarmListViewItem();
    Alarm *alarm() const { return mAlarm; }
    void construct();
  protected:
    Alarm *mAlarm;
};

AlarmListViewItem::AlarmListViewItem( QListView *parent, Alarm *alarm )
    : QListViewItem( parent ), mAlarm( alarm )
{
  construct();
}

AlarmListViewItem::~AlarmListViewItem()
{
}

void AlarmListViewItem::construct()
{
  if ( mAlarm ) {
  
    // Alarm offset:
    QString startend;
    int offset;
    if ( mAlarm->hasStartOffset() ) {
      offset = mAlarm->startOffset().asSeconds();
      setText( 2, i18n( "start" ) );
    } else if ( mAlarm->hasEndOffset() ) {
      offset = mAlarm->endOffset().asSeconds();
      setText( 2, i18n( "end" ) );
    }
    
    QString beforeafter;
    if ( offset<0 ) {
      offset = -offset;
      setText( 1, i18n("after") );
    } else {
      setText( 1, i18n("before") );
    }

    offset = offset / 60; // make minutes
    int useoffset = offset;
    
    if ( offset % (24*60) == 0 && offset>0 ) { // divides evenly into days?
      useoffset = offset / (24*60);
      setText( 0, i18n("1 day", "%n days", useoffset ) );
    } else if (offset % 60 == 0 && offset>0 ) { // divides evenly into hours?
      useoffset = offset / 60;
      setText( 0, i18n("1 hour", "%n hours", useoffset ) );
    } else {
      useoffset = offset;
      setText( 0, i18n("1 minute", "%n minutes", useoffset ) );
    }
    
    // Alarm type:
    QString type( i18n("Unknown") );
    switch ( mAlarm->type() ) {
      case Alarm::Display: type = i18n("Reminder Dialog");
        break;
      case Alarm::Procedure: type = i18n("Application/Script");
        break;
      case Alarm::Email: type = i18n("Email");
        break;
      case Alarm::Audio: type = i18n("Audio");
        break;
      default: break;
    }
    setText( 3, type );
    
    // Alarm repeat
    if ( mAlarm->repeatCount()>0 ) {
      setText( 4, i18n("%1 times every %2 minutes").arg( mAlarm->repeatCount() )
                  .arg( mAlarm->snoozeTime() ) );
    }
  }
}


KOEditorAlarms::KOEditorAlarms( int spacing, QWidget *parent,
                                const char *name )
  : KOEditorAlarms_base( parent, name )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setSpacing( spacing );
}

KOEditorAlarms::~KOEditorAlarms()
{
}

void KOEditorAlarms::slotAdd()
{
/*  KURL uri = KURLRequesterDlg::getURL( QString::null, 0,
                                       i18n("Add Attachment") );
  if ( !uri.isEmpty() ) {
    new QListViewItem( mAttachments, uri.url() );
  }*/
}

void KOEditorAlarms::slotEdit()
{
/*  QListViewItem *item = mAttachments->currentItem();
  if ( !item ) return;

  KURL uri = KURLRequesterDlg::getURL( item->text( 0 ), 0,
                                       i18n("Edit Attachment") );

  if ( !uri.isEmpty() ) item->setText( 0, uri.url() );*/
}

void KOEditorAlarms::slotRemove()
{
/*  QListViewItem *item = mAttachments->currentItem();
  if ( !item ) return;

  if ( KMessageBox::warningContinueCancel(this,
        i18n("This item will be permanently deleted."),
        i18n("KOrganizer Confirmation"),KGuiItem(i18n("&Delete"),"editdelete")) == KMessageBox::Continue )
    delete item;*/
}

void KOEditorAlarms::setDefaults()
{
  mAlarmList->clear();
}

void KOEditorAlarms::readIncidence( Incidence *i )
{
  mAlarmList->clear();
  
  Alarm::List alarms = i->alarms();
  Alarm::List::ConstIterator it;
  for ( it = alarms.begin(); it != alarms.end(); ++it ) {
    new AlarmListViewItem( mAlarmList, *it );
    
  }

/*  Attachment::List attachments = i->attachments();
  Attachment::List::ConstIterator it;
  for( it = attachments.begin(); it != attachments.end(); ++it ) {
    QString uri;
    if ( (*it)->isUri() ) uri = (*it)->uri();
    else uri = i18n("[Binary data]");
    addAttachment( uri, (*it)->mimeType() );
  }*/
}

void KOEditorAlarms::writeIncidence( Incidence *i )
{
  i->clearAlarms();

/*  QListViewItem *item;
  for( item = mAttachments->firstChild(); item; item = item->nextSibling() ) {
    i->addAttachment( new Attachment( item->text( 0 ), item->text( 1 ) ) );
  }*/
}

#include "koeditoralarms.moc"
