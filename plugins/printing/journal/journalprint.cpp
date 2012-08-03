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

#include <calendarsupport/utils.h>

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
  KCalCore::Journal::List journals( mCalendar->journals() );
  if ( mUseDateRange ) {
    KCalCore::Journal::List allJournals = journals;
    journals.clear();
    foreach( const KCalCore::Journal::Ptr &j, allJournals ) {
      const QDate dt = j->dtStart().date();
      if ( mFromDate <= dt && dt <= mToDate ) {
        journals.append( j );
      }
    }
  }

  QRect headerBox( 0, 0, width, headerHeight() );
  QRect footerBox( 0, height - footerHeight(), width, footerHeight() );
  height -= footerHeight();

  drawHeader( p, i18n( "Journal entries" ), QDate(), QDate(), headerBox );
  y = headerHeight() + 15;

  foreach( const KCalCore::Journal::Ptr &j, journals ) {
    drawJournal( j, p, x, y, width, height );
  }

  drawFooter( p, footerBox );
}
