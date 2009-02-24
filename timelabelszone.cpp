/*
  This file is part of KOrganizer.

  Copyright (c) 2007 Bruno Virlet <bruno@virlet.org>

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

#include "timelabelszone.h"
#include "koprefs.h"
#include "views/agendaview/koagenda.h"
#include "views/agendaview/koagendaview.h"

#include <ksystemtimezone.h>

#include <QHBoxLayout>

using namespace KOrg;

TimeLabelsZone::TimeLabelsZone( QWidget *parent, KOAgenda *agenda )
  : QWidget( parent ), mAgenda( agenda ), mParent( dynamic_cast<KOAgendaView*>( parent ) )
{
  mTimeLabelsLayout = new QHBoxLayout( this );
  mTimeLabelsLayout->setMargin( 0 );
  mTimeLabelsLayout->setSpacing( 0 );

  init();
}

void TimeLabelsZone::reset()
{
  foreach ( TimeLabels *label, mTimeLabelsList ) {
    label->hide();
    label->deleteLater();
  }
  mTimeLabelsList.clear();

  init();

  // Update some related geometry from the agenda view
  updateAll();
  if ( mParent ) {
    mParent->createDayLabels();
    mParent->updateTimeBarWidth();
  }
}

void TimeLabelsZone::init()
{
  addTimeLabels( KOPrefs::instance()->timeSpec() );

  foreach ( const QString &zoneStr, KOPrefs::instance()->timeScaleTimezones() ) {
    KTimeZone zone = KSystemTimeZones::zone( zoneStr );
    if ( zone.isValid() ) {
      addTimeLabels( zone );
    }
  }
}

void TimeLabelsZone::addTimeLabels( const KDateTime::Spec &spec )
{
  TimeLabels *labels = new TimeLabels( spec, 24, this );
  mTimeLabelsList.prepend( labels );
  mTimeLabelsLayout->insertWidget( 0, labels );
  setupTimeLabel( labels );
}

void TimeLabelsZone::setupTimeLabel( TimeLabels *timeLabel )
{
  if ( mAgenda ) {
    timeLabel->setAgenda( mAgenda );
    connect( mAgenda->verticalScrollBar(), SIGNAL(valueChanged(int)),
            timeLabel, SLOT(positionChanged()) );
  }
  if ( mParent ) {
    connect( timeLabel->verticalScrollBar(), SIGNAL(valueChanged(int)),
             mParent, SLOT(setContentsPos(int)) );
  }
}

int TimeLabelsZone::timeLabelsWidth()
{
  if ( mTimeLabelsList.isEmpty() ) {
    return 0;
  } else {
    return mTimeLabelsList.first()->width() * mTimeLabelsList.count();
  }
}

void TimeLabelsZone::updateAll()
{
  foreach ( TimeLabels *timeLabel, mTimeLabelsList ) {
    timeLabel->updateConfig();
    timeLabel->positionChanged();
    timeLabel->repaint();
  }
}

void TimeLabelsZone::setTimeLabelsWidth( int width )
{
  foreach ( TimeLabels *timeLabel, mTimeLabelsList ) {
    timeLabel->setFixedWidth( width / mTimeLabelsList.count() );
  }
}

TimeLabels::List TimeLabelsZone::timeLabels() const
{
  return mTimeLabelsList;
}

void TimeLabelsZone::setAgendaView( KOAgendaView *agenda )
{
  mAgenda = agenda->agenda();
  mParent = agenda;
  foreach ( TimeLabels *timeLabel, mTimeLabelsList ) {
    setupTimeLabel( timeLabel );
  }
}

#include "timelabelszone.moc"
