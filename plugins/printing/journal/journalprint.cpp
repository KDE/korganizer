/*
    This file is part of KOrganizer.

    Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef KORG_NOPRINTER

#include "journalprint.h"

#include "calprintpluginbase.h"
#include "calprinthelper.h"
#include <libkcal/journal.h>
#include <libkcal/calendar.h>
#include <libkdepim/kdateedit.h>
#include <kconfig.h> 
#include <kdebug.h>

#include <qbuttongroup.h>

#include "calprintjournalconfig_base.h"


class JournalPrintFactory : public KOrg::PrintPluginFactory {
  public:
    KOrg::PrintPlugin *create() { return new CalPrintJournal; }
};

extern "C" {
  void *init_libkorg_journalprint()
  {
    return (new JournalPrintFactory);
  }
}


/**************************************************************
 *           Print Journal
 **************************************************************/

QWidget *CalPrintJournal::createConfigWidget( QWidget *w )
{
  return new CalPrintJournalConfig_Base( w );
}

void CalPrintJournal::readSettingsWidget()
{
  CalPrintJournalConfig_Base *cfg =
      dynamic_cast<CalPrintJournalConfig_Base*>( mConfigWidget );
  if ( cfg ) {
    mFromDate = cfg->mFromDate->date();
    mToDate = cfg->mToDate->date();
    mUseDateRange = (cfg->mDateRangeGroup->selectedId() == 1);
  }
}

void CalPrintJournal::setSettingsWidget()
{
  CalPrintJournalConfig_Base *cfg =
      dynamic_cast<CalPrintJournalConfig_Base*>( mConfigWidget );
  if ( cfg ) {
    cfg->mFromDate->setDate( mFromDate );
    cfg->mToDate->setDate( mToDate );
    
    cfg->mDateRangeGroup->setButton( (mUseDateRange)?1:0 );
  }
}

void CalPrintJournal::loadConfig()
{
  if ( mConfig ) {
    mUseDateRange = mConfig->readBoolEntry( "JournalsInRange", false );
  }
  setSettingsWidget();
}

void CalPrintJournal::saveConfig()
{
  kdDebug() << "CalPrintJournal::saveConfig()" << endl;

  readSettingsWidget();
  if ( mConfig ) {
    mConfig->writeEntry( "JournalsInRange", mUseDateRange );
  }
}

void CalPrintJournal::setDateRange( const QDate& from, const QDate& to )
{
  CalPrintPluginBase::setDateRange( from, to );
  CalPrintJournalConfig_Base *cfg =
      dynamic_cast<CalPrintJournalConfig_Base*>( mConfigWidget );
  if ( cfg ) {
    cfg->mFromDate->setDate( from );
    cfg->mToDate->setDate( to );
  }
}

void CalPrintJournal::print( QPainter &p, int width, int height )
{
  int x=0, y=0;
  Journal::List journals( mCalendar->journals() );
  if ( mUseDateRange ) {
    Journal::List allJournals = journals;
    journals.clear();
    Journal::List::Iterator it = allJournals.begin();
    for ( ; it != allJournals.end(); ++it ) {
      QDate dt = (*it)->dtStart().date();
      if ( mFromDate <= dt && dt <= mToDate ) {
        journals.append( *it );
      }
    }
  }
  
  mHelper->drawHeader( p, i18n("Journal entries"), QDate(), QDate(), 0, 0, width, mHelper->mHeaderHeight/2 );
  y = mHelper->mHeaderHeight/2 + 15;

  // @TODO: Sort the list of journals
  Journal::List::Iterator it = journals.begin();
  for ( ; it != journals.end(); ++it ) {
    mHelper->drawJournal( *it, p, x, y, width, height );
  }
}

#endif
