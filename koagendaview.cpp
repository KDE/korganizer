/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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

#include <qhbox.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qframe.h>
#include <qlayout.h>
#ifndef KORG_NOSPLITTER
#include <qsplitter.h>
#endif
#include <qfont.h>
#include <qfontmetrics.h>
#include <qpopupmenu.h>
#include <qtooltip.h>
#include <qpainter.h>
#include <qpushbutton.h>
#include <qcursor.h>
#include <qbitarray.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kglobalsettings.h>

#include <libkcal/calendar.h>
#include <libkcal/icaldrag.h>
#include <libkcal/dndfactory.h>
#include <libkcal/calfilter.h>

#include <kcalendarsystem.h>

#include "koglobals.h"
#ifndef KORG_NOPLUGINS
#include "kocore.h"
#endif
#include "koprefs.h"
#include "koagenda.h"
#include "koagendaitem.h"

#include "koincidencetooltip.h"
#include "kogroupware.h"
#include "kodialogmanager.h"
#include "koeventpopupmenu.h"

#include "koagendaview.h"
#include "koagendaview.moc"

using namespace KOrg;

TimeLabels::TimeLabels(int rows,QWidget *parent,const char *name,WFlags f) :
  QScrollView(parent,name,f)
{
  mRows = rows;

  mCellHeight = KOPrefs::instance()->mHourSize*4;

  enableClipper(true);

  setHScrollBarMode(AlwaysOff);
  setVScrollBarMode(AlwaysOff);

  resizeContents(50,mRows * mCellHeight);

  viewport()->setBackgroundMode( PaletteBackground );
}

void TimeLabels::setCellHeight(int height)
{
  mCellHeight = height;
}

/*
  Optimization so that only the "dirty" portion of the scroll view
  is redrawn.  Unfortunately, this is not called by default paintEvent() method.
*/
void TimeLabels::drawContents(QPainter *p,int cx, int cy, int cw, int ch)
{
  // bug:  the parameters cx, cy, cw, ch are the areas that need to be
  //       redrawn, not the area of the widget.  unfortunately, this
  //       code assumes the latter...

  // now, for a workaround...
  // these two assignments fix the weird redraw bug
  cx = contentsX() + 2;
  cw = contentsWidth() - 2;
  int visWidth = visibleWidth();
  double cellHeight=mCellHeight;
  if (mAgenda) cellHeight=(4*mAgenda->gridSpacingY());
  // end of workaround

  int cell = ((int)(cy/cellHeight));
  double y = (cell * cellHeight);
  QFontMetrics fm = fontMetrics();
  QString hour;
  QString suffix;
  QString fullTime;

  while (y < cy + ch) {
    p->drawLine(cx,(int)y,cx+cw,(int)y);
    hour.setNum(cell);
    suffix = "am";

    // handle 24h and am/pm time formats
    // FIXME: don't construct this manually, might break translation!
    if (KGlobal::locale()->use12Clock()) {
      if (cell > 11) suffix = "pm";
      if (cell == 0) hour.setNum(12);
      if (cell > 12) hour.setNum(cell - 12);
    } else {
      suffix = ":00";
    }

    // create string in format of "XX:XX" or "XXpm/am"
    fullTime = hour + suffix;

    // center and draw the time label
    QRect r( cx, (int)y+3, visWidth-4, (int)(y+cellHeight-3) );
    p->drawText ( r, Qt::AlignHCenter | Qt::AlignTop | Qt::SingleLine, fullTime );

    // increment indices
    y += cellHeight;
    cell++;
  }
}

/**
   Calculates the minimum width.
*/
int TimeLabels::minimumWidth() const
{
  QFontMetrics fm = fontMetrics();

  int borderWidth = 4;

  // the maximum width possible
  int width = fm.width("88:88") + 2*borderWidth;

  return width;
}

/** updates widget's internal state */
void TimeLabels::updateConfig()
{
  // set the font
//  config->setGroup("Fonts");
//  QFont font = config->readFontEntry("TimeBar Font");
  setFont(KOPrefs::instance()->mTimeBarFont);

  // update geometry restrictions based on new settings
  setFixedWidth(minimumWidth());

  // update HourSize
  mCellHeight = KOPrefs::instance()->mHourSize*4;
  if (mCellHeight>mAgenda->gridSpacingY())
    mCellHeight=(int)(4*mAgenda->gridSpacingY());
        // FIXME: Why the heck do we set the width to 50???
  resizeContents(50,mRows * mCellHeight);
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
}


/** This is called in response to repaint() */
void TimeLabels::paintEvent(QPaintEvent*)
{
//  kdDebug(5850) << "paintevent..." << endl;
  // this is another hack!
//  QPainter painter(this);
  //QString c
  repaintContents(contentsX(), contentsY(), visibleWidth(), visibleHeight());
}

////////////////////////////////////////////////////////////////////////////

EventIndicator::EventIndicator(Location loc,QWidget *parent,const char *name)
  : QFrame(parent,name)
{
  mColumns = 1;
  mTopBox = 0;
  mLocation = loc;
  mTopLayout = 0;

  if (mLocation == Top) mPixmap = KOGlobals::self()->smallIcon("upindicator");
  else mPixmap = KOGlobals::self()->smallIcon("downindicator");

  setMinimumHeight(mPixmap.height());
}

EventIndicator::~EventIndicator()
{
}

void EventIndicator::drawContents(QPainter *p)
{
//  kdDebug(5850) << "======== top: " << contentsRect().top() << "  bottom "
//         << contentsRect().bottom() << "  left " << contentsRect().left()
//         << "  right " << contentsRect().right() << endl;

  int i;
  for(i=0;i<mColumns;++i) {
    if (mEnabled[i]) {
      int cellWidth = contentsRect().right()/mColumns;
      int xOffset = KOGlobals::self()->reverseLayout() ?
               (mColumns - 1 - i)*cellWidth + cellWidth/2 -mPixmap.width()/2 :
               i*cellWidth + cellWidth/2 -mPixmap.width()/2;
      p->drawPixmap(QPoint(xOffset,0),mPixmap);
    }
  }
}

void EventIndicator::changeColumns(int columns)
{
  mColumns = columns;
  mEnabled.resize(mColumns);

  update();
}

void EventIndicator::enableColumn(int column, bool enable)
{
  mEnabled[column] = enable;
}


#include <libkcal/incidence.h>

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////


KOAlternateLabel::KOAlternateLabel(QString shortlabel, QString longlabel,
    QString extensivelabel, QWidget *parent, const char *name )
  : QLabel(parent, name), mTextTypeFixed(false), mShortText(shortlabel),
    mLongText(longlabel), mExtensiveText(extensivelabel)
{
  setSizePolicy(QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ));
  if (mExtensiveText.isEmpty()) mExtensiveText = mLongText;
  squeezeTextToLabel();
}

KOAlternateLabel::~KOAlternateLabel()
{
}

void KOAlternateLabel::useShortText()
{
  mTextTypeFixed = true;
  QLabel::setText( mShortText );
  QToolTip::remove( this );
  QToolTip::add( this, mExtensiveText );
}

void KOAlternateLabel::useLongText()
{
  mTextTypeFixed = true;
  QLabel::setText( mLongText );
  QToolTip::remove( this );
  QToolTip::add( this, mExtensiveText );
}

void KOAlternateLabel::useExtensiveText()
{
  mTextTypeFixed = true;
  QLabel::setText( mExtensiveText );
  QToolTip::remove( this );
  QToolTip::hide();
}

void KOAlternateLabel::useDefaultText()
{
  mTextTypeFixed = false;
  squeezeTextToLabel();
}

void KOAlternateLabel::squeezeTextToLabel()
{
  if (mTextTypeFixed) return;

  QFontMetrics fm(fontMetrics());
  int labelWidth = size().width();
  int textWidth = fm.width(mLongText);
  int longTextWidth = fm.width(mExtensiveText);
  if (longTextWidth <= labelWidth) {
    QLabel::setText( mExtensiveText );
    QToolTip::remove( this );
    QToolTip::hide();
  } else if (textWidth <= labelWidth) {
    QLabel::setText( mLongText );
    QToolTip::remove( this );
    QToolTip::add( this, mExtensiveText );
  } else {
    QLabel::setText( mShortText );
    QToolTip::remove( this );
    QToolTip::add( this, mExtensiveText );
  }
}

void KOAlternateLabel::resizeEvent( QResizeEvent * )
{
  squeezeTextToLabel();
}

QSize KOAlternateLabel::minimumSizeHint() const
{
  QSize sh = QLabel::minimumSizeHint();
  sh.setWidth(-1);
  return sh;
}

void KOAlternateLabel::setText( const QString &text ) {
  mLongText = text;
  squeezeTextToLabel();
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

KOAgendaView::KOAgendaView(Calendar *cal,QWidget *parent,const char *name) :
  KOEventView (cal,parent,name), mExpandButton( 0 ), mAllowAgendaUpdate( true ),
  mUpdateItem( 0 )
{
  mSelectedDates.append(QDate::currentDate());

  mLayoutDayLabels = 0;
  mDayLabelsFrame = 0;
  mDayLabels = 0;

  bool isRTL = KOGlobals::self()->reverseLayout();

  if ( KOPrefs::instance()->compactDialogs() ) {
    if ( KOPrefs::instance()->mVerticalScreen ) {
      mExpandedPixmap = KOGlobals::self()->smallIcon( "1downarrow" );
      mNotExpandedPixmap = KOGlobals::self()->smallIcon( "1uparrow" );
    } else {
      mExpandedPixmap = KOGlobals::self()->smallIcon( isRTL ? "1leftarrow" : "1rightarrow" );
      mNotExpandedPixmap = KOGlobals::self()->smallIcon( isRTL ? "1rightarrow" : "1leftarrow" );
    }
  }

  QBoxLayout *topLayout = new QVBoxLayout(this);

  // Create day name labels for agenda columns
  mDayLabelsFrame = new QHBox(this);
  topLayout->addWidget(mDayLabelsFrame);

  // Create agenda splitter
#ifndef KORG_NOSPLITTER
  mSplitterAgenda = new QSplitter(Vertical,this);
  topLayout->addWidget(mSplitterAgenda);

#if KDE_IS_VERSION( 3, 1, 93 )
  mSplitterAgenda->setOpaqueResize( KGlobalSettings::opaqueResize() );
#else
  mSplitterAgenda->setOpaqueResize();
#endif

  mAllDayFrame = new QHBox(mSplitterAgenda);

  QWidget *agendaFrame = new QWidget(mSplitterAgenda);
#else
  QVBox *mainBox = new QVBox( this );
  topLayout->addWidget( mainBox );

  mAllDayFrame = new QHBox(mainBox);

  QWidget *agendaFrame = new QWidget(mainBox);
#endif

  // Create all-day agenda widget
  mDummyAllDayLeft = new QVBox( mAllDayFrame );

  if ( KOPrefs::instance()->compactDialogs() ) {
    mExpandButton = new QPushButton(mDummyAllDayLeft);
    mExpandButton->setPixmap( mNotExpandedPixmap );
    mExpandButton->setSizePolicy( QSizePolicy( QSizePolicy::Fixed,
                                  QSizePolicy::Fixed ) );
    connect( mExpandButton, SIGNAL( clicked() ), SIGNAL( toggleExpand() ) );
  } else {
    new QLabel( i18n("All Day"), mDummyAllDayLeft );
  }

  mAllDayAgenda = new KOAgenda(1,mAllDayFrame);
  QWidget *dummyAllDayRight = new QWidget(mAllDayFrame);

  // Create agenda frame
  QGridLayout *agendaLayout = new QGridLayout(agendaFrame,3,3);
//  QHBox *agendaFrame = new QHBox(splitterAgenda);

  // create event indicator bars
  mEventIndicatorTop = new EventIndicator(EventIndicator::Top,agendaFrame);
  agendaLayout->addWidget(mEventIndicatorTop,0,1);
  mEventIndicatorBottom = new EventIndicator(EventIndicator::Bottom,
                                             agendaFrame);
  agendaLayout->addWidget(mEventIndicatorBottom,2,1);
  QWidget *dummyAgendaRight = new QWidget(agendaFrame);
  agendaLayout->addWidget(dummyAgendaRight,0,2);

  // Create time labels
  mTimeLabels = new TimeLabels(24,agendaFrame);
  agendaLayout->addWidget(mTimeLabels,1,0);

  // Create agenda
  mAgenda = new KOAgenda(1,96,KOPrefs::instance()->mHourSize,agendaFrame);
  agendaLayout->addMultiCellWidget(mAgenda,1,1,1,2);
  agendaLayout->setColStretch(1,1);

  // Create event context menu for agenda
  mAgendaPopup = eventPopup();

  // Create event context menu for all day agenda
  mAllDayAgendaPopup = eventPopup();

  // make connections between dependent widgets
  mTimeLabels->setAgenda(mAgenda);

  // Update widgets to reflect user preferences
//  updateConfig();

  createDayLabels();

  // these blank widgets make the All Day Event box line up with the agenda
  dummyAllDayRight->setFixedWidth(mAgenda->verticalScrollBar()->width());
  dummyAgendaRight->setFixedWidth(mAgenda->verticalScrollBar()->width());
  
  updateTimeBarWidth();

  // Scrolling
  connect(mAgenda->verticalScrollBar(),SIGNAL(valueChanged(int)),
          mTimeLabels, SLOT(positionChanged()));

  connect( mAgenda,
    SIGNAL( zoomView( const int, const QPoint & ,const Qt::Orientation ) ),
    SLOT( zoomView( const int, const QPoint &, const Qt::Orientation ) ) );

  connect(mTimeLabels->verticalScrollBar(),SIGNAL(valueChanged(int)),
          SLOT(setContentsPos(int)));

  // Create Events, depends on type of agenda
  connect( mAgenda, SIGNAL(newEventSignal(const QPoint &)),
                    SLOT(newEvent(const QPoint &)));
  connect( mAllDayAgenda, SIGNAL(newEventSignal(const QPoint &)),
                          SLOT(newEventAllDay(const QPoint &)));
  connect( mAgenda, SIGNAL(newEventSignal(const QPoint &, const QPoint &)),
                    SLOT(newEvent(const QPoint &, const QPoint &)));
  connect( mAllDayAgenda, SIGNAL(newEventSignal(const QPoint &, const QPoint &)),
                          SLOT(newEventAllDay(const QPoint &)));
  connect( mAgenda, SIGNAL(newTimeSpanSignal(const QPoint &, const QPoint &)),
                    SLOT(newTimeSpanSelected(const QPoint &, const QPoint &)));
  connect( mAllDayAgenda, SIGNAL(newTimeSpanSignal(const QPoint &, const QPoint &)),
                          SLOT(newTimeSpanSelectedAllDay(const QPoint &, const QPoint &)));

  // event indicator update
  connect( mAgenda, SIGNAL(lowerYChanged(int)),
                    SLOT(updateEventIndicatorTop(int)));
  connect( mAgenda, SIGNAL(upperYChanged(int)),
                    SLOT(updateEventIndicatorBottom(int)));

  connectAgenda( mAgenda, mAgendaPopup, mAllDayAgenda );
  connectAgenda( mAllDayAgenda, mAllDayAgendaPopup, mAgenda);
}


KOAgendaView::~KOAgendaView()
{
  delete mAgendaPopup;
  delete mAllDayAgendaPopup;
}

void KOAgendaView::connectAgenda( KOAgenda *agenda, QPopupMenu *popup,
                                  KOAgenda *otherAgenda )
{
  connect( agenda, SIGNAL( showIncidencePopupSignal( Incidence *, const QDate & ) ),
           popup, SLOT( showIncidencePopup( Incidence *, const QDate & ) ) );

  connect( agenda, SIGNAL( showNewEventPopupSignal() ),
           SLOT( showNewEventPopup() ) );

  agenda->setCalendar( calendar() );

  // Create/Show/Edit/Delete Event
  connect( agenda, SIGNAL( newEventSignal() ), SIGNAL( newEventSignal() ) );

  connect( agenda, SIGNAL( newStartSelectSignal() ),
           otherAgenda, SLOT( clearSelection() ) );

  connect( agenda, SIGNAL( editIncidenceSignal( Incidence * ) ),
                   SIGNAL( editIncidenceSignal( Incidence * ) ) );
  connect( agenda, SIGNAL( showIncidenceSignal( Incidence * ) ),
                   SIGNAL( showIncidenceSignal( Incidence * ) ) );
  connect( agenda, SIGNAL( deleteIncidenceSignal( Incidence * ) ),
                   SIGNAL( deleteIncidenceSignal( Incidence * ) ) );

  connect( agenda, SIGNAL( startMultiModify( const QString & ) ),
                   SIGNAL( startMultiModify( const QString & ) ) );
  connect( agenda, SIGNAL( endMultiModify() ),
                   SIGNAL( endMultiModify() ) );

  connect( agenda, SIGNAL( itemModified( KOAgendaItem * ) ),
                   SLOT( updateEventDates( KOAgendaItem * ) ) );
  connect( agenda, SIGNAL( enableAgendaUpdate( bool ) ),
                   SLOT( enableAgendaUpdate( bool ) ) );

  // drag signals
  connect( agenda, SIGNAL( startDragSignal( Incidence * ) ),
           SLOT( startDrag( Incidence * ) ) );

  // synchronize selections
  connect( agenda, SIGNAL( incidenceSelected( Incidence * ) ),
           otherAgenda, SLOT( deselectItem() ) );
  connect( agenda, SIGNAL( incidenceSelected( Incidence * ) ),
           SIGNAL( incidenceSelected( Incidence * ) ) );

  // rescheduling of todos by d'n'd
  connect( agenda, SIGNAL( droppedToDo( Todo *, const QPoint &, bool ) ),
           SLOT( slotTodoDropped( Todo *, const QPoint &, bool ) ) );

}

void KOAgendaView::zoomInVertically( )
{
  KOPrefs::instance()->mHourSize++;
  mAgenda->updateConfig();
  mAgenda->checkScrollBoundaries();

  mTimeLabels->updateConfig();
  mTimeLabels->positionChanged();
  mTimeLabels->repaint();

  updateView();
}

void KOAgendaView::zoomOutVertically( ) 
{

  if ( KOPrefs::instance()->mHourSize > 4 ) { 
    
    KOPrefs::instance()->mHourSize--;
    mAgenda->updateConfig();
    mAgenda->checkScrollBoundaries();
  
    mTimeLabels->updateConfig();
    mTimeLabels->positionChanged();
    mTimeLabels->repaint();
  
    updateView();
  }
}

// FIXME: Don't have two zoomInHorizontally, but use a default value for the date.
void KOAgendaView::zoomInHorizontally( )
{
  QDate date=mAgenda->selectedIncidenceDate();
  if ( date.isValid() ) {
     zoomInHorizontally( date );
  } else {
    QDate begin;
    QDate end;
    
    begin = mSelectedDates.first();
    end = mSelectedDates.last();
    int d = begin.daysTo( end );
    if ( d > 1 ) {
      emit zoomViewHorizontally( begin.addDays(1),d-1 );
    }
  }
}

// FIXME: Don't have two zoomOutHorizontally, but use a default value for the date.
void KOAgendaView::zoomOutHorizontally( ) 
{
  QDate date=mAgenda->selectedIncidenceDate();
  if ( date.isValid() ) {
     zoomOutHorizontally( date );
  } else {
    QDate begin;
    QDate end;
    
    begin = mSelectedDates.first();
    end = mSelectedDates.last();
    int d = begin.daysTo(end);
    emit zoomViewHorizontally( begin.addDays(-1),d+3 );
  }
}

void KOAgendaView::zoomInHorizontally( const QDate &date )
{
  QDate begin;
  QDate end;
  int ndays;
  
  begin = mSelectedDates.first();
  end = mSelectedDates.last();
  
  ndays = begin.daysTo( end );
  if ( ndays <= 2 ) 
    emit zoomViewHorizontally( date,1 );
      //showDates( date, date );
  else
    emit zoomViewHorizontally( date.addDays( -( ( ndays/2 )-1 ) ) , ndays-1 );
}

void KOAgendaView::zoomOutHorizontally( const QDate &date )
{  
  QDate begin;
  QDate end; 
  int ndays;
  
  begin = mSelectedDates.first();
  end = mSelectedDates.last();

  ndays = begin.daysTo( end );
  if ( abs( ndays ) >= 31 )
    kdDebug(5850) << "change to the mounth view?"<<endl;
  else
    //We want to center the date
    emit zoomViewHorizontally( date.addDays( -( ( ndays/2 )+1 ) ) , ndays+3 );
}

void KOAgendaView::zoomView( const int delta, const QPoint &pos,
  const Qt::Orientation orient )
{
  static QDate zoomDate;
  static QTimer t( this );


  //Zoom to the selected incidence, on the other way
  // zoom to the date on screen after the first mousewheel move.
  if ( orient == Qt::Horizontal ) {
    QDate date=mAgenda->selectedIncidenceDate();
    if ( date.isValid() )
      zoomDate=date;
    else{
      if ( !t.isActive() ) {
        zoomDate= mSelectedDates[pos.x()];
      }
      t.start ( 1000,true );
    }
    if ( delta > 0 ) 
      zoomOutHorizontally( zoomDate );
    else 
      zoomInHorizontally( zoomDate );
  } else {
    // Vertical zoom
    if ( delta > 0 ) 
      zoomOutVertically();
    else  
      zoomInVertically();
  }
}

void KOAgendaView::createDayLabels()
{
//  kdDebug(5850) << "KOAgendaView::createDayLabels()" << endl;

  // ### Before deleting and recreating we could check if mSelectedDates changed...
  // It would remove some flickering and gain speed (since this is called by
  // each updateView() call)
  delete mDayLabels;

  mDayLabels = new QFrame (mDayLabelsFrame);
  mLayoutDayLabels = new QHBoxLayout(mDayLabels);
  mLayoutDayLabels->addSpacing(mTimeLabels->width());

  const KCalendarSystem*calsys=KOGlobals::self()->calendarSystem();

  DateList::ConstIterator dit;
  for( dit = mSelectedDates.begin(); dit != mSelectedDates.end(); ++dit ) {
    QDate date = *dit;
    QBoxLayout *dayLayout = new QVBoxLayout(mLayoutDayLabels);
    mLayoutDayLabels->setStretchFactor(dayLayout, 1);
//    dayLayout->setMinimumWidth(1);

    int dW = calsys->dayOfWeek(date);
    QString veryLongStr = KGlobal::locale()->formatDate( date );
    QString longstr = i18n( "short_weekday date (e.g. Mon 13)","%1 %2" )
        .arg( calsys->weekDayName( dW, true ) )
        .arg( calsys->day(date) );
    QString shortstr = QString::number(calsys->day(date));

    KOAlternateLabel *dayLabel = new KOAlternateLabel(shortstr,
      longstr, veryLongStr, mDayLabels);
    dayLabel->setMinimumWidth(1);
    dayLabel->setAlignment(QLabel::AlignHCenter);
    if (date == QDate::currentDate()) {
      QFont font = dayLabel->font();
      font.setBold(true);
      dayLabel->setFont(font);
    }
    dayLayout->addWidget(dayLabel);

#ifndef KORG_NOPLUGINS
    CalendarDecoration::List cds = KOCore::self()->calendarDecorations();
    CalendarDecoration *it;
    for(it = cds.first(); it; it = cds.next()) {
      QString text = it->shortText( date );
      if ( !text.isEmpty() ) {
        // use a KOAlternateLabel so when the text doesn't fit any more a tooltip is used
        KOAlternateLabel*label = new KOAlternateLabel( text, text, QString::null, mDayLabels );
        label->setMinimumWidth(1);
        label->setAlignment(AlignCenter);
        dayLayout->addWidget(label);
      }
    }

    for(it = cds.first(); it; it = cds.next()) {
      QWidget *wid = it->smallWidget(mDayLabels,date);
      if ( wid ) {
//      wid->setHeight(20);
        dayLayout->addWidget(wid);
      }
    }
#endif
  }

  mLayoutDayLabels->addSpacing(mAgenda->verticalScrollBar()->width());
  mDayLabels->show();
}

void KOAgendaView::enableAgendaUpdate( bool enable )
{
  mAllowAgendaUpdate = enable;
}

int KOAgendaView::maxDatesHint()
{
  // Not sure about the max number of events, so return 0 for now.
  return 0;
}

int KOAgendaView::currentDateCount()
{
  return mSelectedDates.count();
}

Incidence::List KOAgendaView::selectedIncidences()
{
  Incidence::List selected;
  Incidence *incidence;

  incidence = mAgenda->selectedIncidence();
  if (incidence) selected.append(incidence);

  incidence = mAllDayAgenda->selectedIncidence();
  if (incidence) selected.append(incidence);

  return selected;
}

DateList KOAgendaView::selectedDates()
{
  DateList selected;
  QDate qd;

  qd = mAgenda->selectedIncidenceDate();
  if (qd.isValid()) selected.append(qd);

  qd = mAllDayAgenda->selectedIncidenceDate();
  if (qd.isValid()) selected.append(qd);

  return selected;
}

bool KOAgendaView::eventDurationHint( QDateTime &startDt, QDateTime &endDt,
                                      bool &allDay )
{
  if ( selectionStart().isValid() ) {
    startDt = selectionStart();
    endDt = selectionEnd();
    allDay = selectedIsAllDay();
    return true;
  }
  return false;
}

/** returns if only a single cell is selected, or a range of cells */
bool KOAgendaView::selectedIsSingleCell()
{
  if ( !selectionStart().isValid() || !selectionEnd().isValid() ) return false;

  if (selectedIsAllDay()) {
    int days = selectionStart().daysTo(selectionEnd());
    return ( days < 1 );
  } else {
    int secs = selectionStart().secsTo(selectionEnd());
    return ( secs <= 24*60*60/mAgenda->rows() );
  }
}


void KOAgendaView::updateView()
{
//  kdDebug(5850) << "KOAgendaView::updateView()" << endl;
  fillAgenda();
}


/*
  Update configuration settings for the agenda view. This method is not
  complete.
*/
void KOAgendaView::updateConfig()
{
//  kdDebug(5850) << "KOAgendaView::updateConfig()" << endl;

  // update config for children
  mTimeLabels->updateConfig();
  mAgenda->updateConfig();
  mAllDayAgenda->updateConfig();

  // widget synchronization
  // FIXME: find a better way, maybe signal/slot
  mTimeLabels->positionChanged();

  // for some reason, this needs to be called explicitly
  mTimeLabels->repaint();

  updateTimeBarWidth();

  // ToolTips displaying summary of events
  KOAgendaItem::toolTipGroup()->setEnabled(KOPrefs::instance()
                                           ->mEnableToolTips);

  setHolidayMasks();

  createDayLabels();

  updateView();
}

void KOAgendaView::updateTimeBarWidth()
{
  int width;

  width = mDummyAllDayLeft->fontMetrics().width( i18n("All Day") );

#if 0
  if ( mDummyAllDayLeft->width() > mTimeLabels->width() ) {
    width = mDummyAllDayLeft->width();
  } else {
    width = mTimeLabels->width();
  }
#endif
  
  mDummyAllDayLeft->setFixedWidth( width );
  mTimeLabels->setFixedWidth( width );
}


void KOAgendaView::updateEventDates( KOAgendaItem *item )
{
//  kdDebug(5850) << "KOAgendaView::updateEventDates(): " << item->text() << endl;

  QDateTime startDt,endDt;

  // Start date of this incidence, calculate the offset from it (so recurring and
  // non-recurring items can be treated exactly the same, we never need to check
  // for doesRecur(), because we only move the start day by the number of days the
  // agenda item was really moved. Smart, isn't it?)
  QDate thisDate;
  if ( item->cellXLeft() < 0 ) {
    thisDate = ( mSelectedDates.first() ).addDays( item->cellXLeft() );
  } else {
    thisDate = mSelectedDates[ item->cellXLeft() ];
  }
  QDate oldThisDate( item->itemDate() );
  int daysOffset = oldThisDate.daysTo( thisDate );
  int daysLength = 0;

//  startDt.setDate( startDate );

  Incidence *incidence = item->incidence();
  if ( !incidence ) return;

  QTime startTime(0,0,0), endTime(0,0,0);
  if ( incidence->doesFloat() ) {
    daysLength = item->cellWidth() - 1;
  } else {
    startTime = mAgenda->gyToTime( item->cellYTop() );
    if ( item->lastMultiItem() ) {
      endTime = mAgenda->gyToTime( item->lastMultiItem()->cellYBottom() + 1 );
      daysLength = item->lastMultiItem()->cellXLeft() - item->cellXLeft();
    } else {
      endTime = mAgenda->gyToTime( item->cellYBottom() + 1 );
    }
  }

//  kdDebug(5850) << "KOAgendaView::updateEventDates(): now setting dates" << endl;
  // FIXME: use a visitor here
  if ( incidence->type() == "Event" ) {
    startDt = incidence->dtStart();
    startDt = startDt.addDays( daysOffset );
    startDt.setTime( startTime );
    endDt = startDt.addDays( daysLength );
    endDt.setTime( endTime );
    Event*ev = static_cast<Event*>(incidence);
    if( incidence->dtStart() == startDt && ev->dtEnd() == endDt ) {
      // No change
      return;
    }
    incidence->setDtStart( startDt );
    ev->setDtEnd( endDt );
  } else if ( incidence->type() == "Todo" ) {
    Todo *td = static_cast<Todo*>(incidence);
    endDt = td->dtDue();
    endDt = endDt.addDays( daysOffset );
    endDt.setTime( endTime );

    if( td->dtDue() == endDt ) {
      // No change
      return;
    }
    td->setDtDue( endDt );
  }
  // FIXME: Adjusting the recurrence should really go to CalendarView so this
  // functionality will also be available in other views!
  Recurrence *recur = incidence->recurrence();
  if ( recur && (recur->doesRecur()!=Recurrence::rNone) && (daysOffset!=0) ) {
    switch ( recur->doesRecur() ) {
      case Recurrence::rYearlyPos: {
        int freq = recur->frequency();
        int duration = recur->duration();
        QDate endDt( recur->endDate() );
        bool negative = false;

        QPtrList<Recurrence::rMonthPos> monthPos( recur->yearMonthPositions() );
        if ( monthPos.first() ) {
          negative = monthPos.first()->negative;
        }
        QBitArray days( 7 );
        int pos = 0;
        days.fill( false );
        days.setBit( thisDate.dayOfWeek() - 1 );
        if ( negative ) {
          pos =  - ( thisDate.daysInMonth() - thisDate.day() - 1 ) / 7 - 1;
        } else {
          pos =  ( thisDate.day()-1 ) / 7 + 1;
        }
        // Terrible hack: to change the month days, I have to unset the recurrence, and set all days manually again
        recur->unsetRecurs();
        if ( duration != 0 ) {
          recur->setYearly( Recurrence::rYearlyPos, freq, duration );
        } else {
          recur->setYearly( Recurrence::rYearlyPos, freq, endDt );
        }
        recur->addYearlyMonthPos( pos, days );
        recur->addYearlyNum( thisDate.month() );

        break; }
        case Recurrence::rYearlyDay: {
          int freq = recur->frequency();
          int duration = recur->duration();
          QDate endDt( recur->endDate() );
        // Terrible hack: to change the month days, I have to unset the recurrence, and set all days manually again
          recur->unsetRecurs();
          if ( duration == 0 ) { // end by date
            recur->setYearly( Recurrence::rYearlyDay, freq, endDt );
          } else {
            recur->setYearly( Recurrence::rYearlyDay, freq, duration );
          }
          recur->addYearlyNum( thisDate.dayOfYear() );
          break; }
          case Recurrence::rYearlyMonth: {
            int freq = recur->frequency();
            int duration = recur->duration();
            QDate endDt( recur->endDate() );
        // Terrible hack: to change the month days, I have to unset the recurrence, and set all days manually again
            recur->unsetRecurs();
            if ( duration != 0 ) {
              recur->setYearlyByDate( thisDate.day(), recur->feb29YearlyType(), freq, duration );
            } else {
              recur->setYearlyByDate( thisDate.day(), recur->feb29YearlyType(), freq, endDt );
            }
            recur->addYearlyNum( thisDate.month() );
            break; }
            case Recurrence::rMonthlyPos: {
              int freq = recur->frequency();
              int duration = recur->duration();
              QDate endDt( recur->endDate() );
              QPtrList<Recurrence::rMonthPos> monthPos( recur->monthPositions() );
              if ( !monthPos.isEmpty() ) {
          // FIXME: How shall I adapt the day x of week Y if we move the date across month borders???
          // for now, just use the date of the moved item and assume the recurrence only occurs on that day.
          // That's fine for korganizer, but might mess up other organizers.
                QBitArray rDays( 7 );
                rDays = monthPos.first()->rDays;
                bool negative = monthPos.first()->negative;
                int newPos;
                rDays.fill( false );
                rDays.setBit( thisDate.dayOfWeek() - 1 );
                if ( negative ) {
                  newPos =  - ( thisDate.daysInMonth() - thisDate.day() - 1 ) / 7 - 1;
                } else {
                  newPos =  ( thisDate.day()-1 ) / 7 + 1;
                }

          // Terrible hack: to change the month days, I have to unset the recurrence, and set all days manually again
                recur->unsetRecurs();
                if ( duration == 0 ) { // end by date
                  recur->setMonthly( Recurrence::rMonthlyPos, freq, endDt );
                } else {
                  recur->setMonthly( Recurrence::rMonthlyPos, freq, duration );
                }
                recur->addMonthlyPos( newPos, rDays );
              }
              break;}
              case Recurrence::rMonthlyDay: {
                int freq = recur->frequency();
                int duration = recur->duration();
                QDate endDt( recur->endDate() );
                QPtrList<int> monthDays( recur->monthDays() );
        // Terrible hack: to change the month days, I have to unset the recurrence, and set all days manually again
                recur->unsetRecurs();
                if ( duration == 0 ) { // end by date
                  recur->setMonthly( Recurrence::rMonthlyDay, freq, endDt );
                } else {
                  recur->setMonthly( Recurrence::rMonthlyDay, freq, duration );
                }
        // FIXME: How shall I adapt the n-th day if we move the date across month borders???
        // for now, just use the date of the moved item and assume the recurrence only occurs on that day.
        // That's fine for korganizer, but might mess up other organizers.
                recur->addMonthlyDay( thisDate.day() );

                break;}
                case Recurrence::rWeekly: {
                  QBitArray days(7), oldDays( recur->days() );
                  int offset = daysOffset % 7;
                  if ( offset<0 ) offset = (offset+7) % 7;
        // rotate the days
                  for (int d=0; d<7; d++ ) {
                    days.setBit( (d+offset) % 7, oldDays.at(d) );
                  }
                  if ( recur->duration() == 0 ) { // end by date
                    recur->setWeekly( recur->frequency(), days, recur->endDate(), recur->weekStart() );
                  } else { // duration or no end
                    recur->setWeekly( recur->frequency(), days, recur->duration(), recur->weekStart() );
                  }
                  break;}
      // nothing to be done for the following:
      case Recurrence::rDaily:
      case Recurrence::rHourly:
      case Recurrence::rMinutely:
      case Recurrence::rNone:
      default:
        break;
    }
    if ( recur->duration()==0 ) { // end by date
      recur->setEndDate( recur->endDate().addDays( daysOffset ) );
    }
    KMessageBox::information( this, i18n("A recurring incidence was moved "
                              "to a different day. The recurrence settings "
                              "have been updated with that move. Please check "
                              "them in the incidence editor."), 
                              i18n("Recurrence Moved"), 
                              "RecurrenceMoveInAgendaWarning" );
  }

  // FIXME: use a visitor here
  if ( incidence->type() == "Event" ) {
    incidence->setDtStart( startDt );
    (static_cast<Event*>( incidence ) )->setDtEnd( endDt );
  } else if ( incidence->type() == "Todo" ) {
    (static_cast<Todo*>( incidence ) )->setDtDue( endDt );
  }
  item->setItemDate( startDt.date() );

  KOIncidenceToolTip::remove( item );
  KOIncidenceToolTip::add( item, incidence, KOAgendaItem::toolTipGroup() );

  // don't update the agenda as the item already has the correct coordinates.
  // an update would delete the current item and recreate it, but we are still
  // using a pointer to that item! => CRASH
  enableAgendaUpdate( false );
  // We need to do this in a timer to make sure we are not deleting the item
  // we are currently working on, which would lead to crashes
  // Only the actually moved agenda item is already at the correct position and mustn't be
  // recreated. All others have to!!!
  if ( incidence->doesRecur() ) {
    mUpdateItem = incidence;
    QTimer::singleShot( 0, this, SLOT( doUpdateItem() ) );
  }

    enableAgendaUpdate( true );

//  kdDebug(5850) << "KOAgendaView::updateEventDates() done " << endl;
}

void KOAgendaView::doUpdateItem()
{
  if ( mUpdateItem ) {
    changeIncidenceDisplay( mUpdateItem, KOGlobals::INCIDENCEEDITED );
    mUpdateItem = 0;
  }
}



void KOAgendaView::showDates( const QDate &start, const QDate &end )
{
//  kdDebug(5850) << "KOAgendaView::selectDates" << endl;

  mSelectedDates.clear();

  QDate d = start;
  while (d <= end) {
    mSelectedDates.append(d);
    d = d.addDays( 1 );
  }

  // and update the view
  fillAgenda();
}


void KOAgendaView::showIncidences( const Incidence::List & )
{
  kdDebug(5850) << "KOAgendaView::showIncidences( const Incidence::List & ) is not yet implemented" << endl;
}

void KOAgendaView::insertIncidence( Incidence *incidence, QDate curDate,
                                    int curCol )
{
  // FIXME: Use a visitor here, or some other method to get rid of the dynamic_cast's
  Event *event = dynamic_cast<Event *>( incidence );
  Todo  *todo  = dynamic_cast<Todo  *>( incidence );

  if ( curCol < 0 ) {
    curCol = mSelectedDates.findIndex( curDate );
  }
  // The date for the event is not displayed, just ignore it
  if ( curCol < 0 || curCol > int( mSelectedDates.size() ) )
    return;

  int beginX;
  int endX;
  if ( event ) {
    beginX = curDate.daysTo( incidence->dtStart().date() ) + curCol;
    endX = curDate.daysTo( event->dtEnd().date() ) + curCol;
  } else if ( todo ) {
    beginX = curDate.daysTo( todo->dtDue().date() ) + curCol;
    endX = beginX;
  } else {
    return;
  }

  if ( incidence->doesFloat() ) {
    if ( incidence->recurrence()->doesRecur() ) {
      mAllDayAgenda->insertAllDayItem( incidence, curDate, curCol, curCol );
    } else {
      if ( ( beginX <= 0 && curCol == 0 ) || beginX == curCol ) {
        mAllDayAgenda->insertAllDayItem( incidence, curDate, beginX, endX );
      }
    }
  } else if ( event && event->isMultiDay() ) {
    int startY = mAgenda->timeToY( event->dtStart().time() );
    int endY = mAgenda->timeToY( event->dtEnd().time() ) - 1;
    if ( (beginX <= 0 && curCol == 0) || beginX == curCol ) {
      mAgenda->insertMultiItem( event, curDate, beginX, endX, startY, endY );
    }
    if ( beginX == curCol ) {
      mMaxY[curCol] = mAgenda->timeToY( QTime(23,59) );
      if ( startY < mMinY[curCol] ) mMinY[curCol] = startY;
    } else if ( endX == curCol ) {
      mMinY[curCol] = mAgenda->timeToY( QTime(0,0) );
      if ( endY > mMaxY[curCol] ) mMaxY[curCol] = endY;
    } else {
      mMinY[curCol] = mAgenda->timeToY( QTime(0,0) );
      mMaxY[curCol] = mAgenda->timeToY( QTime(23,59) );
    }
  } else {
    int startY = 0, endY = 0;
    if ( event ) {
      startY = mAgenda->timeToY( incidence->dtStart().time() );
      endY = mAgenda->timeToY( event->dtEnd().time() ) - 1;
    }
    if ( todo ) {
      QTime t = todo->dtDue().time();
      endY = mAgenda->timeToY( t ) - 1;
      startY = mAgenda->timeToY( t.addSecs( -1800 ) );
    }
    if ( endY < startY ) endY = startY;
    mAgenda->insertItem( incidence, curDate, curCol, startY, endY );
    if ( startY < mMinY[curCol] ) mMinY[curCol] = startY;
    if ( endY > mMaxY[curCol] ) mMaxY[curCol] = endY;
  }
}

void KOAgendaView::changeIncidenceDisplayAdded( Incidence *incidence )
{
  Todo *todo = dynamic_cast<Todo *>(incidence);
  if ( !calendar()->filter()->filterIncidence( incidence ) ||
     ( todo && !KOPrefs::instance()->showAllDayTodo() ) )
    return;

  QDate f = mSelectedDates.first();
  QDate l = mSelectedDates.last();
  QDate startDt = incidence->dtStart().date();

  if ( incidence->doesRecur() ) {
    DateList::ConstIterator dit;
    QDate curDate;
    for( dit = mSelectedDates.begin(); dit != mSelectedDates.end(); ++dit ) {
      curDate = *dit;
      if ( incidence->recursOn( curDate ) ) {
        insertIncidence( incidence, curDate );
      }
    }
    return;
  }

  QDate endDt;
  if ( incidence->type() == "Event" )
    endDt = (static_cast<Event *>(incidence))->dtEnd().date();
  if ( todo ) {
    bool overdue = (!todo->isCompleted()) &&
                   (todo->dtDue() < QDate::currentDate() );
    endDt = overdue ? QDate::currentDate()
                    : todo->dtDue().date();
    if ( endDt >= f && endDt <= l ) {
      insertIncidence( incidence, endDt );
      return;
    }
  }

  if ( startDt <= l ) {
    if ( startDt >= f ) {
      insertIncidence( incidence, startDt );
    } else if ( endDt >= f ) {
      insertIncidence( incidence, endDt );
    }
  }
}

void KOAgendaView::changeIncidenceDisplay( Incidence *incidence, int mode )
{
  switch ( mode ) {
    case KOGlobals::INCIDENCEADDED: {
        //  Add an event. No need to recreate the whole view!
        // recreating everything even causes troubles: dropping to the day matrix
        // recreates the agenda items, but the evaluation is still in an agendaItems' code,
        // which was deleted in the mean time. Thus KOrg crashes...
      changeIncidenceDisplayAdded( incidence );
      break;
    }
    case KOGlobals::INCIDENCEEDITED: {
      if ( !mAllowAgendaUpdate ) {
        updateEventIndicators();
      } else {
        removeIncidence( incidence );
        updateEventIndicators();
        changeIncidenceDisplayAdded( incidence );
      }
      break;
    }
    case KOGlobals::INCIDENCEDELETED: {
      mAgenda->removeIncidence( incidence );
      mAllDayAgenda->removeIncidence( incidence );
      updateEventIndicators();
      break;
    }
    default:
      updateView();
  }
}

void KOAgendaView::fillAgenda( const QDate & )
{
  fillAgenda();
}

void KOAgendaView::fillAgenda()
{
  /* Remember the uids of the selected items. In case one of the
   * items was deleted and re-added, we want to reselect it. */
  const QString &selectedAgendaUid = 
    mAgenda->selectedIncidence()? mAgenda->selectedIncidence()->uid():QString::null;
  const QString &selectedAllDayAgendaUid = 
    mAllDayAgenda->selectedIncidence()? mAllDayAgenda->selectedIncidence()->uid():QString::null;
  
  enableAgendaUpdate( true );
  clearView();

  mAllDayAgenda->changeColumns(mSelectedDates.count());
  mAgenda->changeColumns(mSelectedDates.count());
  mEventIndicatorTop->changeColumns(mSelectedDates.count());
  mEventIndicatorBottom->changeColumns(mSelectedDates.count());

  createDayLabels();
  setHolidayMasks();

  mMinY.resize(mSelectedDates.count());
  mMaxY.resize(mSelectedDates.count());

  Event::List dayEvents;

  // ToDo items shall be displayed for the day they are due, but only shown today if they are already overdue.
  // Therefore, get all of them.
  Todo::List todos  = calendar()->todos();

  mAgenda->setDateList(mSelectedDates);

  QDate today = QDate::currentDate();

  bool somethingReselected = false;
  DateList::ConstIterator dit;
  int curCol = 0;
  for( dit = mSelectedDates.begin(); dit != mSelectedDates.end(); ++dit ) {
    QDate currentDate = *dit;
//    kdDebug(5850) << "KOAgendaView::fillAgenda(): " << currentDate.toString()
//              << endl;

    dayEvents = calendar()->events(currentDate,true);

    // Default values, which can never be reached
    mMinY[curCol] = mAgenda->timeToY(QTime(23,59)) + 1;
    mMaxY[curCol] = mAgenda->timeToY(QTime(0,0)) - 1;

    unsigned int numEvent;
    for(numEvent=0;numEvent<dayEvents.count();++numEvent) {
      Event *event = *dayEvents.at(numEvent);
//      kdDebug(5850) << " Event: " << event->summary() << endl;
      insertIncidence( event, currentDate, curCol );
      if( event->uid() == selectedAgendaUid && !selectedAgendaUid.isEmpty() ) {
        mAgenda->selectItemByUID( event->uid() );
        somethingReselected = true;
      }
      if( event->uid() == selectedAllDayAgendaUid && !selectedAllDayAgendaUid.isEmpty() ) {
        mAllDayAgenda->selectItemByUID( event->uid() );
        somethingReselected = true;
      }

    }
//    if (numEvent == 0) kdDebug(5850) << " No events" << endl;


    // ---------- [display Todos --------------
    if ( KOPrefs::instance()->showAllDayTodo() ) {
      unsigned int numTodo;
      for (numTodo = 0; numTodo < todos.count(); ++numTodo) {
        Todo *todo = *todos.at(numTodo);

        if ( ! todo->hasDueDate() ) continue;  // todo shall not be displayed if it has no date

        // ToDo items shall be displayed for the day they are due, but only showed today if they are already overdue.
        // Already completed items can be displayed on their original due date
        bool overdue = (!todo->isCompleted()) && (todo->dtDue() < today);

        if ( (( todo->dtDue().date() == currentDate) && !overdue) ||
             (( currentDate == today) && overdue) ||
             ( todo->recursOn( currentDate ) ) ) {
          if ( todo->doesFloat() || overdue ) {  // Todo has no due-time set or is already overdue
            //kdDebug(5850) << "todo without time:" << todo->dtDueDateStr() << ";" << todo->summary() << endl;

            mAllDayAgenda->insertAllDayItem(todo, currentDate, curCol, curCol);
          } else {
            //kdDebug(5850) << "todo with time:" << todo->dtDueStr() << ";" << todo->summary() << endl;

            int endY = mAgenda->timeToY(todo->dtDue().time()) - 1;
            int startY = endY - 1;

            mAgenda->insertItem(todo,currentDate,curCol,startY,endY);

            if (startY < mMinY[curCol]) mMinY[curCol] = startY;
            if (endY > mMaxY[curCol]) mMaxY[curCol] = endY;
          }
        }
      }
    }
    // ---------- display Todos] --------------

    ++curCol;
  }

  mAgenda->checkScrollBoundaries();

//  mAgenda->viewport()->update();
//  mAllDayAgenda->viewport()->update();

// make invalid
  deleteSelectedDateTime();

  if( !somethingReselected ) {
    emit incidenceSelected( 0 );
  }

//  kdDebug(5850) << "Fill Agenda done" << endl;
}

void KOAgendaView::clearView()
{
//  kdDebug(5850) << "ClearView" << endl;
  mAllDayAgenda->clear();
  mAgenda->clear();
}

CalPrinter::PrintType KOAgendaView::printType()
{
  if ( currentDateCount() == 1 ) return CalPrinter::Day;
  else return CalPrinter::Week;
}

void KOAgendaView::newEvent( const QPoint &pos)
{
  if (!mSelectedDates.count()) return;

  QDate day = mSelectedDates[pos.x()];

  QTime time = mAgenda->gyToTime(pos.y());
  QDateTime dt(day,time);

  emit newEventSignal(dt);
}

void KOAgendaView::newEvent(const QPoint &start, const QPoint &end)
{
  if (!mSelectedDates.count()) return;

  QDate dayStart = mSelectedDates[start.x()];
  QDate dayEnd = mSelectedDates[end.x()];

  QTime timeStart = mAgenda->gyToTime( start.y() );
  QTime timeEnd = mAgenda->gyToTime( end.y() + 1 );

  QDateTime dtStart(dayStart,timeStart);
  QDateTime dtEnd(dayEnd,timeEnd);

  emit newEventSignal(dtStart,dtEnd);
}

void KOAgendaView::newEventAllDay( const QPoint &start )
{
  if (!mSelectedDates.count()) return;

  QDate day = mSelectedDates[start.x()];

  emit newEventSignal(day);
}

void KOAgendaView::updateEventIndicatorTop(int newY)
{
  uint i;
  for(i=0;i<mMinY.size();++i) {
    if (newY >= mMinY[i]) mEventIndicatorTop->enableColumn(i,true);
    else mEventIndicatorTop->enableColumn(i,false);
  }

  mEventIndicatorTop->update();
}

void KOAgendaView::updateEventIndicatorBottom(int newY)
{
  uint i;
  for(i=0;i<mMaxY.size();++i) {
    if (newY <= mMaxY[i]) mEventIndicatorBottom->enableColumn(i,true);
    else mEventIndicatorBottom->enableColumn(i,false);
  }

  mEventIndicatorBottom->update();
}

void KOAgendaView::slotTodoDropped( Todo *todo, const QPoint &gpos, bool allDay )
{
  if ( gpos.x()<0 || gpos.y()<0 ) return;
  QDate day = mSelectedDates[gpos.x()];
  QTime time = mAgenda->gyToTime( gpos.y() );
  QDateTime newTime( day, time );

  if ( todo ) {
    Todo *existingTodo = calendar()->todo(todo->uid());
    if ( existingTodo ) {
      kdDebug(5850) << "Drop existing Todo" << endl;
      Todo *oldTodo = existingTodo->clone();
      if ( mChanger && mChanger->beginChange( existingTodo ) ) {
        existingTodo->setDtDue( newTime );
        existingTodo->setFloats( allDay );
        existingTodo->setHasDueDate( true );
        mChanger->changeIncidence( oldTodo, existingTodo );
        mChanger->endChange( existingTodo );
      } else {
        KMessageBox::sorry( this, i18n("Unable to modify this to-do item, "
                            "because it cannot be locked.") );
      }
      delete oldTodo;
    } else {
      kdDebug(5850) << "Drop new Todo" << endl;
      todo->setDtDue( newTime );
      todo->setFloats( allDay );
      existingTodo->setHasDueDate( true );
      if ( !mChanger->addIncidence( todo ) ) {
        KODialogManager::errorSaveIncidence( this, todo );
      }
    }
  }
}

void KOAgendaView::startDrag( Incidence *incidence )
{
#ifndef KORG_NODND
  DndFactory factory( calendar() );
  ICalDrag *vd = factory.createDrag( incidence, this );
  if ( vd->drag() ) {
    kdDebug(5850) << "KOAgendaView::startDrag(): Delete drag source" << endl;
  }
#endif
}

void KOAgendaView::readSettings()
{
  readSettings(KOGlobals::self()->config());
}

void KOAgendaView::readSettings(KConfig *config)
{
//  kdDebug(5850) << "KOAgendaView::readSettings()" << endl;

  config->setGroup("Views");

#ifndef KORG_NOSPLITTER
  QValueList<int> sizes = config->readIntListEntry("Separator AgendaView");
  if (sizes.count() == 2) {
    mSplitterAgenda->setSizes(sizes);
  }
#endif

  updateConfig();
}

void KOAgendaView::writeSettings(KConfig *config)
{
//  kdDebug(5850) << "KOAgendaView::writeSettings()" << endl;

  config->setGroup("Views");

#ifndef KORG_NOSPLITTER
  QValueList<int> list = mSplitterAgenda->sizes();
  config->writeEntry("Separator AgendaView",list);
#endif
}

void KOAgendaView::setHolidayMasks()
{
  mHolidayMask.resize( mSelectedDates.count() + 1 );

  for( uint i = 0; i < mSelectedDates.count(); ++i ) {
    mHolidayMask[i] = !KOCore::self()->isWorkDay( mSelectedDates[ i ] );
  }

  // Store the information about the day before the visible area (needed for
  // overnight working hours) in the last bit of the mask:
  bool showDay = !KOCore::self()->isWorkDay( mSelectedDates[ 0 ].addDays( -1 ) );
  mHolidayMask[ mSelectedDates.count() ] = showDay;

  mAgenda->setHolidayMask( &mHolidayMask );
  mAllDayAgenda->setHolidayMask( &mHolidayMask );
}

void KOAgendaView::setContentsPos( int y )
{
  mAgenda->setContentsPos( 0, y );
}

void KOAgendaView::setExpandedButton( bool expanded )
{
  if ( !mExpandButton ) return;

  if ( expanded ) {
    mExpandButton->setPixmap( mExpandedPixmap );
  } else {
    mExpandButton->setPixmap( mNotExpandedPixmap );
  }
}

void KOAgendaView::clearSelection()
{
  mAgenda->deselectItem();
  mAllDayAgenda->deselectItem();
}

void KOAgendaView::newTimeSpanSelectedAllDay( const QPoint &start, const QPoint &end )
{
  newTimeSpanSelected( start, end );
  mTimeSpanInAllDay = true;
}

void KOAgendaView::newTimeSpanSelected( const QPoint &start, const QPoint &end )
{
  if (!mSelectedDates.count()) return;

  mTimeSpanInAllDay = false;

  QDate dayStart = mSelectedDates[start.x()];
  QDate dayEnd = mSelectedDates[end.x()];

  QTime timeStart = mAgenda->gyToTime(start.y());
  QTime timeEnd = mAgenda->gyToTime( end.y() + 1 );

  QDateTime dtStart(dayStart,timeStart);
  QDateTime dtEnd(dayEnd,timeEnd);

  mTimeSpanBegin = dtStart;
  mTimeSpanEnd = dtEnd;
}

void KOAgendaView::deleteSelectedDateTime()
{
  mTimeSpanBegin.setDate(QDate());
  mTimeSpanEnd.setDate(QDate());
  mTimeSpanInAllDay = false;
}

void KOAgendaView::setTypeAheadReceiver( QObject *o )
{
  mAgenda->setTypeAheadReceiver( o );
  mAllDayAgenda->setTypeAheadReceiver( o );
}

void KOAgendaView::finishTypeAhead()
{
  mAgenda->finishTypeAhead();
  mAllDayAgenda->finishTypeAhead();
}

void KOAgendaView::removeIncidence( Incidence *incidence )
{
  mAgenda->removeIncidence( incidence );
  mAllDayAgenda->removeIncidence( incidence );
}

void KOAgendaView::updateEventIndicators()
{
  mMinY = mAgenda->minContentsY();
  mMaxY = mAgenda->maxContentsY();

  mAgenda->checkScrollBoundaries();
}

void KOAgendaView::setIncidenceChanger( IncidenceChangerBase *changer ) 
{ 
  mChanger = changer;
  mAgenda->setIncidenceChanger( changer );
  mAllDayAgenda->setIncidenceChanger( changer );
}
