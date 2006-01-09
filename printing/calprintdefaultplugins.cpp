/*
    This file is part of KOrganizer.

    Copyright (c) 1998 Preston Brown <pbrown@kde.org>
    Copyright (c) 2003 Reinhold Kainhofer <reinhold@kainhofer.com>
    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef KORG_NOPRINTER

#include <qpainter.h>
#include <q3datetimeedit.h>
#include <qdatetime.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <q3buttongroup.h>

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <kprinter.h>
#include <kconfig.h>
#include <kcalendarsystem.h>

#include <libkcal/todo.h>
#include <libkcal/calendar.h>

#include <libkdepim/kdateedit.h>

#include "calprinthelper.h"
#include "calprintpluginbase.h"
#include "calprintdefaultplugins.h"

#include "calprintdayconfig_base.h"
#include "calprintweekconfig_base.h"
#include "calprintmonthconfig_base.h"
#include "calprinttodoconfig_base.h"


/**************************************************************
 *           Print Day
 **************************************************************/

CalPrintDay::CalPrintDay() : CalPrintPluginBase()
{
}

CalPrintDay::~CalPrintDay()
{
}

QWidget *CalPrintDay::createConfigWidget( QWidget *w )
{
  return new CalPrintDayConfig_Base( w );
}

void CalPrintDay::readSettingsWidget()
{
  CalPrintDayConfig_Base *cfg =
      dynamic_cast<CalPrintDayConfig_Base*>( mConfigWidget );
  if ( cfg ) {
    mFromDate = cfg->mFromDate->date();
    mToDate = cfg->mToDate->date();

    mStartTime = cfg->mFromTime->time();
    mEndTime = cfg->mToTime->time();
    mIncludeAllEvents = cfg->mIncludeAllEvents->isChecked();

    mIncludeTodos = cfg->mIncludeTodos->isChecked();
    mUseColors = cfg->mColors->isChecked();
  }
}

void CalPrintDay::setSettingsWidget()
{
  CalPrintDayConfig_Base *cfg =
      dynamic_cast<CalPrintDayConfig_Base*>( mConfigWidget );
  if ( cfg ) {
    cfg->mFromDate->setDate( mFromDate );
    cfg->mToDate->setDate( mToDate );

    cfg->mFromTime->setTime( mStartTime );
    cfg->mToTime->setTime( mEndTime );
    cfg->mIncludeAllEvents->setChecked( mIncludeAllEvents );

    cfg->mIncludeTodos->setChecked( mIncludeTodos );
    cfg->mColors->setChecked( mUseColors );
  }
}

void CalPrintDay::loadConfig()
{
  if ( mConfig ) {
    QDate dt;
    QTime tm1( mCoreHelper->dayStart() );
    QDateTime startTm( dt, tm1 );
    QDateTime endTm( dt, tm1.addSecs( 12 * 60 * 60 ) );
    mStartTime = mConfig->readDateTimeEntry( "Start time", &startTm ).time();
    mEndTime = mConfig->readDateTimeEntry( "End time", &endTm ).time();
    mIncludeTodos = mConfig->readBoolEntry( "Include todos", false );
    mIncludeAllEvents = mConfig->readBoolEntry( "Include all events", false );
  }
  setSettingsWidget();
}

void CalPrintDay::saveConfig()
{
  kdDebug(5850) << "CalPrintDay::saveConfig()" << endl;

  readSettingsWidget();
  if ( mConfig ) {
    mConfig->writeEntry( "Start time", QDateTime( QDate(), mStartTime ) );
    mConfig->writeEntry( "End time", QDateTime( QDate(), mEndTime ) );
    mConfig->writeEntry( "Include todos", mIncludeTodos );
    mConfig->writeEntry( "Include all events", mIncludeAllEvents );
  }
}

void CalPrintDay::setDateRange( const QDate& from, const QDate& to )
{
  CalPrintPluginBase::setDateRange( from, to );
  CalPrintDayConfig_Base *cfg =
      dynamic_cast<CalPrintDayConfig_Base*>( mConfigWidget );
  if ( cfg ) {
    cfg->mFromDate->setDate( from );
    cfg->mToDate->setDate( to );
  }
}

void CalPrintDay::print( QPainter &p, int width, int height )
{
  QDate curDay( mFromDate );

  do {
    int x = 0;
    int y = 0;
    int currHeight=( height - y ) / 20;
    QTime curStartTime( mStartTime );
    QTime curEndTime( mEndTime );
    if ( curStartTime.secsTo( curEndTime ) <= 3600 ) {
      if ( curStartTime.hour() == 0 ) {
        curStartTime = QTime( 0, 0, 0 );
        curEndTime = curStartTime.addSecs( 3600 );
      } else if ( curEndTime.hour() == 23 ) {
        curEndTime=QTime( 23, 59, 59 );
        if ( curStartTime > QTime( 23, 0, 0 ) ) {
          curStartTime = QTime( 23, 0, 0 );
        }
      } else {
        curStartTime = curStartTime.addSecs( -1200 );
      }
      curEndTime = curEndTime.addSecs( 1200 );
    }

    KLocale *local = KGlobal::locale();
    mHelper->drawHeader( p, local->formatDate( curDay, false ),
                curDay, QDate(), 0, 0, width, mHelper->mHeaderHeight );

    y += mHelper->mHeaderHeight + 5;
    x = 80;
    Event::List eventList = mCalendar->events( curDay,
                                               EventSortStartDate,
                                               SortDirectionAscending );

    p.setFont( QFont( "helvetica", 12 ) );
    mHelper->drawAllDayBox( p, eventList, curDay, true, x, y, width - x, currHeight );
    y += currHeight;
    mHelper->drawAgendaDayBox( p, eventList, curDay, mIncludeAllEvents,
                      curStartTime, curEndTime, x, y, width - x, height - y );
    mHelper->drawTimeLine( p, curStartTime, curEndTime, 0, y, x - 5, height - y );
    curDay = curDay.addDays( 1 );
    if ( curDay <= mToDate ) mPrinter->newPage();
  } while ( curDay <= mToDate );
}



/**************************************************************
 *           Print Week
 **************************************************************/

CalPrintWeek::CalPrintWeek() : CalPrintPluginBase()
{
}

CalPrintWeek::~CalPrintWeek()
{
}

QWidget *CalPrintWeek::createConfigWidget( QWidget *w )
{
  return new CalPrintWeekConfig_Base( w );
}

void CalPrintWeek::readSettingsWidget()
{
  CalPrintWeekConfig_Base *cfg =
      dynamic_cast<CalPrintWeekConfig_Base*>( mConfigWidget );
  if ( cfg ) {
    mFromDate = cfg->mFromDate->date();
    mToDate = cfg->mToDate->date();

    mWeekPrintType = (eWeekPrintType)( cfg->mPrintType->id(
      cfg->mPrintType->selected() ) );

    mStartTime = cfg->mFromTime->time();
    mEndTime = cfg->mToTime->time();

    mIncludeTodos = cfg->mIncludeTodos->isChecked();
    mUseColors = cfg->mColors->isChecked();
  }
}

void CalPrintWeek::setSettingsWidget()
{
  CalPrintWeekConfig_Base *cfg =
      dynamic_cast<CalPrintWeekConfig_Base*>( mConfigWidget );
  if ( cfg ) {
    cfg->mFromDate->setDate( mFromDate );
    cfg->mToDate->setDate( mToDate );

    cfg->mPrintType->setButton( mWeekPrintType );

    cfg->mFromTime->setTime( mStartTime );
    cfg->mToTime->setTime( mEndTime );

    cfg->mIncludeTodos->setChecked( mIncludeTodos );
    cfg->mColors->setChecked( mUseColors );
  }
}

void CalPrintWeek::loadConfig()
{
  if ( mConfig ) {
    QDate dt;
    QTime tm1( mCoreHelper->dayStart() );
    QDateTime startTm( dt, tm1  );
    QDateTime endTm( dt, tm1.addSecs( 43200 ) );
    mStartTime = mConfig->readDateTimeEntry( "Start time", &startTm ).time();
    mEndTime = mConfig->readDateTimeEntry( "End time", &endTm ).time();
    mIncludeTodos = mConfig->readBoolEntry( "Include todos", false );
    mWeekPrintType =(eWeekPrintType)( mConfig->readNumEntry( "Print type", (int)Filofax ) );
  }
  setSettingsWidget();
}

void CalPrintWeek::saveConfig()
{
  readSettingsWidget();
  if ( mConfig ) {
    mConfig->writeEntry( "Start time", QDateTime( QDate(), mStartTime ) );
    mConfig->writeEntry( "End time", QDateTime( QDate(), mEndTime ) );
    mConfig->writeEntry( "Include todos", mIncludeTodos );
    mConfig->writeEntry( "Print type", int( mWeekPrintType ) );
  }
}

KPrinter::Orientation CalPrintWeek::orientation()
{
  if ( mWeekPrintType == Filofax ) return KPrinter::Portrait;
  else if ( mWeekPrintType == SplitWeek ) return KPrinter::Portrait;
  else return KPrinter::Landscape;
}

void CalPrintWeek::setDateRange( const QDate &from, const QDate &to )
{
  CalPrintPluginBase::setDateRange( from, to );
  CalPrintWeekConfig_Base *cfg =
      dynamic_cast<CalPrintWeekConfig_Base*>( mConfigWidget );
  if ( cfg ) {
    cfg->mFromDate->setDate( from );
    cfg->mToDate->setDate( to );
  }
}

void CalPrintWeek::print( QPainter &p, int width, int height )
{
  QDate curWeek, fromWeek, toWeek;

  // correct begin and end to first and last day of week
  int weekdayCol = mHelper->weekdayColumn( mFromDate.dayOfWeek() );
  fromWeek = mFromDate.addDays( -weekdayCol );
  weekdayCol = mHelper->weekdayColumn( mFromDate.dayOfWeek() );
  toWeek = mToDate.addDays( 6 - weekdayCol );

  curWeek = fromWeek.addDays( 6 );
  KLocale *local = KGlobal::locale();

  switch ( mWeekPrintType ) {
    case Filofax:
      do {
        QString line1( local->formatDate( curWeek.addDays( -6 ) ) );
        QString line2( local->formatDate( curWeek ) );
        mHelper->drawHeader( p, line1 + "\n" + line2, curWeek.addDays( -6 ), QDate(),
                    0, 0, width, mHelper->mHeaderHeight );
        int top = mHelper->mHeaderHeight + 10;
        mHelper->drawWeek( p, curWeek, 0, top, width, height - top );
        curWeek = curWeek.addDays( 7 );
        if ( curWeek <= toWeek )
          mPrinter->newPage();
      } while ( curWeek <= toWeek );
      break;

    case Timetable:
    default:
      do {
        QString line1( local->formatDate( curWeek.addDays( -6 ) ) );
        QString line2( local->formatDate( curWeek ) );
        int hh = int(mHelper->mHeaderHeight * 2./3.);
        mHelper->drawHeader( p, i18n("date from - to", "%1 - %2\nWeek %3")
                             .arg( line1 )
                             .arg( line2 )
                             .arg( curWeek.weekNumber() ),
                             curWeek, QDate(), 0, 0, width, hh );
        mHelper->drawTimeTable( p, fromWeek, curWeek,
                       mStartTime, mEndTime, 0, hh + 5,
                       width, height - hh - 5 );
        fromWeek = fromWeek.addDays( 7 );
        curWeek = fromWeek.addDays( 6 );
        if ( curWeek <= toWeek )
          mPrinter->newPage();
      } while ( curWeek <= toWeek );
      break;

    case SplitWeek:
      do {
        QString line1( local->formatDate( curWeek.addDays( -6 ) ) );
        QString line2( local->formatDate( curWeek ) );
        QDate endLeft( fromWeek.addDays( 3 ) );
        int hh = mHelper->mHeaderHeight;

        mHelper->drawTimeTable( p, fromWeek, endLeft,
                       mStartTime, mEndTime, 0, hh + 5,
                       width, height - hh - 5 );
        mPrinter->newPage();
        mHelper->drawSplitHeaderRight( p, fromWeek, curWeek, QDate(), width, hh );
        mHelper->drawTimeTable( p, endLeft.addDays( 1 ), curWeek,
                       mStartTime, mEndTime, 0, hh + 5,
                       int( ( width - 50 ) * 3. / 4. + 50 ), height - hh - 5 );

        fromWeek = fromWeek.addDays( 7 );
        curWeek = fromWeek.addDays( 6 );
        if ( curWeek <= toWeek )
          mPrinter->newPage();
      } while ( curWeek <= toWeek );
      break;
  }
}




/**************************************************************
 *           Print Month
 **************************************************************/

CalPrintMonth::CalPrintMonth() : CalPrintPluginBase()
{
}

CalPrintMonth::~CalPrintMonth()
{
}

QWidget *CalPrintMonth::createConfigWidget( QWidget *w )
{
  return new CalPrintMonthConfig_Base( w );
}

void CalPrintMonth::readSettingsWidget()
{
  CalPrintMonthConfig_Base *cfg =
      dynamic_cast<CalPrintMonthConfig_Base *>( mConfigWidget );
  if ( cfg ) {
    mFromDate = cfg->mFromDate->date();
    mToDate = cfg->mToDate->date();

    mWeekNumbers =  cfg->mWeekNumbers->isChecked();
    mRecurDaily = cfg->mRecurDaily->isChecked();
    mRecurWeekly = cfg->mRecurWeekly->isChecked();
    mIncludeTodos = cfg->mIncludeTodos->isChecked();
//    mUseColors = cfg->mColors->isChecked();
  }
}

void CalPrintMonth::setSettingsWidget()
{
  CalPrintMonthConfig_Base *cfg =
      dynamic_cast<CalPrintMonthConfig_Base *>( mConfigWidget );
  if ( cfg ) {
    cfg->mFromDate->setDate( mFromDate );
    cfg->mToDate->setDate( mToDate );

    cfg->mWeekNumbers->setChecked( mWeekNumbers );
    cfg->mRecurDaily->setChecked( mRecurDaily );
    cfg->mRecurWeekly->setChecked( mRecurWeekly );
    cfg->mIncludeTodos->setChecked( mIncludeTodos );
//    cfg->mColors->setChecked( mUseColors );
  }
}

void CalPrintMonth::loadConfig()
{
  if ( mConfig ) {
    mWeekNumbers = mConfig->readBoolEntry( "Print week numbers", true );
    mRecurDaily = mConfig->readBoolEntry( "Print daily incidences", true );
    mRecurWeekly = mConfig->readBoolEntry( "Print weekly incidences", true );
    mIncludeTodos = mConfig->readBoolEntry( "Include todos", false );
  }
  setSettingsWidget();
}

void CalPrintMonth::saveConfig()
{
  readSettingsWidget();
  if ( mConfig ) {
    mConfig->writeEntry( "Print week numbers", mWeekNumbers );
    mConfig->writeEntry( "Print daily incidences", mRecurDaily );
    mConfig->writeEntry( "Print weekly incidences", mRecurWeekly );
    mConfig->writeEntry( "Include todos", mIncludeTodos );
  }
}

void CalPrintMonth::setDateRange( const QDate &from, const QDate &to )
{
  CalPrintPluginBase::setDateRange( from, to );
  CalPrintMonthConfig_Base *cfg =
      dynamic_cast<CalPrintMonthConfig_Base *>( mConfigWidget );
  if ( cfg ) {
    cfg->mFromDate->setDate( from );
    cfg->mToDate->setDate( to );
  }
}

void CalPrintMonth::print( QPainter &p, int width, int height )
{
  QDate curMonth, fromMonth, toMonth;

  fromMonth = mFromDate.addDays( -( mFromDate.day() - 1 ) );
  toMonth = mToDate.addDays( mToDate.daysInMonth() - mToDate.day() );

  curMonth = fromMonth;
  const KCalendarSystem *calSys = mHelper->calendarSystem();
  do {
    QString title( i18n("monthname year", "%1 %2") );
    title = title.arg( calSys->monthName( curMonth ) )
                 .arg( curMonth.year() );
    QDate tmp( fromMonth );
    int weekdayCol = mHelper->weekdayColumn( tmp.dayOfWeek() );
    tmp = tmp.addDays( -weekdayCol );

    mHelper->drawHeader( p, title,
                curMonth.addMonths( -1 ), curMonth.addMonths( 1 ),
                0, 0, width, mHelper->mHeaderHeight );
    mHelper->drawMonth( p, curMonth, mWeekNumbers, mRecurDaily, mRecurWeekly, 0, mHelper->mHeaderHeight + 5,
               width, height - mHelper->mHeaderHeight - 5 );
    curMonth = curMonth.addDays( curMonth.daysInMonth() );
    if ( curMonth <= toMonth ) mPrinter->newPage();
  } while ( curMonth <= toMonth );

}




/**************************************************************
 *           Print Todos
 **************************************************************/

CalPrintTodos::CalPrintTodos() : CalPrintPluginBase()
{
}

CalPrintTodos::~CalPrintTodos()
{
}

QWidget *CalPrintTodos::createConfigWidget( QWidget *w )
{
  return new CalPrintTodoConfig_Base( w );
}

void CalPrintTodos::readSettingsWidget()
{
  CalPrintTodoConfig_Base *cfg =
      dynamic_cast<CalPrintTodoConfig_Base *>( mConfigWidget );
  if ( cfg ) {
    mPageTitle = cfg->mTitle->text();

    mTodoPrintType = (eTodoPrintType)( cfg->mPrintType->id(
      cfg->mPrintType->selected() ) );

    mFromDate = cfg->mFromDate->date();
    mToDate = cfg->mToDate->date();

    mIncludeDescription = cfg->mDescription->isChecked();
    mIncludePriority = cfg->mPriority->isChecked();
    mIncludeDueDate = cfg->mDueDate->isChecked();
    mIncludePercentComplete = cfg->mPercentComplete->isChecked();
    mConnectSubTodos = cfg->mConnectSubTodos->isChecked();
    mStrikeOutCompleted = cfg->mStrikeOutCompleted->isChecked();

    mTodoSortField = (eTodoSortField)cfg->mSortField->currentItem();
    mTodoSortDirection = (eTodoSortDirection)cfg->mSortDirection->currentItem();
  }
}

void CalPrintTodos::setSettingsWidget()
{
  CalPrintTodoConfig_Base *cfg =
      dynamic_cast<CalPrintTodoConfig_Base *>( mConfigWidget );
  if ( cfg ) {
    cfg->mTitle->setText( mPageTitle );

    cfg->mPrintType->setButton( mTodoPrintType );

    cfg->mFromDate->setDate( mFromDate );
    cfg->mToDate->setDate( mToDate );

    cfg->mDescription->setChecked( mIncludeDescription );
    cfg->mPriority->setChecked( mIncludePriority );
    cfg->mDueDate->setChecked( mIncludeDueDate );
    cfg->mPercentComplete->setChecked( mIncludePercentComplete );
    cfg->mConnectSubTodos->setChecked( mConnectSubTodos );
    cfg->mStrikeOutCompleted->setChecked( mStrikeOutCompleted );

    cfg->mSortField->insertItem( i18n("Summary") );
    cfg->mSortField->insertItem( i18n("Start Date") );
    cfg->mSortField->insertItem( i18n("Due Date") );
    cfg->mSortField->insertItem( i18n("Priority") );
    cfg->mSortField->insertItem( i18n("Percent Complete") );
    cfg->mSortField->setCurrentItem( mTodoSortField );

    cfg->mSortDirection->insertItem( i18n( "Ascending" ) );
    cfg->mSortDirection->insertItem( i18n( "Descending" ) );
    cfg->mSortDirection->setCurrentItem( mTodoSortDirection );
  }
}

void CalPrintTodos::loadConfig()
{
  if ( mConfig ) {
    mPageTitle = mConfig->readEntry( "Page title", i18n("To-do list") );
    mTodoPrintType = (eTodoPrintType)mConfig->readNumEntry( "Print type", (int)TodosAll );
    mIncludeDescription = mConfig->readBoolEntry( "Include description", true );
    mIncludePriority = mConfig->readBoolEntry( "Include priority", true );
    mIncludeDueDate = mConfig->readBoolEntry( "Include due date", true );
    mIncludePercentComplete = mConfig->readBoolEntry( "Include percentage completed", true );
    mConnectSubTodos = mConfig->readBoolEntry( "Connect subtodos", true );
    mStrikeOutCompleted = mConfig->readBoolEntry( "Strike out completed summaries",  true );
    mTodoSortField = (eTodoSortField)mConfig->readNumEntry( "Sort field", (int)TodoFieldSummary );
    mTodoSortDirection = (eTodoSortDirection)mConfig->readNumEntry( "Sort direction", (int)TodoDirectionAscending );
  }
  setSettingsWidget();
}

void CalPrintTodos::saveConfig()
{
  readSettingsWidget();
  if ( mConfig ) {
    mConfig->writeEntry( "Page title", mPageTitle );
    mConfig->writeEntry( "Print type", int( mTodoPrintType ) );
    mConfig->writeEntry( "Include description", mIncludeDescription );
    mConfig->writeEntry( "Include priority", mIncludePriority );
    mConfig->writeEntry( "Include due date", mIncludeDueDate );
    mConfig->writeEntry( "Include percentage completed", mIncludePercentComplete );
    mConfig->writeEntry( "Connect subtodos", mConnectSubTodos );
    mConfig->writeEntry( "Strike out completed summaries", mStrikeOutCompleted );
    mConfig->writeEntry( "Sort field", (int)mTodoSortField );
    mConfig->writeEntry( "Sort direction", (int)mTodoSortDirection );
  }
}

void CalPrintTodos::print( QPainter &p, int width, int height )
{
  int pospriority = 10;
  int possummary = 60;
  int posdue = width - 65;
  int poscomplete = posdue - 70; //Complete column is to right of the Due column
  int lineSpacing = 15;
  int fontHeight = 10;

  // Draw the First Page Header
  mHelper->drawHeader( p, mPageTitle, mFromDate, QDate(),
                       0, 0, width, mHelper->mHeaderHeight );

  // Draw the Column Headers
  int mCurrentLinePos = mHelper->mHeaderHeight + 5;
  QString outStr;
  QFont oldFont( p.font() );

  p.setFont( QFont( "helvetica", 10, QFont::Bold ) );
  lineSpacing = p.fontMetrics().lineSpacing();
  mCurrentLinePos += lineSpacing;
  if ( mIncludePriority ) {
    outStr += i18n( "Priority" );
    p.drawText( pospriority, mCurrentLinePos - 2, outStr );
  } else {
    possummary = 10;
    pospriority = -1;
  }

  outStr.truncate( 0 );
  outStr += i18n( "Summary" );
  p.drawText( possummary, mCurrentLinePos - 2, outStr );

  if ( mIncludePercentComplete ) {
    if ( !mIncludeDueDate ) //move Complete column to the right
      poscomplete = posdue; //if not print the Due Date column
    outStr.truncate( 0 );
    outStr += i18n( "Complete" );
    p.drawText( poscomplete, mCurrentLinePos - 2, outStr );
  } else {
    poscomplete = -1;
  }

  if ( mIncludeDueDate ) {
    outStr.truncate( 0 );
    outStr += i18n( "Due" );
    p.drawText( posdue, mCurrentLinePos - 2, outStr );
  } else {
    posdue = -1;
  }

  p.setFont( QFont( "helvetica", 10 ) );
  fontHeight = p.fontMetrics().height();

  Todo::List todoList;
  Todo::List tempList;
  Todo::List::ConstIterator it;

  // Convert sort options to the corresponding enums
  TodoSortField sortField;
  switch( mTodoSortField ) {
  case TodoFieldSummary:
    sortField = TodoSortSummary; break;
  case TodoFieldStartDate:
    sortField = TodoSortStartDate; break;
  case TodoFieldDueDate:
    sortField = TodoSortDueDate; break;
  case TodoFieldPriority:
    sortField = TodoSortPriority; break;
  case TodoFieldPercentComplete:
    sortField = TodoSortPercentComplete; break;
  }

  SortDirection sortDirection;
  switch( mTodoSortDirection ) {
  case TodoDirectionAscending:
    sortDirection = SortDirectionAscending; break;
  case TodoDirectionDescending:
    sortDirection = SortDirectionDescending; break;
  }

  // Create list of to-dos which will be printed
  // Also sort toplevel by summaries.
  todoList = mCalendar->todos( TodoSortSummary,  sortDirection );
  switch( mTodoPrintType ) {
  case TodosAll:
    break;
  case TodosUnfinished:
    for( it = todoList.begin(); it!= todoList.end(); ++it ) {
      if ( !(*it)->isCompleted() )
        tempList.append( *it );
    }
    todoList = tempList;
    break;
  case TodosDueRange:
    for( it = todoList.begin(); it!= todoList.end(); ++it ) {
      if ( (*it)->hasDueDate() ) {
        if ( (*it)->dtDue().date() >= mFromDate &&
             (*it)->dtDue().date() <= mToDate )
          tempList.append( *it );
      } else {
        tempList.append( *it );
      }
    }
    todoList = tempList;
    break;
  }

  // Print to-dos
  int count = 0;
  for ( it=todoList.begin(); it!=todoList.end(); ++it ) {
    Todo *currEvent = *it;

    // Skip sub-to-dos. They will be printed recursively in drawTodo()
    if ( !currEvent->relatedTo() ) {
      count++;
      mHelper->drawTodo( count, currEvent, p,
                         sortField, sortDirection,
                         mConnectSubTodos,
                         mStrikeOutCompleted, mIncludeDescription,
                         pospriority, possummary, posdue, poscomplete,
                         0, 0, mCurrentLinePos, width, height, todoList );
    }
  }
  p.setFont( oldFont );
}


#endif
