// $Id$

#include <qlayout.h>
#include <qtextview.h>
#include <qtextcodec.h>
#include <qfileinfo.h>

#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
//#include <khtml_part.h>
#include <kstddirs.h>
#include <kmessagebox.h>

#include "calendar.h"
#include "calprinter.h"
#include "calendarview.h"
#include "version.h"

#include "kowhatsnextview.h"
#include "kowhatsnextview.moc"


KOWhatsNextView::KOWhatsNextView(Calendar *calendar, QWidget *parent,
                                 const char *name)
  : KOBaseView(calendar, parent, name)
{
  mView = new QTextView(this);
//  mViewer = new KHTMLPart(this);

  QBoxLayout *layoutTop = new QVBoxLayout(this);
  layoutTop->addWidget(mView);
//  layoutTop->addWidget(mView->widget());
}

KOWhatsNextView::~KOWhatsNextView()
{
}

int KOWhatsNextView::maxDatesHint()
{
  return 0;
}

int KOWhatsNextView::currentDateCount()
{
  return 0;
}

QList<Incidence> KOWhatsNextView::getSelected()
{
  QList<Incidence> eventList;

  return eventList;
}


void KOWhatsNextView::printPreview(CalPrinter *calPrinter, const QDate &fd,
                               const QDate &td)
{
  calPrinter->preview(CalPrinter::Day, fd, td);
}

void KOWhatsNextView::updateView()
{
  mText = i18n("<h1>What's next?</h1>");


  QList<Event> events = mCalendar->getEvents(QDate::currentDate(),
                                               QDate::currentDate().addDays(1));
  if (events.count() > 0) {
    mText += i18n("<h2>Events:</h2>");
    mText += i18n("<table>");
    Event *ev = events.first();
    while(ev) {
      appendEvent(ev);
      ev = events.next();
    }
    mText += i18n("</table>");
  }

  QList<Todo> todos = mCalendar->getTodoList();
  if (todos.count() > 0) {  
    mText += i18n("<h2>Todo:</h2>");
    mText += i18n("<table>");
    Todo *ev = todos.first();
    while(ev) {
      if (ev->priority() == 1 ||
          (ev->hasDueDate() && ev->dtDue().date() == QDate::currentDate()))
        appendTodo(ev);
      ev = todos.next();
    }
    mText += i18n("</table>");
  }

  mView->setText(mText);
  
#if 0
  mView->begin();
  mView->write(mText);
  mView->end();
#endif
}

void KOWhatsNextView::selectDates(const QDateList dateList)
{
  updateView();
}

void KOWhatsNextView::selectEvents(QList<Event> eventList)
{
}

void KOWhatsNextView::changeEventDisplay(Event *event, int action)
{
  switch(action) {
    case CalendarView::EVENTADDED:
      break;
    case CalendarView::EVENTEDITED:
      break;
    case CalendarView::EVENTDELETED:
      break;
    default:
      kdDebug() << "KOWhatsNextView::changeEventDisplay(): Illegal action " << action << endl;
  }
}

void KOWhatsNextView::appendEvent(Event *ev)
{
  mText += "<tr><td><b>";
  if (!ev->doesFloat()) {
    mText += ev->dtStartTimeStr() + " - " + ev->dtEndTimeStr();
  }
  mText += "</b></td><td>";
  mText += ev->summary();
  mText += "</td></tr>";
}

void KOWhatsNextView::appendTodo(Todo *ev)
{
  mText += "<tr><td><b>";
  mText += QString::number(ev->priority());
  mText += "</b></td><td>";
  mText += ev->summary();
  mText += "</td></tr>";
}

void KOWhatsNextView::displayAboutPage()
{
#if 0
  QString location = locate("data", "korganizer/about/main.html");
  QString content = kFileToString(location);
  mViewer->closeURL();
  mViewer->begin(location);
  QTextCodec *codec = QTextCodec::codecForName(KGlobal::locale()->charset());
  if (codec) mViewer->setCharset(codec->name(), true);
  else mViewer->setCharset(KGlobal::locale()->charset(), true);

  QString info =
    i18n("<h2>Welcome to KOrganizer %1</h2><p>KOrganizer is the personal "
    "organizer of calendaring and scheduling information for the K "
    "Desktop Environment. It is designed to be fully compatible with Internet "
    "calendaring standards including vCalendar and iCalendar.</p>\n"
    "<ul><li>KOrganizer has many powerful features which are described in the "
    "<A HREF=\"%2\">documentation</A></li>\n"
    "<li>You can find news and updates at the <A HREF=\"%3\">KOrganizer homepage"
    "</A></li></ul>\n").arg(korgVersion)
    .arg("help:korganizer")
    .arg("http://korganizer.kde.org/") +
    i18n("<p>Some of the new features in this release of KOrganizer include "
    "(compared to KOrganizer 2.1, which is part of KDE 2.1):</p>\n"
    "<ul>\n"
    "<li>iCalendar support</li>\n"
    "<li>Journal view</li>\n"
    "<li>Project view</li>\n"
    "</ul>\n");
  info += i18n("<p>We hope that you will enjoy KOrganizer.</p>\n"
    "<p>Thank you,</p>\n"
    "<p>&nbsp; &nbsp; The KOrganizer Team</p>");
  mViewer->write(content.arg(info));
  mViewer->end();
#endif
}

static void msgDialog(const QString &msg)
{
  KMessageBox::sorry(0, msg, i18n("File I/O Error"));
}

QString KOWhatsNextView::kFileToString(const QString &aFileName,bool aEnsureNL,
                                       bool aVerbose)
{
  QCString result;
  QFileInfo info(aFileName);
  unsigned int readLen;
  unsigned int len = info.size();
  QFile file(aFileName);

  //assert(aFileName!=NULL);
  if( aFileName == NULL)
    return "";

  if (!info.exists())
  {
    if (aVerbose)
      msgDialog(i18n("The specified file does not exist:\n%1").arg(aFileName));
    return QString::null;
  }
  if (info.isDir())
  {
    if (aVerbose)
      msgDialog(i18n("This is a directory and not a file:\n%1").arg(aFileName));
    return QString::null;
  }
  if (!info.isReadable())
  {
    if (aVerbose)
      msgDialog(i18n("You do not have read permissions "
				   "to the file:\n%1").arg(aFileName));
    return QString::null;
  }
  if (len <= 0) return QString::null;

  if (!file.open(IO_Raw|IO_ReadOnly))
  {
    if (aVerbose) switch(file.status())
    {
    case IO_ReadError:
      msgDialog(i18n("Could not read file:\n%1").arg(aFileName));
      break;
    case IO_OpenError:
      msgDialog(i18n("Could not open file:\n%1").arg(aFileName));
      break;
    default:
      msgDialog(i18n("Error while reading file:\n%1").arg(aFileName));
    }
    return QString::null;
  }

  result.resize(len + (int)aEnsureNL + 1);
  readLen = file.readBlock(result.data(), len);
  if (aEnsureNL && result[len-1]!='\n')
  {
    result[len++] = '\n';
    readLen++;
  }
  result[len] = '\0';

  if (readLen < len)
  {
    QString msg = i18n("Could only read %1 bytes of %2.")
		.arg(readLen).arg(len);
    msgDialog(msg);
    return QString::null;
  }

  return result;
}
