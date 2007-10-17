/*
  This file is part of KOrganizer.
  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

TimeScaleConfigDialog::TimeScaleConfigDialog( QWidget *parent )
  : QDialog( parent )
{
  ui.setupUi( this );

  QStringList list;
  const KTimeZones::ZoneMap timezones = KSystemTimeZones::zones();
  for (KTimeZones::ZoneMap::ConstIterator it = timezones.begin();  it != timezones.end();  ++it) {
    list.append(i18n(it.key().toUtf8()));
  }
  list.sort();
  ui.zoneCombo->addItems( list );
  ui.zoneCombo->setCurrentIndex( 0 );

  ui.addButton->setIcon( KIcon( "plus" ) );
  ui.removeButton->setIcon( KIcon( "edit-delete" ) );
  ui.upButton->setIcon( KIcon( "go-up" ) );
  ui.downButton->setIcon( KIcon( "go-down" ) );

  connect( ui.addButton, SIGNAL( clicked() ), SLOT( add() ) );
  connect( ui.removeButton, SIGNAL( clicked() ), SLOT( remove() ) );
  connect( ui.upButton, SIGNAL( clicked() ), SLOT( up() ) );
  connect( ui.downButton, SIGNAL( clicked() ), SLOT( down() ) );

  connect( ui.okButton, SIGNAL( clicked() ), SLOT( okClicked() ) );
  connect( ui.cancelButton, SIGNAL( clicked() ), SLOT( reject() ) );

  ui.listWidget->addItems( KOPrefs::instance()->timeScaleTimezones() );
}

void TimeScaleConfigDialog::okClicked()
{
  KOPrefs::instance()->setTimeScaleTimezones( zones() );
  accept();
}

void TimeScaleConfigDialog::add()
{
  // Do not add duplicates
  for ( int i=0; i < ui.listWidget->count(); i++ )
  {
    if ( ui.listWidget->item( i )->text() == ui.zoneCombo->currentText() )
      return;
  }

  ui.listWidget->addItem( ui.zoneCombo->currentText() );
}

void TimeScaleConfigDialog::remove()
{
  delete ui.listWidget->takeItem( ui.listWidget->currentRow() );
}

void TimeScaleConfigDialog::up()
{
  int row = ui.listWidget->currentRow();
  QListWidgetItem *item = ui.listWidget->takeItem( row );
  ui.listWidget->insertItem( qMax( row - 1, 0 ), item );
  ui.listWidget->setCurrentRow( qMax( row - 1, 0 ) );
}

void TimeScaleConfigDialog::down()
{
  int row = ui.listWidget->currentRow();
  QListWidgetItem *item = ui.listWidget->takeItem( row );
  ui.listWidget->insertItem( qMin( row + 1, ui.listWidget->count() ), item );
  ui.listWidget->setCurrentRow( qMin( row + 1, ui.listWidget->count() - 1 ) );
}

QStringList TimeScaleConfigDialog::zones()
{
  QStringList list;
  for ( int i=0; i < ui.listWidget->count(); i++ )
  {
    list << ui.listWidget->item( i )->text();
  }
  return list;
}

TimeLabels::TimeLabels( const KDateTime::Spec &spec, int rows, TimeLabelsZone *parent, Qt::WFlags f) :
// TODO_QT4: Use constructor without *name=0 param
  Q3ScrollView(parent,/*name*/0,f)
{
  mTimeLabelsZone = parent;
  mSpec = spec;

  mRows = rows;
  mMiniWidth = 0;

  mCellHeight = KOPrefs::instance()->mHourSize*4;

  enableClipper(true);

  setHScrollBarMode(AlwaysOff);
  setVScrollBarMode(AlwaysOff);

  resizeContents(50, int(mRows * mCellHeight) );

  viewport()->setBackgroundRole( QPalette::Background );
  setBackgroundRole( QPalette::Background );

  mMousePos = new QFrame(this);
  mMousePos->setLineWidth( 1 );
  mMousePos->setFrameStyle( QFrame::HLine );
//  mMousePos->setMargin(0);
  QPalette pal;
  pal.setColor( QPalette::Dark,
                KOPrefs::instance()->agendaMarcusBainsLineLineColor() );
  mMousePos->setPalette( pal );
  mMousePos->setFixedSize(width(), 1);
  addChild(mMousePos, 0, 0);

  if ( mSpec.isValid() )
    setToolTip( i18n( "Timezone:" ) + mSpec.timeZone().name() );
  else
    setToolTip( i18n( "Calendar display timezone:" )
                + KOPrefs::instance()->timeSpec().timeZone().name() );

}

void TimeLabels::mousePosChanged(const QPoint &pos)
{
  moveChild(mMousePos, 0, pos.y());

  // The repaint somehow prevents that the red line leaves a black artifact when
  // moved down. It's not a full solution, though.
  repaint();
}

void TimeLabels::showMousePos()
{
  mMousePos->show();
}

void TimeLabels::hideMousePos()
{
  mMousePos->hide();
}

void TimeLabels::setCellHeight(double height)
{
  mCellHeight = height;
}

/*
  Optimization so that only the "dirty" portion of the scroll view
  is redrawn.  Unfortunately, this is not called by default paintEvent() method.
*/
void TimeLabels::drawContents( QPainter *p, int cx, int cy, int cw, int ch )
{
  int beginning;

  if ( !mSpec.isValid() )
    beginning = 0;
  else
    beginning = ( mSpec.timeZone().currentOffset()
                 - KOPrefs::instance()->timeSpec().timeZone().currentOffset() )
                 / ( 60 * 60 );


  p->setBrush( palette().background() ); // TODO: theming, see if we want sth here...
  p->drawRect( cx, cy, cw, ch);

  // bug:  the parameters cx and cw are the areas that need to be
  //       redrawn, not the area of the widget.  unfortunately, this
  //       code assumes the latter...

  // now, for a workaround...
  cx = contentsX() + frameWidth()*2;
  cw = contentsWidth();

  // end of workaround
  int cell = ((int)(cy / mCellHeight)) + beginning;  // indicates which hour we start drawing with
  double y = ( cell - beginning ) * mCellHeight;
  QFontMetrics fm = fontMetrics();
  QString hour;
  int timeHeight = fm.ascent();
  QFont hourFont = KOPrefs::instance()->agendaTimeLabelsFont();
  p->setFont( font() );

  QString suffix;
  if ( ! KGlobal::locale()->use12Clock() ) {
      suffix = "00";
  } else {
    suffix = "am";
    if (cell > 11) suffix = "pm";
  }

  // We adjust the size of the hour font to keep it reasonable
  if ( timeHeight >  mCellHeight ) {
    timeHeight = int(mCellHeight-1);
    int pointS = hourFont.pointSize();
    while ( pointS > 4 ) { // TODO: use smallestReadableFont() when added to kdelibs
      hourFont.setPointSize( pointS );
      fm = QFontMetrics( hourFont );
      if ( fm.ascent() < mCellHeight )
        break;
      -- pointS;
    }
    fm = QFontMetrics( hourFont );
    timeHeight = fm.ascent();
  }
  //timeHeight -= (timeHeight/4-2);
  QFont suffixFont = hourFont;
  suffixFont.setPointSize( suffixFont.pointSize()/2 );
  QFontMetrics fmS(  suffixFont );
  int startW = mMiniWidth - frameWidth()-2 ;
  int tw2 = fmS.width(suffix);
  int divTimeHeight = (timeHeight-1) /2 - 1;
  //testline
  //p->drawLine(0,0,0,contentsHeight());
  while (y < cy + ch+mCellHeight) {
    // hour, full line
    p->drawLine( cx, int(y), cw+2, int(y) );

    hour.setNum(cell % 24 );
    // handle different timezones
    if ( cell < 0 )
      hour.setNum( cell + 24 );
    // handle 24h and am/pm time formats
    if (KGlobal::locale()->use12Clock()) {
      if (cell == 12) suffix = "pm";
      if (cell == 0) hour.setNum(12);
      if (cell > 12) hour.setNum(cell - 12);
    }

    QPen pen;
    if ( cell < 0 || cell >= 24 ) {
      pen.setColor( QColor( 150, 150, 150 ) );
    } else {
      pen.setColor( QPalette::Text );
    }
    p->setPen( pen );

    // center and draw the time label
    int timeWidth = fm.width(hour);
    int offset = startW - timeWidth - tw2 -1 ;
    p->setFont( hourFont );
    p->drawText( offset, int(y+timeHeight), hour);
    p->setFont( suffixFont );
    offset = startW - tw2;
    p->drawText( offset, int(y+timeHeight-divTimeHeight), suffix);

    // increment indices
    y += mCellHeight;
    cell++;
  }

}

/**
   Calculates the minimum width.
*/
int TimeLabels::minimumWidth() const
{
  return mMiniWidth;
}

/** updates widget's internal state */
void TimeLabels::updateConfig()
{
  setFont(KOPrefs::instance()->agendaTimeLabelsFont());

  QString test = "20";
  if ( KGlobal::locale()->use12Clock() )
      test = "12";
  mMiniWidth = fontMetrics().width( test );
  if ( KGlobal::locale()->use12Clock() )
      test = "pm";
  else {
      test = "00";
  }
  QFont sFont = font();
  sFont.setPointSize(  sFont.pointSize()/2 );
  QFontMetrics fmS(   sFont );
  mMiniWidth += fmS.width(  test ) + frameWidth()*2+4 ;
  // update geometry restrictions based on new settings
  setFixedWidth(  mMiniWidth );

  // update HourSize
  mCellHeight = KOPrefs::instance()->mHourSize*4;
  // If the agenda is zoomed out so that more then 24 would be shown,
  // the agenda only shows 24 hours, so we need to take the cell height
  // from the agenda, which is larger than the configured one!
  if ( mCellHeight < 4*mAgenda->gridSpacingY() )
       mCellHeight = 4*mAgenda->gridSpacingY();
  resizeContents( mMiniWidth, int(mRows * mCellHeight+1) );
}

/** update time label positions */
void TimeLabels::positionChanged()
{
  int adjustment = mAgenda->contentsY();
  setContentsPos(0, adjustment);
}

/**  */
void TimeLabels::setAgenda(KOAgenda* agenda)
{
  mAgenda = agenda;

  connect(mAgenda, SIGNAL(mousePosSignal(const QPoint &)), this, SLOT(mousePosChanged(const QPoint &)));
  connect(mAgenda, SIGNAL(enterAgenda()), this, SLOT(showMousePos()));
  connect(mAgenda, SIGNAL(leaveAgenda()), this, SLOT(hideMousePos()));
  connect(mAgenda, SIGNAL(gridSpacingYChanged( double ) ), this, SLOT( setCellHeight( double ) ) );
}


/** This is called in response to repaint() */
void TimeLabels::paintEvent(QPaintEvent*)
{
//  kDebug(5850) <<"paintevent...";
  // this is another hack!
//  QPainter painter(this);
  //QString c
  repaintContents(contentsX(), contentsY(), visibleWidth(), visibleHeight());
}

void TimeLabels::contextMenuEvent( QContextMenuEvent *event )
{
  Q_UNUSED( event );

  QMenu popup( this );
  QAction *editTimeZones = popup.addAction( KIcon( "edit" ), i18n( "&Edit timezones" ) );
  QAction *removeTimeZone = popup.addAction( KIcon( "delete" ), i18n( "&Remove %1 timezone", mSpec.timeZone().name() ) );
  if ( !mSpec.isValid() )
    removeTimeZone->setEnabled( false );

  QAction *activatedAction = popup.exec( QCursor::pos() );
  if ( activatedAction == editTimeZones ) {
    TimeScaleConfigDialog dialog( this );
    if ( dialog.exec() == QDialog::Accepted )
      mTimeLabelsZone->reset();
  } else if ( activatedAction == removeTimeZone ) {
    QStringList list = KOPrefs::instance()->timeScaleTimezones();
    list.removeAll( mSpec.timeZone().name() );
    KOPrefs::instance()->setTimeScaleTimezones( list );
    mTimeLabelsZone->reset();
    hide();
    deleteLater();
  }
}

KDateTime::Spec TimeLabels::timeSpec()
{
  return mSpec;
}

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
