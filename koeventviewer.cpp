/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// $Id$

#include <klocale.h>
#include <kapp.h>
#include <libkcal/event.h>
#include <libkcal/todo.h>
#include <kdebug.h>
#include <kiconloader.h>
#include "koeventviewer.h"
#include "koeventviewer.moc"
#include <krun.h>
#include <dcopclient.h>
#include <qcstring.h>
#include "korganizer.h"
KOEventViewer::KOEventViewer(QWidget *parent,const char *name)
  : QTextBrowser(parent,name)
{
}

KOEventViewer::~KOEventViewer()
{
}

void KOEventViewer::setSource(const QString& n)
{
  kdDebug() << "KOEventViewer::setSource(): " << n << endl;
  QString tmpStr;
  if (n.startsWith("mailto:")) {
    kapp->invokeMailer(n.mid(7),QString::null);
    //emit showIncidence(n);
    return;
  } else if (n.startsWith("uid:")) {
   DCOPClient *client = KApplication::kApplication()->dcopClient();
   const QByteArray noParamData;
   const QByteArray paramData;
   QByteArray replyData;
   QCString replyTypeStr;
   #define PING_ABBROWSER (client->call("kaddressbook", "KAddressBookIface", "interfaces()",  noParamData, replyTypeStr, replyData))
   bool foundAbbrowser = PING_ABBROWSER;

   if (foundAbbrowser)
   {
   //KAddressbook is already running, so just DCOP to it to bring up the contact editor
   //client->send("kaddressbook","KAddressBookIface",
   QDataStream arg(paramData, IO_WriteOnly);
   arg << n.mid(6);
   client->send("kaddressbook", "KAddressBookIface", "showContactEditor( QString )",  paramData);
   return;
   }  else {
   /*KaddressBook is not already running.  Pass it the UID of the contact via the command line while starting it - its neater.
   We start it without its main interface*/
    KIconLoader* iconLoader = new KIconLoader();
    QString iconPath = iconLoader->iconPath("go",KIcon::Small);
    KOrganizer::setStartedKAddressBook(true);
    tmpStr = "kaddressbook --editor-only --uid ";
    tmpStr += n.mid(6);
    bool result = KRun::runCommand(tmpStr,"KAddressBook",iconPath);
    return;
   }
  } else {
    //QTextBrowser::setSource(n);
  }
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
  formatAttendees(event);

  if (event->recurrence()->doesRecur()) {
    addTag("p","<em>" + i18n("This is a recurring event.") + "</em>");
  }

  formatReadOnly(event);

  setText(mText);
}

void KOEventViewer::appendTodo(Todo *event)
{
  addTag("h1",event->summary());
  
  if (event->hasDueDate()) {
    mText.append(i18n("<b>Due on:</b> %1").arg(event->dtDueStr()));
  }

  if (!event->description().isEmpty()) addTag("p",event->description());  

  formatCategories(event);
  formatAttendees(event);

  mText.append(i18n("<p><b>Priority:</b> %2</p>")
               .arg(QString::number(event->priority())));

  mText.append(i18n("<p><i>%1 % completed</i></p>")
                    .arg(event->percentComplete()));

  formatReadOnly(event);

  setText(mText);
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
  QPtrList<Attendee> attendees = event->attendees();
  if (attendees.count()) {
    addTag("h2",i18n("Attendees"));
    Attendee *a;
    mText.append("<ul>");
    for(a=attendees.first();a;a=attendees.next()) {

      mText += "<li><a href=\"uid:" + a->uid() + "\">";
      mText += a->name();
      mText += "</a>\n";
      kdDebug() << "formatAttendees: uid = " << a->uid() << endl;

      if (!a->email().isEmpty()) {
      KIconLoader* iconLoader = new KIconLoader();
      QString iconPath = iconLoader->iconPath("mail_generic",KIcon::Small);
      if (iconPath)
      {
      mText += "<a href=\"mailto:" + a->name() +" "+ "<" + a->email() + ">" + "\">";
      mText += "<IMG src=\"" + iconPath + "\">";
      mText += "</a>\n";
      }
      }
    }
    mText.append("</li></ul>");
  }
}

void KOEventViewer::formatReadOnly(Incidence *event)
{
  if (event->isReadOnly()) {
    addTag("p","<em>(" + i18n("read-only") + ")</em>");
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

void KOEventViewer::clearEvents(bool now)
{
  mText = "";
  if (now) setText(mText);
}

void KOEventViewer::addText(QString text)
{
  mText.append(text);
  setText(mText);
}
