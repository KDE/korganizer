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
#include <qlabel.h>

#include <klocale.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <ktempfile.h>
#include <kurl.h>
#include <kio/job.h>
#include <kstddirs.h>
#include <kconfig.h>

#include "calobject.h"
#include "kdateedit.h"
#include "koprefs.h"

ExportWebDialog::ExportWebDialog (CalObject *cal, QWidget *parent,
                                  const char *name) :
  KDialogBase(Tabbed,i18n("Export calendar as web page"),
              Help|Default|User1|Cancel,User1,parent,name,false,false,
              i18n("Export")),
  mCalendar(cal)
{
  setupGeneralPage();
  setupEventPage();
  setupTodoPage();
// Disabled bacause the functionality is not yet implemented.
//  setupAdvancedPage();
  
  QObject::connect(this,SIGNAL(user1Clicked()),this,SLOT(exportWebPage()));
}

ExportWebDialog::~ExportWebDialog()
{
  delete mConfig;
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
  mOutputFileEdit = new QLineEdit(KOPrefs::instance()->mHtmlExportFile,
                                  outputFileLayout);
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
  
  mCbCategoriesTodo = new QCheckBox (i18n("Categories"),mTodoPage);
  topLayout->addWidget(mCbCategoriesTodo);
  
  mCbAttendeesTodo = new QCheckBox (i18n("Attendees"),mTodoPage);
  topLayout->addWidget(mCbAttendeesTodo);
  
  topLayout->addStretch(1);
}

void ExportWebDialog::setupEventPage()
{
  mEventPage = addPage(i18n("Event"));
  
  QVBoxLayout *topLayout = new QVBoxLayout(mEventPage, 10);
  
  mCbCategoriesEvent = new QCheckBox (i18n("Categories"),mEventPage);
  topLayout->addWidget(mCbCategoriesEvent);
  
  mCbAttendeesEvent = new QCheckBox (i18n("Attendees"),mEventPage);
  topLayout->addWidget(mCbAttendeesEvent);
  
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
//  kdDebug() << "ExportWebDialog::browseOutputFile()" << endl;

  KURL u = KFileDialog::getSaveURL();
  if(!u.isEmpty()) mOutputFileEdit->setText(u.prettyURL());

//  QString str = QFileDialog::getSaveFileName();
//  if(!str.isEmpty())
//    mOutputFileEdit->setText(str);
}

void ExportWebDialog::exportWebPage()
{
  kdDebug() << "ExportWebDialog::exportWebPage()" << endl;
  KTempFile tmpFile;
  kdDebug() << "ExportWebDialog::exportWebPage() tmpFile" << endl;
//  tmpFile.setAutoDelete(true);
  QTextStream *ts = tmpFile.textStream();
  kdDebug() << "ExportWebDialog::exportWebPage() textStream" << endl;
  
  // Write HTML header
  *ts << "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\" ";
  *ts << "\"http://www.w3.org/TR/REC-html40/loose.dtd\">\n";

  kdDebug() << "ExportWebDialog::exportWebPage() header" << endl;
  
  *ts << "<HTML><HEAD>" << endl;
  *ts << "  <TITLE>" << i18n("KOrganizer To-Do List") << "</TITLE>\n";
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
    kdDebug() << "ExportWebDialog::exportWebPage() evlist" << endl;
    *ts << "<H1>" << i18n("KOrganizer Calendar") << "</H1>\n";

    // Write HTML page content
    createHtmlEventList(ts);
  }

  // Write Todo List
  if (mCbTodo->isChecked()) {
    kdDebug() << "ExportWebDialog::exportWebPage() todolist" << endl;
    *ts << "<H1>" << i18n("KOrganizer To-Do List") << "</H1>\n";

    // Write HTML page content
    createHtmlTodoList(ts);
  }

  kdDebug() << "ExportWebDialog::exportWebPage() trailer" << endl;

  // Write KOrganizer trailer
  *ts << "<P>" << i18n("This page was created by <A HREF=\"http://"
        "korganizer.kde.org\">KOrganizer</A>") << "</P>\n";
  
  // Write HTML trailer
  *ts << "</BODY></HTML>\n";

  // Copy temporary file to final destination
  KURL src;
  src.setPath(tmpFile.name());

  KURL dest(mOutputFileEdit->text());
  // Remember destination.
  KOPrefs::instance()->mHtmlExportFile = mOutputFileEdit->text();

  kdDebug() << "ExportWebDialog::exportWebPage() move" << endl;
  
  KIO::Job *job = KIO::move(src,dest);
  connect(job,SIGNAL(result(KIO::Job *)),SLOT(slotResult(KIO::Job *)));
  kdDebug() << "ExportWebDialog::exportWebPage() done" << endl;
}

void ExportWebDialog::slotResult(KIO::Job *job)
{
  kdDebug() << "slotResult" << endl;
  int err = job->error();
  if (err)
  {
    kdDebug() << "Error " << err << ": " << job->errorString() << endl;
    job->showErrorDialog();
  } else {
    kdDebug() << "No Error" << endl;
    accept();
  }
  kdDebug() << "slotResult done" << endl;
}

void ExportWebDialog::createHtmlEventList (QTextStream *ts)
{
  *ts << "<TABLE BORDER=0 CELLPADDING=3 CELLSPACING=3>\n";
  *ts << "  <TR>\n";
  *ts << "    <TH CLASS=sum>" << i18n("Event") << "</TH>\n";
  *ts << "    <TH>" << i18n("Start Time") << "</TH>\n";
  *ts << "    <TH>" << i18n("End Time") << "</TH>\n";
  if (mCbCategoriesEvent->isChecked()) {
    *ts << "    <TH>" << i18n("Categories") << "</TH>\n";
  }
  if (mCbAttendeesEvent->isChecked()) {
    *ts << "    <TH>" << i18n("Attendees") << "</TH>\n";
  }

  *ts << "  </TR>\n";

  int columns = 3;
  if (mCbCategoriesEvent->isChecked()) ++columns;
  if (mCbAttendeesEvent->isChecked()) ++columns;

  QDate dt = mFromDate->getDate();
  for (dt = mFromDate->getDate(); dt <= mToDate->getDate();
       dt = dt.addDays(1)) {
    kdDebug() << "Getting events for " << dt.toString() << endl;
    QList<KOEvent> events = mCalendar->getEventsForDate(dt,true);
    if (events.count()) {
      *ts << "  <TR><TD COLSPAN=" << QString::number(columns)
          << " CLASS=datehead><I>"
          << KGlobal::locale()->formatDate(dt)
          << "</I></TD></TR>\n";
      KOEvent *ev;
      for(ev = events.first(); ev; ev = events.next()) {
        createHtmlEvent(ts,ev,dt);
      }
    }
  }
  
  *ts << "</TABLE>\n";  
}

void ExportWebDialog::createHtmlEvent (QTextStream *ts, KOEvent *event,
                                       QDate date)
{
  kdDebug() << "ExportWebDialog::createHtmlEvent()" << endl;
  *ts << "  <TR>\n";

  *ts << "    <TD CLASS=sum>\n";
  *ts << "      <B>" << event->getSummary() << "</B>\n";
  if (!event->getDescription().isEmpty()) {
    *ts << "      <P>" << event->getDescription() << "</P>\n";
  }
  *ts << "    </TD>\n";

  if (!event->doesFloat()) {
    if (event->getDtStart().date() == date) {
      *ts << "    <TD>" << event->getDtStartTimeStr() << "</TD>\n";
    } else {
      *ts << "    <TD>&nbsp;</TD>\n";
    }
    if (event->getDtEnd().date() == date) {
      *ts << "    <TD>" << event->getDtEndTimeStr() << "</TD>\n";
    } else {
      *ts << "    <TD>&nbsp;</TD>\n";
    }
  } else {
    *ts << "    <TD>&nbsp;</TD><TD>&nbsp;</TD>\n";
  }
  
  if (mCbCategoriesTodo->isChecked()) {
    *ts << "  <TD>\n";
    formatHtmlCategories(ts,event);  
    *ts << "  </TD>\n";  
  }

  if (mCbAttendeesTodo->isChecked()) {
    *ts << "  <TD>\n";
    formatHtmlAttendees(ts,event);
    *ts << "  </TD>\n";
  }
  
  *ts << "  </TR>\n";
}

void ExportWebDialog::createHtmlTodoList (QTextStream *ts)
{
  KOEvent *ev,*subev;
  
  QList<KOEvent> todoList = mCalendar->getTodoList();  
  
  *ts << "<TABLE BORDER=0 CELLPADDING=3 CELLSPACING=3>\n";
  *ts << "  <TR>\n";
  *ts << "    <TH CLASS=sum>" << i18n("Task") << "</TH>\n";
  *ts << "    <TH>" << i18n("Priority") << "</TH>\n";
  *ts << "    <TH>" << i18n("Status") << "</TH>\n";
  if (mCbDueDates->isChecked()) {
    *ts << "    <TH>" << i18n("Due Date") << "</TH>\n";
  }
  if (mCbCategoriesTodo->isChecked()) {
    *ts << "    <TH>" << i18n("Categories") << "</TH>\n";
  }
  if (mCbAttendeesTodo->isChecked()) {
    *ts << "    <TH>" << i18n("Attendees") << "</TH>\n";
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
      if (mCbCategoriesTodo->isChecked()) ++columns;
      if (mCbAttendeesTodo->isChecked()) ++columns;
      *ts << "\"" << QString::number(columns) << "\"";
      *ts << "><A NAME=\"sub" << ev->getVUID() << "\"></A>"
          << i18n("Sub-Tasks of: ") << "<A HREF=\"#"
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
  kdDebug() << "ExportWebDialog::createHtmlTodo()" << endl;

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
        << "\">" << i18n("Sub-Tasks") << "</A></DIV>\n";
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
  *ts << "    " << (completed ? i18n("not yet done","Done") : i18n("Open"))
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

  if (mCbCategoriesTodo->isChecked()) {
    *ts << "  <TD";
    if (completed) *ts << " CLASS=done";
    *ts << ">\n";
    formatHtmlCategories(ts,todo);  
    *ts << "  </TD>\n";  
  }

  if (mCbAttendeesTodo->isChecked()) {
    *ts << "  <TD";
    if (completed) *ts << " CLASS=done";
    *ts << ">\n";
    formatHtmlAttendees(ts,todo);
    *ts << "  </TD>\n";
  }

  *ts << "</TR>\n";
}

void ExportWebDialog::formatHtmlCategories (QTextStream *ts,KOEvent *event)
{
  if (!event->getCategoriesStr().isEmpty()) {
    *ts << "    " << event->getCategoriesStr() << "\n";
  } else {
    *ts << "    &nbsp;\n";
  }
}

void ExportWebDialog::formatHtmlAttendees (QTextStream *ts,KOEvent *event)
{
  QList<Attendee> attendees = event->getAttendeeList();
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
}
