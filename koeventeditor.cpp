// $Id$

#include <stdio.h>

#include <qtooltip.h>
#include <qframe.h>
#include <qpixmap.h>
#include <qlayout.h>
#include <qwidgetstack.h>

#include <kdatepik.h>
#include <kiconloader.h>
#include <kapp.h>
#include <kdebug.h>
#include <klocale.h>
#include <ktoolbarbutton.h>
#include <kstddirs.h>

#include "koprefs.h"
#include "categoryselectdialog.h"
#include "koprefs.h"

#include "koeventeditor.h"
#include "koeventeditor.moc"

KOEventEditor::KOEventEditor(Calendar *calendar) :
  KOIncidenceEditor(i18n("Edit Event"),calendar)
{
  mEvent = 0;
}

KOEventEditor::~KOEventEditor()
{
}

QWidget *KOEventEditor::setupGeneralTabWidget(QWidget *parent)
{
  mGeneral = new KOEditorGeneralEvent(spacingHint(),parent);

  return mGeneral;
}

void KOEventEditor::setupCustomTabs()
{
  setupRecurrenceTab();

  // Propagate date time settings to recurrence tab
  connect(mGeneral,SIGNAL(dateTimesChanged(QDateTime,QDateTime)),
          mRecurrence,SLOT(setDateTimes(QDateTime,QDateTime)));
  connect(mGeneral,SIGNAL(dateTimeStrChanged(const QString &)),
          mRecurrence,SLOT(setDateTimeStr(const QString &)));

  // Enable/Disable recurrence tab
  connect(mGeneral,SIGNAL(recursChanged(bool)),
          SLOT(enableRecurrence(bool)));

  // Category dialog
  connect(mGeneral,SIGNAL(openCategoryDialog()),mCategoryDialog,SLOT(show()));
  connect(mCategoryDialog,SIGNAL(categoriesSelected(QString)),
          mGeneral,SLOT(setCategories(QString)));
}

void KOEventEditor::setupRecurrenceTab()
{
  QFrame *topFrame = addPage(i18n("Recurrence"));

  QBoxLayout *topLayout = new QVBoxLayout(topFrame);  
  topLayout->setMargin(marginHint());

  mRecurrenceStack = new QWidgetStack(topFrame);
  topLayout->addWidget(mRecurrenceStack);

  mRecurrence = new KOEditorRecurrence(spacingHint(),mRecurrenceStack);
  mRecurrenceStack->addWidget(mRecurrence,0);
  
  mRecurrenceDisabled = new QLabel(
      i18n("This event does not recur.\nEnable Recurrence in General Tab."),
      mRecurrenceStack);
  mRecurrenceDisabled->setAlignment(AlignCenter);
  mRecurrenceStack->addWidget(mRecurrenceDisabled,1);
}

void KOEventEditor::enableRecurrence(bool enable)
{
  if (enable) mRecurrenceStack->raiseWidget(mRecurrence);
  else mRecurrenceStack->raiseWidget(mRecurrenceDisabled);
  
  mRecurrence->setEnabled(enable);
}

void KOEventEditor::editEvent(Event *event,QDate)
{
  mEvent = event;
  readEvent(mEvent);
}

void KOEventEditor::newEvent( QDateTime from, QDateTime to, bool allDay )
{
  mEvent = 0;
  setDefaults(from,to,allDay);

  enableButton(User1,false);
}

void KOEventEditor::slotDefault()
{
  int fmt = KOPrefs::instance()->mStartTime;
  
  QDateTime from(QDate::currentDate(), QTime(fmt,0,0));
  QDateTime to(QDate::currentDate(),
               QTime(fmt+KOPrefs::instance()->mDefaultDuration,0,0));

  setDefaults(from,to,false);
}

bool KOEventEditor::processInput()
{
  if (!validateInput()) return false;

  Event *event = 0;

  if (mEvent) event = mEvent;
  else {
    event = new Event;
    event->setOrganizer(KOPrefs::instance()->mEmail);
  }
  
  writeEvent(event);
  
  if (mEvent) {
    event->setRevision(event->revision()+1);
    emit eventChanged(event);
  } else {
    mCalendar->addEvent(event);
    mEvent = event;
    emit eventAdded(event);
  }
  
  return true;
}

void KOEventEditor::slotUser1()
{
  kdDebug() << "Delete event" << endl;
  if (mEvent) {
    emit eventToBeDeleted(mEvent);
    mCalendar->deleteEvent(mEvent);
    emit eventDeleted();
    reject();
  } else {
    reject();
  }
}

void KOEventEditor::setDefaults(QDateTime from, QDateTime to, bool allDay)
{
  mGeneral->setDefaults(from,to,allDay);
  mDetails->setDefaults();
  mRecurrence->setDefaults(from,to,allDay);

  enableRecurrence(false);
}

void KOEventEditor::readEvent(Event *event)
{
  mGeneral->readEvent(event);
  mDetails->readEvent(event);
  mRecurrence->readEvent(event);

  enableRecurrence(event->recurrence()->doesRecur());

  // categories
  mCategoryDialog->setSelected(event->categories());
}

void KOEventEditor::writeEvent(Event *event)
{
  mGeneral->writeEvent(event);
  mDetails->writeEvent(event);
  mRecurrence->writeEvent(event);
}

bool KOEventEditor::validateInput()
{
  if (!mGeneral->validateInput()) return false;
  if (!mDetails->validateInput()) return false;
  if (!mRecurrence->validateInput()) return false;
  return true;
}
