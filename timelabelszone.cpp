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

#include "timelabels.h"
#include "koagendaview.h"
#include "koglobals.h"
#include "koprefs.h"
#include "koagenda.h"
#include "koagendaitem.h"
#include "kogroupware.h"
#include "kodialogmanager.h"
#include "koeventpopupmenu.h"
#include "koalternatelabel.h"

#include <kcal/calendar.h>
#include <kcal/icaldrag.h>
#include <kcal/dndfactory.h>
#include <kcal/calfilter.h>
#include <kcal/incidenceformatter.h>

#include <kapplication.h>
#include <kcalendarsystem.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kholidays.h>
#include <KVBox>
#include <ksystemtimezone.h>
#include <kpushbutton.h>
#include <kcombobox.h>

#include <QLabel>
#include <QFrame>
#include <QLayout>
#include <QFont>
#include <QFontMetrics>
#include <QMenu>
#include <QPainter>
#include <QPushButton>
#include <QToolButton>
#include <QCursor>
#include <QBitArray>
#include <QPaintEvent>
#include <QGridLayout>
#include <QBoxLayout>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QListWidget>

using namespace KOrg;

TimeLabelsZone::TimeLabelsZone( KOAgendaView *parent, KOAgenda *agenda)
  : QWidget( parent ), mAgenda( agenda ),  mParent( parent )
{
  mTimeLabelsLayout = new QHBoxLayout( this );
  mTimeLabelsLayout->setMargin( 0 );
  mTimeLabelsLayout->setSpacing( 0 );

  init();
}

void TimeLabelsZone::reset()
{
  foreach( TimeLabels* label, mTimeLabelsList ) {
    label->hide();
    label->deleteLater();
  }
  mTimeLabelsList.clear();

  init();

  // Update some related geometry from the agenda view
  updateAll();
  mParent->createDayLabels();
  mParent->updateTimeBarWidth();
}

void TimeLabelsZone::init()
{
  addTimeLabels( KDateTime::Spec() );

  foreach( QString zoneStr, KOPrefs::instance()->timeScaleTimezones() ) {
    KTimeZone zone = KSystemTimeZones::zone( zoneStr );
    if ( zone.isValid() )
      addTimeLabels( zone );
  }
}

void TimeLabelsZone::addTimeLabels( const KDateTime::Spec &spec )
{
  TimeLabels *labels = new TimeLabels( spec, 24, this );
  mTimeLabelsList.prepend( labels );
  mTimeLabelsLayout->insertWidget( 0, labels );
  setupTimeLabel( labels );
}

void TimeLabelsZone::setupTimeLabel( TimeLabels* timeLabel )
{
  timeLabel->setAgenda( mAgenda );
  connect( mAgenda->verticalScrollBar(), SIGNAL( valueChanged(int) ),
           timeLabel, SLOT( positionChanged() ) );
  connect( timeLabel->verticalScrollBar(), SIGNAL( valueChanged(int) ),
           mParent, SLOT( setContentsPos(int) ) );
}

int TimeLabelsZone::timeLabelsWidth()
{
  if ( mTimeLabelsList.isEmpty() )
    return 0;
  else {
    return mTimeLabelsList.first()->width() * mTimeLabelsList.count();
  }
}

void TimeLabelsZone::updateAll()
{
  foreach( TimeLabels* timeLabel, mTimeLabelsList ) {
    timeLabel->updateConfig();
    timeLabel->positionChanged();
    timeLabel->repaint();
  }
}

void TimeLabelsZone::setTimeLabelsWidth( int width )
{
  foreach( TimeLabels* timeLabel, mTimeLabelsList ) {
    timeLabel->setFixedWidth( width / mTimeLabelsList.count() );
  }
}

#include "timelabelszone.moc"
