/*
    This file is part of KOrganizer.

    Copyright (c) 2001,2003 Cornelius Schumacher <schumacher@kde.org>

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

#include "koeventviewer.h"

#include "kocore.h"
#include "urihandler.h"

#include <libkcal/event.h>
#include <libkcal/todo.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kapplication.h>
#include <kdebug.h>
#ifndef KORG_NOKABC
 #include <kabc/stdaddressbook.h>
#endif

KOEventViewer::KOEventViewer(QWidget *parent,const char *name)
  : QTextBrowser(parent,name)
{
}

KOEventViewer::~KOEventViewer()
{
}

void KOEventViewer::setSource( const QString &n )
{
  UriHandler::process( n );
}

void KOEventViewer::addTag(const QString & tag,const QString & text)
{
  int number=text.contains("\n");
  QString str = "<" + tag + ">";
  QString tmpText=text;
  QString tmpStr=str;
  if(number !=-1)
    {
      if (number > 0) {
        int pos=0;
        QString tmp;
        for(int i=0;i<=number;i++) {
          pos=tmpText.find("\n");
          tmp=tmpText.left(pos);
          tmpText=tmpText.right(tmpText.length()-pos-1);
          tmpStr+=tmp+"<br>";
        }
      }
      else tmpStr += tmpText;
      tmpStr+="</" + tag + ">";
      mText.append(tmpStr);
    }
  else
    {
      str += text + "</" + tag + ">";
      mText.append(str);
    }
}

void KOEventViewer::appendEvent(Event *event)
{
  addTag("h1",event->summary());

  if (!event->location().isEmpty()) {
    addTag("b",i18n("Location: "));
    mText.append(event->location()+"<br>");
  }
  if (event->doesFloat()) {
    if (event->isMultiDay()) {
      mText.append(i18n("<b>From:</b> %1 <b>To:</b> %2")
                   .arg(event->dtStartDateStr())
                   .arg(event->dtEndDateStr()));
    } else {
      mText.append(i18n("<b>On:</b> %1").arg(event->dtStartDateStr()));
    }
  } else {
    if (event->isMultiDay()) {
      mText.append(i18n("<b>From:</b> %1 <b>To:</b> %2")
                   .arg(event->dtStartStr())
                   .arg(event->dtEndStr()));
    } else {
      mText.append(i18n("<b>On:</b> %1 <b>From:</b> %2 <b>To:</b> %3")
                   .arg(event->dtStartDateStr())
                   .arg(event->dtStartTimeStr())
                   .arg(event->dtEndTimeStr()));
    }
  }

  if (!event->description().isEmpty()) addTag("p",event->description());

  formatCategories(event);

  if (event->doesRecur()) {
    addTag("p","<em>" + i18n("This is a recurring event.") + "</em>");
  }

  formatReadOnly( event );
  formatAttendees( event );
  formatAttachments( event );

  setText( mText );
}

void KOEventViewer::appendTodo( Todo *todo )
{
  addTag( "h1", todo->summary() );

  if ( !todo->location().isEmpty() ) {
    addTag( "b", i18n("Location:") );
    mText.append( todo->location() + "<br>" );
  }
  if ( todo->hasDueDate() ) {
    mText.append( i18n("<b>Due on:</b> %1").arg( todo->dtDueStr() ) );
  }

  if ( !todo->description().isEmpty() ) addTag( "p", todo->description() );

  formatCategories( todo );

  mText.append( i18n("<p><b>Priority:</b> %2</p>")
                .arg( QString::number( todo->priority() ) ) );

  mText.append( i18n("<p><i>%1 % completed</i></p>")
                     .arg( todo->percentComplete() ) );

  formatReadOnly( todo );
  formatAttendees( todo );
  formatAttachments( todo );

  setText( mText );
}

void KOEventViewer::formatCategories(Incidence *event)
{
  if (!event->categoriesStr().isEmpty()) {
    if (event->categories().count() == 1) {
      addTag("h2",i18n("Category"));
    } else {
      addTag("h2",i18n("Categories"));
    }
    addTag("p",event->categoriesStr());
  }
}

void KOEventViewer::formatAttendees(Incidence *event)
{
  Attendee::List attendees = event->attendees();
  if ( attendees.count() ) {
    KIconLoader* iconLoader = new KIconLoader();
    QString iconPath = iconLoader->iconPath("mail_generic",KIcon::Small);
    addTag("h3",i18n("Organizer"));
    mText.append("<ul><li>");
#ifndef KORG_NOKABC
    KABC::AddressBook *add_book = KABC::StdAddressBook::self();
    KABC::Addressee::List addressList;
    addressList = add_book->findByEmail(event->organizer());
    KABC::Addressee o = addressList.first();
    if ( !o.isEmpty() && addressList.size() < 2 ) {
      addLink( "uid" + o.uid(), o.formattedName() );
    } else {
      mText.append( event->organizer() );
    }
#else
    mText.append( event->organizer() );
#endif
    if ( !iconPath.isNull() ) {
      addLink( "mailto:" + event->organizer(),
               "<img src=\"" + iconPath + "\">" );
    }
    mText.append("</li></ul>");

    addTag("h3",i18n("Attendees"));
    mText.append("<ul>");
    Attendee::List::ConstIterator it;
    for( it = attendees.begin(); it != attendees.end(); ++it ) {
      Attendee *a = *it;
#ifndef KORG_NOKABC
      if ( a->name().isEmpty() ) {
        addressList = add_book->findByEmail( a->email() );
        KABC::Addressee o = addressList.first();
        if ( !o.isEmpty() && addressList.size() < 2 ) {
          addLink( "uid" + o.uid(), o.formattedName() );
        } else {
          mText += "<li>";
          mText.append( a->email() );
          mText += "\n";
        }
      } else {
        mText += "<li><a href=\"uid:" + a->uid() + "\">";
        if (!a->name().isEmpty()) mText += a->name();
        else mText += a->email();
        mText += "</a>\n";
      }
#else
      mText += "<li><a href=\"uid:" + a->uid() + "\">";
      if (!a->name().isEmpty()) mText += a->name();
      else mText += a->email();
      mText += "</a>\n";
#endif
      kdDebug(5850) << "formatAttendees: uid = " << a->uid() << endl;

      if (!a->email().isEmpty()) {
        if (!iconPath.isNull()) {
          mText += "<a href=\"mailto:" + a->name() +" "+ "<" + a->email() + ">" + "\">";
          mText += "<img src=\"" + iconPath + "\">";
          mText += "</a>\n";
        }
      }
    }
    mText.append("</li></ul>");
  }
}

void KOEventViewer::formatReadOnly( Incidence *i )
{
  if ( i->isReadOnly() ) {
    addTag( "p", "<em>(" + i18n("read-only") + ")</em>" );
  }
}

void KOEventViewer::formatAttachments( Incidence *i )
{
  Attachment::List as = i->attachments();
  if ( as.count() > 0 ) {
    mText += "<ul>";
    Attachment::List::ConstIterator it;
    for( it = as.begin(); it != as.end(); ++it ) {
      if ( (*it)->isUri() ) {
        mText += "<li>";
        addLink( (*it)->uri(), (*it)->uri() );
        mText += "</li>";
      }
    }
    mText += "</ul>";
  }
}

void KOEventViewer::setTodo(Todo *event)
{
  clearEvents();
  appendTodo(event);
}

void KOEventViewer::setEvent(Event *event)
{
  clearEvents();
  appendEvent(event);
}

void KOEventViewer::addEvent(Event *event)
{
  appendEvent(event);
}

void KOEventViewer::clearEvents(bool now)
{
  mText = "";
  if (now) setText(mText);
}

void KOEventViewer::addText( const QString &text )
{
  mText.append( text );
  setText( mText );
}

void KOEventViewer::addLink( const QString &ref, const QString &text,
                             bool newline )
{
  mText += "<a href=\"" + ref + "\">" + text + "</a>";
  if ( newline ) mText += "\n";
}

#include "koeventviewer.moc"
