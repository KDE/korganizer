/*
  This file is part of the KOrganizer interfaces.

  Copyright (c) 1999,2001,2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2004           Reinhold Kainhofer   <reinhold@kainhofer.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "baseview.h"

namespace KOrg {

BaseView::BaseView( Calendar *cal, QWidget *parent )
  : QWidget( parent ), mCalendar( cal ), mChanger( 0 )
{}

BaseView::~BaseView()
{}

void BaseView::setCalendar( Calendar *cal )
{
  mCalendar = cal;
}

CalPrinterBase::PrintType BaseView::printType()
{
  return CalPrinterBase::Month;
}

Calendar *BaseView::calendar()
{
  return mCalendar;
}

bool BaseView::isEventView()
{
  return false;
}

void BaseView::dayPassed( const QDate & )
{
  updateView();
}

void BaseView::setIncidenceChanger( IncidenceChangerBase *changer )
{
  mChanger = changer;
}

void BaseView::flushView()
{}

void BaseView::updateConfig()
{}

void BaseView::clearSelection()
{}

bool BaseView::eventDurationHint( QDateTime &startDt, QDateTime &endDt, bool &allDay )
{
  Q_UNUSED( startDt );
  Q_UNUSED( endDt );
  Q_UNUSED( allDay );
  return false;
}

void BaseView::getHighlightMode( bool &highlightEvents,
                                 bool &highlightTodos,
                                 bool &highlightJournals ) {
  highlightEvents   = true;
  highlightTodos    = false;
  highlightJournals = false;
}

bool BaseView::usesFullWindow()
{
  return false;
}

} //namespace KOrg

#include "baseview.moc"
