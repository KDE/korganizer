/*
    This file is part of KOrganizer.
    Copyright (c) 2000, 2001 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef HTMLEXPORT_H
#define HTMLEXPORT_H
// $Id$

#include <qstring.h>
#include <qdatetime.h>

#include <libkcal/calendar.h>

class QFile;
class QTextStream;

using namespace KCal;

/**
  This class provides the functions to export a calendar as a HTML page.
*/
class HtmlExport {
  public:
    /** Create new HTML exporter for calendar */
    HtmlExport(Calendar *calendar) :
        mCalendar(calendar),
        mMonthViewEnabled(true),mEventsEnabled(false),mTodosEnabled(true),
        mCategoriesTodoEnabled(false),mAttendeesTodoEnabled(false),
        mCategoriesEventEnabled(false),mAttendeesEventEnabled(false),
        mDueDateEnabled(false),
        mExcludePrivateEventEnabled(false),
        mExcludeConfidentialEventEnabled(false),
        mExcludePrivateTodoEnabled(false),
        mExcludeConfidentialTodoEnabled(false) {}
    virtual ~HtmlExport() {};

    /**
      writes out the calendar in HTML format.
    */
    bool save(const QString &fileName);

    /**
      writes out calendar to file. The QFile has to already be opened for writing.
    */
    bool save(QTextStream *);

    void setMonthViewEnabled(bool enable=true) { mMonthViewEnabled = enable; }
    bool monthViewEnabled() { return mMonthViewEnabled; }

    void setEventsEnabled(bool enable=true) { mEventsEnabled = enable; }
    bool eventsEnabled() { return mEventsEnabled; }

    void setTodosEnabled(bool enable=true) { mTodosEnabled = enable; }
    bool todosEnabled() { return mTodosEnabled; }
  
    void setCategoriesTodoEnabled(bool enable=true) { mCategoriesTodoEnabled = enable; }
    bool categoriesTodoEnabled() { return mCategoriesTodoEnabled; }
  
    void setAttendeesTodoEnabled(bool enable=true) { mAttendeesTodoEnabled = enable; }
    bool attendeesTodoEnabled() { return mAttendeesTodoEnabled; }

    void setExcludePrivateTodoEnabled(bool enable=true) { mExcludePrivateTodoEnabled = enable; }
    bool excludePrivateTodoEnabled() { return mExcludePrivateTodoEnabled; }

    void setExcludeConfidentialTodoEnabled(bool enable=true) { mExcludeConfidentialTodoEnabled = enable; }
    bool excludeConfidentialTodoEnabled() { return mExcludeConfidentialTodoEnabled; }
  
    void setCategoriesEventEnabled(bool enable=true) { mCategoriesEventEnabled = enable; }
    bool categoriesEventEnabled() { return mCategoriesEventEnabled; }
  
    void setAttendeesEventEnabled(bool enable=true) { mAttendeesEventEnabled = enable; }
    bool attendeesEventEnabled() { return mAttendeesEventEnabled; }

    void setExcludePrivateEventEnabled(bool enable=true) { mExcludePrivateEventEnabled = enable; }
    bool excludePrivateEventEnabled() { return mExcludePrivateEventEnabled; }

    void setExcludeConfidentialEventEnabled(bool enable=true) { mExcludeConfidentialEventEnabled = enable; }
    bool excludeConfidentialEventEnabled() { return mExcludeConfidentialEventEnabled; }

    void setDueDateEnabled(bool enable=true) { mDueDateEnabled = enable; }
    bool dueDateEnabled() { return mDueDateEnabled; }
  
    void setDateRange(const QDate &from,const QDate &to) { mFromDate = from, mToDate = to; }
    QDate fromDate() { return mFromDate; }
    QDate toDate() { return mToDate; }
  
  protected:

    void createHtmlMonthView (QTextStream *ts);  
    void createHtmlEventList (QTextStream *ts);
    void createHtmlTodoList (QTextStream *ts);

    void createHtmlTodo (QTextStream *ts,Todo *todo);
    void createHtmlEvent (QTextStream *ts,Event *event,QDate date, bool withDescription = true);

    bool checkSecrecyTodo (Todo *todo);
    bool checkSecrecyEvent (Event *event);

    void formatHtmlCategories (QTextStream *ts,Incidence *event);
    void formatHtmlAttendees (QTextStream *ts,Incidence *event);

    QString breakString(const QString &text);

  private:
    Calendar *mCalendar;

    bool mMonthViewEnabled;
    bool mEventsEnabled;
    bool mTodosEnabled;
    bool mCategoriesTodoEnabled;
    bool mAttendeesTodoEnabled;
    bool mExcludePrivateTodoEnabled;
    bool mExcludeConfidentialTodoEnabled;
    bool mCategoriesEventEnabled;
    bool mAttendeesEventEnabled;
    bool mExcludePrivateEventEnabled;
    bool mExcludeConfidentialEventEnabled;
    bool mDueDateEnabled;
    
    QDate mFromDate;
    QDate mToDate;
};

#endif
