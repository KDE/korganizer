// $Id$
//
// ExportWebDialog.cpp - Implementation of web page export dialog.
//

#include "exportwebdialog.h"
#include "exportwebdialog.moc"

#include <qlayout.h>
#include <qhgroupbox.h>
#include <qvgroupbox.h>
#include <qvbuttongroup.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qhbox.h>
#include <qfiledialog.h>
#include <qtextstream.h>

#include <klocale.h>
#include <kfiledialog.h>
#include <ktempfile.h>
#include <kurl.h>
#include <kio/job.h>

#include "calobject.h"
#include "kdated.h"

ExportWebDialog::ExportWebDialog (CalObject *cal, QWidget *parent,
                                  const char *name) :
  KDialogBase(Tabbed,i18n("Export calendar as web page"),
              Help|Default|User1|Cancel,User1,parent,name,false,false,
              i18n("Export")),
  mCalendar(cal)
{
  setupGeneralPage();
  setupTodoPage();
  setupAdvancedPage();
  
  QObject::connect(this,SIGNAL(user1Clicked()),this,SLOT(exportWebPage()));
}

ExportWebDialog::~ExportWebDialog()
{
}

void ExportWebDialog::setupGeneralPage()
{
  mGeneralPage = addPage(i18n("General"));

  QVBoxLayout *topLayout = new QVBoxLayout(mGeneralPage, 10);
  
  QGroupBox *rangeGroup = new QHGroupBox(i18n("Date Range"),mGeneralPage);
  topLayout->addWidget(rangeGroup);
  
  mFromDate = new KDateEdit(rangeGroup);
  mFromDate->setDate(QDate::currentDate());
  
  mToDate = new KDateEdit(rangeGroup);
  mToDate->setDate(QDate::currentDate());

  QButtonGroup *typeGroup = new QVButtonGroup(i18n("View Type"),mGeneralPage);
  topLayout->addWidget(typeGroup);
  
  
  // For now we just support the todo view. Other view types will follow
  // shortly.
//  new QRadioButton(i18n("Day"), typeGroup);
//  new QRadioButton(i18n("Week"), typeGroup);
//  new QRadioButton(i18n("Month"), typeGroup);

  mCbEvent = new QCheckBox(i18n("Event List"), typeGroup);
  mCbTodo = new QCheckBox(i18n("To-Do List"), typeGroup);
  mCbTodo->setChecked(true);  

  QGroupBox *destGroup = new QVGroupBox(i18n("Destination"),mGeneralPage);
  topLayout->addWidget(destGroup);

  new QLabel(i18n("Output File:"),destGroup);

  QHBox *outputFileLayout = new QHBox(destGroup);
  QString str = QDir::homeDirPath() + "/calendar.html";
  mOutputFileEdit = new QLineEdit(str,outputFileLayout);
  QPushButton *browseButton = new QPushButton(i18n("Browse"),outputFileLayout);
  QObject::connect(browseButton, SIGNAL(clicked()),
                   this, SLOT(browseOutputFile()));
  
  topLayout->addStretch(1);
}

void ExportWebDialog::setupTodoPage()
{
  mTodoPage = addPage(i18n("To-Do"));
  
  QVBoxLayout *topLayout = new QVBoxLayout(mTodoPage, 10);
  
  mCbDueDates = new QCheckBox (i18n("Due Dates"),mTodoPage);
  topLayout->addWidget(mCbDueDates);
  
  mCbAttendees = new QCheckBox (i18n("Attendees"),mTodoPage);
  topLayout->addWidget(mCbAttendees);
  
  topLayout->addStretch(1);
}

void ExportWebDialog::setupAdvancedPage()
{
  mAdvancedPage = addPage(i18n("Advanced"));
  
  QVBoxLayout *topLayout = new QVBoxLayout(mAdvancedPage, 10);
  
  mCbHtmlFragment = new QCheckBox (i18n("Only generate HTML fragment"),
                                   mAdvancedPage);
  topLayout->addWidget(mCbHtmlFragment);  
  
  QPushButton *colorsButton = new QPushButton(i18n("Colors"),mAdvancedPage);
  topLayout->addWidget(colorsButton);
  
  // Implement the functionality to enable this buttons.
  mCbHtmlFragment->setEnabled(false);
  colorsButton->setEnabled(false);
  
  topLayout->addStretch(1);
}

void ExportWebDialog::browseOutputFile()
{
// KFileDialog seems to be broken, use QFileDIalog instead
#if 0
  qDebug("ExportWebDialog::browseOutputFile()");
  KURL u = KFileDialog::getSaveURL();
//  KURL url = KFileDialog::getSaveURL(QString::null, QString::null, this,
//                                     i18n("Output File"));
  qDebug("ExportWebDialog::browseOutputFile() 1");
  QString str = u.path();
  qDebug("ExportWebDialog::browseOutputFile() 2");
  if(!str.isEmpty())
    qDebug("ExportWebDialog::browseOutputFile() 3");
    mOutputFileEdit->setText(str);
  qDebug("ExportWebDialog::browseOutputFile() done");
#endif

  QString str = QFileDialog::getSaveFileName();
  if(!str.isEmpty())
    mOutputFileEdit->setText(str);
}

void ExportWebDialog::exportWebPage()
{
  qDebug ("ExportWebDialog::exportWebPage()");
  KTempFile tmpFile;
  qDebug ("ExportWebDialog::exportWebPage() tmpFile");
//  tmpFile.setAutoDelete(true);
  QTextStream *ts = tmpFile.textStream();
  qDebug ("ExportWebDialog::exportWebPage() textStream");
  
  // Write HTML header
  *ts << "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\" ";
  *ts << "\"http://www.w3.org/TR/REC-html40/loose.dtd\">\n";

  qDebug ("ExportWebDialog::exportWebPage() header");
  
  *ts << "<HTML><HEAD>" << endl;
  *ts << "  <TITLE>KOrganizer To-Do List</TITLE>\n";
  *ts << "  <style type=\"text/css\">\n";
  *ts << "    body { background-color:white; color:black }\n";
  *ts << "    td { text-align:center; background-color:#eee }\n";
  *ts << "    th { text-align:center; background-color:#228; color:white }\n";
  *ts << "    td.sum { text-align:left }\n";
  *ts << "    td.sumdone { text-align:left; background-color:#ccc }\n";
  *ts << "    td.done { background-color:#ccc }\n";
  *ts << "    td.subhead { text-align:center; background-color:#ccf }\n";
  *ts << "    td.datehead { text-align:center; background-color:#ccf }\n";
  *ts << "    td.space { background-color:white }\n";
  *ts <<   "</style>\n";
  *ts << "</HEAD><BODY>\n";

  // TO DO: Write KOrganizer header
  // (Heading, Calendar-Owner, Calendar-Date, ...)
  
  // Write Event List
  if (mCbEvent->isChecked()) {
    qDebug ("ExportWebDialog::exportWebPage() evlist");
    *ts << "<H1>KOrganizer Calendar</H1>\n";

    // Write HTML page content
    createHtmlEventList(ts);
  }

  // Write Todo List
  if (mCbTodo->isChecked()) {
    qDebug ("ExportWebDialog::exportWebPage() todolist");
    *ts << "<H1>KOrganizer To-Do List</H1>\n";

    // Write HTML page content
    createHtmlTodoList(ts);
  }

  qDebug ("ExportWebDialog::exportWebPage() trailer");

  // Write KOrganizer trailer
  *ts << "<P>This page was created by <A HREF=\"http://"
      << "devel-home.kde.org/~korganiz\">KOrganizer</A></P>\n";
  
  // Write HTML trailer
  *ts << "</BODY></HTML>\n";

  // Copy temporary file to final destination
  KURL src;
  src.setPath(tmpFile.name());

  KURL dest;
  dest.setPath(mOutputFileEdit->text());

  qDebug ("ExportWebDialog::exportWebPage() move");
  
  KIO::Job *job = KIO::move(src,dest);
  connect(job,SIGNAL(result(KIO::Job *)),SLOT(slotResult(KIO::Job *)));
  qDebug ("ExportWebDialog::exportWebPage() done");
}

void ExportWebDialog::slotResult(KIO::Job *job)
{
  if (job->error())
  {
    job->showErrorDialog();
  } else {
    accept();
  }
}

void ExportWebDialog::createHtmlEventList (QTextStream *ts)
{
  *ts << "<TABLE BORDER=0 CELLPADDING=3 CELLSPACING=3>\n";
  *ts << "  <TR>\n";
  *ts << "    <TH CLASS=sum>Task</TH>\n";
  *ts << "    <TH>Start Time</TH>\n";
  *ts << "    <TH>End Time</TH>\n";
  *ts << "  </TR>\n";

  QDate dt = mFromDate->getDate();
  for (dt = mFromDate->getDate(); dt <= mToDate->getDate();
       dt = dt.addDays(1)) {
    qDebug("Getting events for %s",dt.toString().latin1());
    QList<KOEvent> events = mCalendar->getEventsForDate(dt,true);
    if (events.count()) {
      *ts << "  <TR><TD COLSPAN=3 CLASS=datehead><I>" << dt.toString()
          << "</I></TD></TR>\n";
      KOEvent *ev;
      for(ev = events.first(); ev; ev = events.next()) {
        createHtmlEvent(ts,ev);
      }
    }
  }
  
  *ts << "</TABLE>\n";  
}

void ExportWebDialog::createHtmlEvent (QTextStream *ts, KOEvent *event)
{
  qDebug("ExportWebDialog::createHtmlEvent()");
  *ts << "  <TR>\n";

  *ts << "    <TD CLASS=sum>\n";
  *ts << "      <B>" << event->getSummary() << "</B>\n";
  if (!event->getDescription().isEmpty()) {
    *ts << "      <P>" << event->getDescription() << "</P>\n";
  }
  *ts << "    </TD>\n";

  if (!event->doesFloat()) {
    *ts << "    <TD>" << event->getDtStartTimeStr() << "</TD>\n";
    *ts << "    <TD>" << event->getDtEndTimeStr() << "</TD>\n";
  } else {
    *ts << "    <TD>&nbsp;</TD><TD>&nbsp;</TD>\n";
  }
  
  *ts << "  </TR>\n";
}

void ExportWebDialog::createHtmlTodoList (QTextStream *ts)
{
  KOEvent *ev,*subev;
  
  QList<KOEvent> todoList = mCalendar->getTodoList();  
  
  *ts << "<TABLE BORDER=0 CELLPADDING=3 CELLSPACING=3>\n";
  *ts << "  <TR>\n";
  *ts << "    <TH CLASS=sum>Task</TH>\n";
  *ts << "    <TH>Priority</TH>\n";
  *ts << "    <TH>Status</TH>\n";
  if (mCbDueDates->isChecked()) {
    *ts << "    <TH>Due Date</TH>\n";
  }
  if (mCbAttendees->isChecked()) {
    *ts << "    <TH>Attendees</TH>\n";
  }
  *ts << "  </TR>\n";

  // Create top-level list.
  for(ev=todoList.first();ev;ev=todoList.next()) {
    if (!ev->getRelatedTo()) createHtmlTodo(ts,ev);
  }

  // Create sub-level lists
  for(ev=todoList.first();ev;ev=todoList.next()) {
    QList<KOEvent> relations = ev->getRelations();
    if (relations.count()) {
      // Generate sub-task list of event ev
      *ts << "  <TR>\n";
      *ts << "    <TD CLASS=subhead COLSPAN=";
      int columns = 3;
      if (mCbDueDates->isChecked()) ++columns;
      if (mCbAttendees->isChecked()) ++columns;
      *ts << "\"" << QString::number(columns) << "\"";
      *ts << "><A NAME=\"sub" << ev->getVUID() << "\"></A>"
          << "Sub-Tasks of: <A HREF=\"#"
          << ev->getVUID() << "\"><B>" << ev->getSummary() << "</B></A></TD>\n";
      *ts << "  </TR>\n";
      
      for(subev=relations.first();subev;subev=relations.next()) {
        createHtmlTodo(ts,subev);
      }
    }
  }

  *ts << "</TABLE>\n";
}

void ExportWebDialog::createHtmlTodo (QTextStream *ts,KOEvent *todo)
{
  qDebug("ExportWebDialog::createHtmlTodo()");

  bool completed = todo->getStatus() == KOEvent::COMPLETED;
  QList<KOEvent> relations = todo->getRelations();

  *ts << "<TR>\n";

  *ts << "  <TD CLASS=sum";
  if (completed) *ts << "done";
  *ts << ">\n";
  *ts << "    <A NAME=\"" << todo->getVUID() << "\"></A>\n";
  *ts << "    <B>" << todo->getSummary() << "</B>\n";
  if (!todo->getDescription().isEmpty()) {
    *ts << "    <P>" << todo->getDescription() << "</P>\n";
  }
  if (relations.count()) {
    *ts << "    <DIV ALIGN=right><A HREF=\"#sub" << todo->getVUID()
        << "\">Sub-Tasks</A></DIV>\n";
  }

  *ts << "  </TD";
  if (completed) *ts << " CLASS=done";
  *ts << ">\n";

  *ts << "  <TD";
  if (completed) *ts << " CLASS=done";
  *ts << ">\n";
  *ts << "    " << todo->getPriority() << "\n";
  *ts << "  </TD>\n";

  *ts << "  <TD";
  if (completed) *ts << " CLASS=done";
  *ts << ">\n";
  *ts << "    " << (completed ? "Done" : "Open")
      << "\n";
  *ts << "  </TD>\n";

  if (mCbDueDates->isChecked()) {
    *ts << "  <TD";
    if (completed) *ts << " CLASS=done";
    *ts << ">\n";
    if (todo->hasDueDate()) {
      *ts << "    " << todo->getDtDueDateStr() << "\n";
    } else {
      *ts << "    &nbsp;\n";
    }
    *ts << "  </TD>\n";
  }

  if (mCbAttendees->isChecked()) {
    *ts << "  <TD";
    if (completed) *ts << " CLASS=done";
    *ts << ">\n";
  
    QList<Attendee> attendees = todo->getAttendeeList();
    if (attendees.count()) {
      Attendee *a;
      for(a=attendees.first();a;a=attendees.next()) {
        *ts << "    " << a->getName();
        if (!a->getEmail().isEmpty()) *ts << " &lt;" << a->getEmail() << "&gt;";
        *ts << "<BR>" << "\n";
      }
    } else {
      *ts << "    &nbsp;\n";
    }

    *ts << "  </TD>\n";
  }

  *ts << "</TR>\n";
}
