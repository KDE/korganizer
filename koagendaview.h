/*
    This file is part of KOrganizer.

    Copyright (c) 2000,2001,2003 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KOAGENDAVIEW_H
#define KOAGENDAVIEW_H

#include <qscrollview.h>
#include <qlabel.h>

#include "calprinter.h"
#include "koeventview.h"

class QHBox;
class QPushButton;

class KOAgenda;
class KOAgendaItem;
class KConfig;

class TimeLabels : public QScrollView
{
    Q_OBJECT
  public:
    TimeLabels( int rows, QWidget *parent = 0, const char *name = 0,
                WFlags f = 0 );

    void setCellHeight( int height );

    /** Calculates the minimum width */
    virtual int minimumWidth() const;

    /** updates widget's internal state */
    void updateConfig();

    /**  */
    void setAgenda( KOAgenda *agenda );

    /**  */
    virtual void paintEvent( QPaintEvent *e );

  public slots:
    /** update time label positions */
    void positionChanged();

  protected:
    void drawContents( QPainter *p, int cx, int cy, int cw, int ch );

  private:
    int mRows;
    int mCellHeight;

    KOAgenda* mAgenda;
};

class EventIndicator : public QFrame
{
    Q_OBJECT
  public:
    enum Location { Top, Bottom };
    EventIndicator( Location loc = Top, QWidget *parent = 0,
                    const char *name = 0 );
    virtual ~EventIndicator();

    void changeColumns( int columns );

    void enableColumn( int column, bool enable );

  protected:
    void drawContents( QPainter * );

  private:
    int mColumns;
    QHBox *mTopBox;
    QBoxLayout *mTopLayout;
    Location mLocation;
    QPixmap mPixmap;
    QMemArray<bool> mEnabled;
};

class KOAlternateLabel : public QLabel
{
    Q_OBJECT
  public:
    KOAlternateLabel( QString shortlabel, QString longlabel,
                      QString extensivelabel = QString::null,
                      QWidget *parent = 0, const char *name = 0 );
    ~KOAlternateLabel();

    virtual QSize minimumSizeHint() const;

  public slots:
    void setText( const QString & );
    void useShortText();
    void useLongText();
    void useExtensiveText();
    void useDefaultText();

  protected:
    virtual void resizeEvent( QResizeEvent * );
    virtual void squeezeTextToLabel();
    bool mTextTypeFixed;
    QString mShortText, mLongText, mExtensiveText;
};

/**
  KOAgendaView is the agenda-like view used to display events in an one or
  multi-day view.
*/
class KOAgendaView : public KOEventView
{
    Q_OBJECT
  public:
    KOAgendaView( Calendar *cal, QWidget *parent = 0, const char *name = 0 );
    virtual ~KOAgendaView();

    

    /** Returns maximum number of days supported by the koagendaview */
    virtual int maxDatesHint();

    /** Returns number of currently shown dates. */
    virtual int currentDateCount();

    /** returns the currently selected events */
    virtual Incidence::List selectedIncidences();

    /** returns the currently selected events */
    virtual DateList selectedDates();

    /** return the default start/end date/time for new events   */
    virtual bool eventDurationHint(QDateTime &startDt, QDateTime &endDt, bool &allDay);

    /** Remove all events from view */
    void clearView();

    CalPrinter::PrintType printType();

    /** start-datetime of selection */
    QDateTime selectionStart() { return mTimeSpanBegin; }
    /** end-datetime of selection */
    QDateTime selectionEnd() { return mTimeSpanEnd; }
    /** returns true if selection is for whole day */
    bool selectedIsAllDay() { return mTimeSpanInAllDay; }
    /** make selected start/end invalid */
    void deleteSelectedDateTime();
    /** returns if only a single cell is selected, or a range of cells */
    bool selectedIsSingleCell();

    void setTypeAheadReceiver( QObject * );

  public slots:
    virtual void updateView();
    virtual void updateConfig();
    virtual void showDates( const QDate &start, const QDate &end );
    virtual void showIncidences( const Incidence::List &incidenceList );

    void insertIncidence( Incidence *incidence, QDate curDate, int curCol = -1 );
    void changeIncidenceDisplayAdded( Incidence *incidence );
    void changeIncidenceDisplay( Incidence *incidence, int mode );

    void clearSelection();

    void newEvent( const QPoint &pos );
    void newEvent( const QPoint &start, const QPoint &end );
    void newEventAllDay( const QPoint &pos );

    void startDrag( Incidence * );

    void readSettings();
    void readSettings( KConfig * );
    void writeSettings( KConfig * );

    void setContentsPos( int y );

    void setExpandedButton( bool expanded );

    void finishTypeAhead();

    /** reschedule the todo  to the given x- and y- coordinates. Third parameter determines all-day (no time specified) */
    void slotTodoDropped( Todo *, const QPoint &, bool );

    void enableAgendaUpdate( bool enable );
    void setIncidenceChanger( IncidenceChangerBase *changer );

    void zoomInHorizontally( );
    void zoomInHorizontally( const QDate& );
    
    void zoomOutHorizontally( );
    void zoomOutHorizontally( const QDate& );
    
    void zoomInVertically( );
    void zoomOutVertically( );
    
    void zoomView( const int delta, const QPoint &pos,
      const Qt::Orientation orient=Qt::Horizontal );
  signals:
    void toggleExpand();

  protected:
    /** Fill agenda beginning with date startDate */
    void fillAgenda( const QDate &startDate );

    /** Fill agenda using the current set value for the start date */
    void fillAgenda();

    void connectAgenda( KOAgenda*agenda, QPopupMenu*popup, KOAgenda* otherAgenda );

    /** Create labels for the selected dates. */
    void createDayLabels();

    /**
      Set the masks on the agenda widgets indicating, which days are holidays.
    */
    void setHolidayMasks();

    void removeIncidence( Incidence * );
    /**
      Updates the event indicators after a certain incidence was modified or
      removed.
    */
    void updateEventIndicators();

    void updateTimeBarWidth();

  protected slots:
    /** Update event belonging to agenda item */
    void updateEventDates( KOAgendaItem *item );
    /** update just the display of the given incidence, called by a single-shot timer */
    void doUpdateItem();

    void updateEventIndicatorTop( int newY );
    void updateEventIndicatorBottom( int newY );

    /** Updates data for selected timespan */
    void newTimeSpanSelected( const QPoint &start, const QPoint &end );
    /** Updates data for selected timespan for all day event*/
    void newTimeSpanSelectedAllDay( const QPoint &start, const QPoint &end );

  private:
    // view widgets
    QFrame *mDayLabels;
    QHBox *mDayLabelsFrame;
    QBoxLayout *mLayoutDayLabels;
    QFrame *mAllDayFrame;
    KOAgenda *mAllDayAgenda;
    KOAgenda *mAgenda;
    TimeLabels *mTimeLabels;
    QWidget *mDummyAllDayLeft;
    QSplitter *mSplitterAgenda;
    QPushButton *mExpandButton;

    DateList mSelectedDates;  // List of dates to be displayed
    int mViewType;

    KOEventPopupMenu *mAgendaPopup;
    KOEventPopupMenu *mAllDayAgendaPopup;

    EventIndicator *mEventIndicatorTop;
    EventIndicator *mEventIndicatorBottom;

    QMemArray<int> mMinY;
    QMemArray<int> mMaxY;

    QMemArray<bool> mHolidayMask;

    QPixmap mExpandedPixmap;
    QPixmap mNotExpandedPixmap;

    QDateTime mTimeSpanBegin;
    QDateTime mTimeSpanEnd;
    bool mTimeSpanInAllDay;
    bool mAllowAgendaUpdate;

    Incidence *mUpdateItem;
};

#endif
