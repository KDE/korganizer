/*
  This file is part of KOrganizer.

  Copyright (c) 2008 Bruno Virlet <bruno.virlet@gmail.com>

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

#ifndef VIEW_H
#define VIEW_H

#include "koeventview.h"

#include <Akonadi/Item>

#include <QtCore/QTimer>

class KOEventPopupMenu;

class QWheelEvent;
class QKeyEvent;

namespace KOrg {

class MonthGraphicsView;
class MonthScene;

/**
  New month view.
*/
class MonthView : public KOEventView
{
  Q_OBJECT
  public:
    explicit MonthView( QWidget *parent = 0 );
    ~MonthView();

    virtual int currentDateCount() const;
    int currentMonth() const;

    Akonadi::Item::List selectedIncidences();

    /** Returns dates of the currently selected events */
    virtual KCalCore::DateList selectedIncidenceDates();

    virtual QDateTime selectionStart();

    virtual QDateTime selectionEnd();

    virtual bool eventDurationHint( QDateTime &startDt, QDateTime &endDt, bool &allDay );

    /**
     * Returns the average date in the view
     */
    QDate averageDate() const;

    bool usesFullWindow();

    bool supportsDateRangeSelection() { return false; }

    virtual CalPrinterBase::PrintType printType() const;

  public slots:
    virtual void updateView();

    virtual void showIncidences( const Akonadi::Item::List &incidenceList, const QDate &date );

    void changeIncidenceDisplay( const Akonadi::Item &, int );

    /* reimp */void updateConfig();

  protected slots:
    void moveBackMonth();
    void moveBackWeek();
    void moveFwdWeek();
    void moveFwdMonth();
    /* reimp */void calendarReset();

  protected:
    int maxDatesHint() const;

    virtual void wheelEvent( QWheelEvent *event );
    virtual void keyPressEvent( QKeyEvent *event );
    virtual void keyReleaseEvent( QKeyEvent *event );

    /* reimp */void incidencesAdded( const Akonadi::Item::List &incidences );
    /* reimp */void incidencesAboutToBeRemoved( const Akonadi::Item::List &incidences );
    /* reimp */void incidencesChanged( const Akonadi::Item::List &incidences );
    /* reimp */QPair<KDateTime,KDateTime> actualDateRange( const KDateTime &start,
                                                           const KDateTime &end ) const;

    /**
     * @deprecated
     */
    void showDates( const QDate &start, const QDate &end );

  private slots:
    // Compute and update the whole view
    void reloadIncidences();

  private:
    void addIncidence( const Akonadi::Item &incidence );
    void moveStartDate( int weeks, int months );
    void triggerDelayedReload() {
      if ( !mReloadTimer.isActive() ) {
        mReloadTimer.start( 50 );
      }
    }

    MonthGraphicsView *mView;
    MonthScene *mScene;
    Akonadi::Item::Id mSelectedItemId;
    QDate mSelectedItemDate;

    QTimer mReloadTimer;

    KOEventPopupMenu *mViewPopup;

    friend class MonthScene;
    friend class MonthGraphicsView;
};

}

#endif
