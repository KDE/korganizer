/*
    $Id$

     Requires the Qt and KDE widget libraries, available at no cost at
     http://www.troll.no and http://www.kde.org respectively

     Copyright (C) 1997, 1998 Preston Brown
     preston.brown@yale.edu

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

     -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-

     This file implements a class for displaying a dialog box for
     adding or editing appointments/events.
*/

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

#include "catdlg.h"
#include "koprefs.h"
#include "categoryselectdialog.h"

#include "koeventeditor.h"
#include "koeventeditor.moc"

KOEventEditor::KOEventEditor(CalObject *calendar) :
  KDialogBase(Tabbed,i18n("Edit Event"),Ok|Apply|Cancel|Default|User1,Ok,0,0,
              false,false,i18n("Delete"))
{
  mCalendar = calendar;
  mEvent = 0;

//  mCategoryDialog = new CategoryDialog();
  mCategoryDialog = new CategorySelectDialog();

  setupGeneralTab();
  setupDetailsTab();
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
  connect(mCategoryDialog,SIGNAL(editCategories()),SIGNAL(editCategories()));
//  connect(mCategoryDialog,SIGNAL(categoryConfigChanged()),
//          SIGNAL(categoryConfigChanged()));

  // Clicking cancel exits the dialog without saving
  connect(this,SIGNAL(cancelClicked()),SLOT(reject()));
}

KOEventEditor::~KOEventEditor()
{
  delete mCategoryDialog;
}

void KOEventEditor::setupGeneralTab()
{
  QFrame *topFrame = addPage(i18n("General"));

  QBoxLayout *topLayout = new QVBoxLayout(topFrame);  
  topLayout->setMargin(marginHint());

  mGeneral = new KOEditorGeneralEvent(spacingHint(),topFrame);
  topLayout->addWidget(mGeneral);
}

void KOEventEditor::setupDetailsTab()
{
  QFrame *topFrame = addPage(i18n("Attendees"));

  QBoxLayout *topLayout = new QVBoxLayout(topFrame);  
  topLayout->setMargin(marginHint());

  mDetails = new KOEditorDetails(spacingHint(),topFrame);
  topLayout->addWidget(mDetails);
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
  else event = new Event;
  
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

void KOEventEditor::slotApply()
{
  processInput();
}

void KOEventEditor::slotOk()
{
  if (processInput()) accept();
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

void KOEventEditor::updateCategoryConfig()
{
  mCategoryDialog->updateCategoryConfig();
}
