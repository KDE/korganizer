/*
  This file is part of KOrganizer.

  Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "journalprint.h"
#include "calprintpluginbase.h"

#include <libkdepim/kdateedit.h>

#include <kcal/journal.h>
#include <kcal/calendar.h>

#include <kconfig.h>
#include <kdebug.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>

#include <QButtonGroup>

class JournalPrintFactory : public KOrg::PrintPluginFactory
{
  public:
    KOrg::PrintPlugin *createPluginFactory() { return new CalPrintJournal; }
};

K_EXPORT_PLUGIN( JournalPrintFactory )

/**************************************************************
 *           Print Journal
 **************************************************************/

QWidget *CalPrintJournal::createConfigWidget( QWidget *w )
{
  return new CalPrintJournalConfig( w );
}

void CalPrintJournal::readSettingsWidget()
{
  CalPrintJournalConfig *cfg =
      dynamic_cast<CalPrintJournalConfig*>( ( QWidget* )mConfigWidget );
  if ( cfg ) {
    mFromDate = cfg->mFromDate->date();
    mToDate = cfg->mToDate->date();
    mUseDateRange = cfg->mRangeJournals->isChecked();
  }
}

void CalPrintJournal::setSettingsWidget()
{
  CalPrintJournalConfig *cfg =
      dynamic_cast<CalPrintJournalConfig*>( ( QWidget* )mConfigWidget );
  if ( cfg ) {
    cfg->mFromDate->setDate( mFromDate );
    cfg->mToDate->setDate( mToDate );

    if ( mUseDateRange ) {
      cfg->mRangeJournals->setChecked( true );
    } else {
      cfg->mAllJournals->setChecked( true );
    }
  }
}

void CalPrintJournal::loadConfig()
{
  if ( mConfig ) {
    KConfigGroup config( mConfig, "Journalprint" );
    mUseDateRange = config.readEntry( "JournalsInRange", false );
  }
  setSettingsWidget();
}

void CalPrintJournal::saveConfig()
{
  kDebug();

  readSettingsWidget();
  if ( mConfig ) {
    KConfigGroup config( mConfig, "Journalprint" );
    config.writeEntry( "JournalsInRange", mUseDateRange );
  }
}

void CalPrintJournal::setDateRange( const QDate &from, const QDate &to )
{
  CalPrintPluginBase::setDateRange( from, to );
  CalPrintJournalConfig *cfg =
      dynamic_cast<CalPrintJournalConfig*>( ( QWidget* )mConfigWidget );
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

  drawHeader( p, i18n( "Journal entries" ), QDate(), QDate(),
              QRect( 0, 0, width, headerHeight() ) );
  y = headerHeight() + 15;

  Journal::List::Iterator it = journals.begin();
  for ( ; it != journals.end(); ++it ) {
    drawJournal( *it, p, x, y, width, height );
  }
}
