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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <qcstring.h>

#include <klocale.h>
#include <kapplication.h>
#include <libkcal/event.h>
#include <libkcal/todo.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <krun.h>
#include <kprocess.h>
#ifndef KORG_NOKABC
 #include <kabc/stdaddressbook.h>
#endif

#ifndef KORG_NODCOP
#include <dcopclient.h>
#include "korganizer.h"
#include "actionmanager.h"
#endif

#include "koeventviewer.h"
#include "koeventviewer.moc"

KOEventViewer::KOEventViewer(QWidget *parent,const char *name)
  : QTextBrowser(parent,name)
{
}

KOEventViewer::~KOEventViewer()
{
}

void KOEventViewer::setSource(const QString& n)
{
#ifndef KORG_NODCOP
  kdDebug() << "KOEventViewer::setSource(): " << n << endl;
  QString tmpStr;
  if (n.startsWith("mailto:")) {
    KApplication::kApplication()->invokeMailer(n.mid(7),QString::null);
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

    if (foundAbbrowser) {
      //KAddressbook is already running, so just DCOP to it to bring up the contact editor
      //client->send("kaddressbook","KAddressBookIface",
      QDataStream arg(paramData, IO_WriteOnly);
      arg << n.mid(6);
      client->send("kaddressbook", "KAddressBookIface", "showContactEditor( QString )",  paramData);
      return;
    } else {
      /*
        KaddressBook is not already running.  Pass it the UID of the contact via the command line while starting it - its neater.
        We start it without its main interface
      */
      KIconLoader* iconLoader = new KIconLoader();
      QString iconPath = iconLoader->iconPath("go",KIcon::Small);
      ActionManager::setStartedKAddressBook(true);
      tmpStr = "kaddressbook --editor-only --uid ";
      tmpStr += KProcess::quote(n.mid(6));
      KRun::runCommand(tmpStr,"KAddressBook",iconPath);
      return;
    }
  } else {
    //QTextBrowser::setSource(n);
  }
#endif
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

  if (event->recurrence()->doesRecur()) {
    addTag("p","<em>" + i18n("This is a recurring event.") + "</em>");
  }

  formatReadOnly(event);
  formatAttendees(event);

  setText(mText);
}

void KOEventViewer::appendTodo(Todo *event)
{
  addTag("h1",event->summary());

  if (!event->location().isEmpty()) {
    addTag("b",i18n("Location: "));
    mText.append(event->location()+"<br>");
  }
  if (event->hasDueDate()) {
    mText.append(i18n("<b>Due on:</b> %1").arg(event->dtDueStr()));
  }

  if (!event->description().isEmpty()) addTag("p",event->description());

  formatCategories(event);

  mText.append(i18n("<p><b>Priority:</b> %2</p>")
               .arg(QString::number(event->priority())));

  mText.append(i18n("<p><i>%1 % completed</i></p>")
                    .arg(event->percentComplete()));

  formatReadOnly(event);
  formatAttendees(event);

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
    KIconLoader* iconLoader = new KIconLoader();
    QString iconPath = iconLoader->iconPath("mail_generic",KIcon::Small);
    addTag("h3",i18n("Organizer"));
    mText.append("<ul><li>");
#ifndef KORG_NOKABC
    KABC::AddressBook *add_book = KABC::StdAddressBook::self();
    KABC::Addressee::List addressList;
    addressList = add_book->findByEmail(event->organizer());
    KABC::Addressee o = addressList.first();
    if (!o.isEmpty() && addressList.size()<2) {
      mText += "<a href=\"uid:" + o.uid() + "\">";
      mText += o.formattedName();
      mText += "</a>\n";
    } else {
      mText.append(event->organizer());
    }
#else
    mText.append(event->organizer());
#endif
    if (iconPath) {
      mText += " <a href=\"mailto:" + event->organizer() + "\">";
      mText += "<IMG src=\"" + iconPath + "\">";
      mText += "</a>\n";
    }
    mText.append("</li></ul>");

    addTag("h3",i18n("Attendees"));
    Attendee *a;
    mText.append("<ul>");
    for(a=attendees.first();a;a=attendees.next()) {
#ifndef KORG_NOKABC
      if (a->name().isEmpty()) {
        addressList = add_book->findByEmail(a->email());
        KABC::Addressee o = addressList.first();
        if (!o.isEmpty() && addressList.size()<2) {
          mText += "<a href=\"uid:" + o.uid() + "\">";
          mText += o.formattedName();
          mText += "</a>\n";
        } else {
	  mText += "<li>";
          mText.append(a->email());
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
      kdDebug() << "formatAttendees: uid = " << a->uid() << endl;

      if (!a->email().isEmpty()) {
        if (iconPath) {
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

void KOEventViewer::addEvent(Event *event)
{
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
