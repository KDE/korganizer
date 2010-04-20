/*
  Copyright (c) 2007 Volker Krause <vkrause@kde.org>

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
*/

#ifndef KORG_MULTIAGENDAVIEW_H_H
#define KORG_MULTIAGENDAVIEW_H_H

#include "views/agendaview/agendaview.h"

#include <Akonadi/Item>

#include <KDialog>

class KOAgendaView;
class TimeLabelsZone;

class KHBox;

class Q3ScrollView;
class QAbstractItemModel;
class QModelIndex;
class QResizeEvent;
class QScrollBar;
class QSplitter;

namespace Akonadi {
  class CollectionSelectionProxyModel;
}

namespace KOrg {

class MultiAgendaViewConfigDialog : public KDialog
{
  Q_OBJECT
  public:
    explicit MultiAgendaViewConfigDialog( QAbstractItemModel *baseModel, QWidget *parent=0 );
    ~MultiAgendaViewConfigDialog();

    bool useCustomColumns() const;
    void setUseCustomColumns( bool );

    int numberOfColumns() const;
    void setNumberOfColumns( int n );

    QString columnTitle( int column ) const;
    void setColumnTitle( int column, const QString &title );
    Akonadi::CollectionSelectionProxyModel *takeSelectionModel( int column );
    void setSelectionModel( int column, Akonadi::CollectionSelectionProxyModel *model );

  public Q_SLOTS:
    /**
     * reimplemented from QDialog
     */
    void accept();

  private Q_SLOTS:
    void useCustomToggled( bool );
    void numberOfColumnsChanged( int );
    void currentChanged( const QModelIndex &index );
    void titleEdited( const QString &text );

  private:
    class Private;
    Private *const d;
};

/**
  Shows one agenda for every resource side-by-side.
*/
class MultiAgendaView : public AgendaView
{
  Q_OBJECT
  public:
    explicit MultiAgendaView( QWidget *parent = 0 );
    ~MultiAgendaView();

    Akonadi::Item::List selectedIncidences();
    DateList selectedIncidenceDates();
    int currentDateCount();
    int maxDatesHint();

    bool eventDurationHint( QDateTime &startDt, QDateTime &endDt, bool &allDay );
    /* reimp */void setCalendar( Akonadi::Calendar *cal );

    /**
     * reimplemented from KOrg::BaseView
     */
    bool hasConfigurationDialog() const;

    /**
     * reimplemented from KOrg::BaseView
     */
    void showConfigurationDialog( QWidget *parent );

  public slots:
    void showDates( const QDate &start, const QDate &end );
    void showIncidences( const Akonadi::Item::List &incidenceList, const QDate &date );
    void updateView();
    void changeIncidenceDisplay( const Akonadi::Item &, int mode );
    void updateConfig();

    void setIncidenceChanger( Akonadi::IncidenceChanger *changer );

    void setUpdateNeeded();

  protected:
    void resizeEvent( QResizeEvent *event );
    void showEvent( QShowEvent *event );

    /* reimp */void doRestoreConfig( const KConfigGroup &configGroup );
    /* reimp */void doSaveConfig( KConfigGroup &configGroup );

  protected Q_SLOTS:
    /**
     * Reimplemented from KOrg::BaseView
     */
    void collectionSelectionChanged();

  private:
    void addView( const Akonadi::Collection &collection );
    void addView( Akonadi::CollectionSelectionProxyModel *selectionProxy, const QString &title );
    KOAgendaView *createView( const QString &title );

    void deleteViews();
    void setupViews();
    void resizeScrollView( const QSize &size );

  private slots:
    void slotSelectionChanged();
    void slotClearTimeSpanSelection();
    void resizeSplitters();
    void setupScrollBar();
    void zoomView( const int delta, const QPoint &pos, const Qt::Orientation ori );
    void slotResizeScrollView();
    void recreateViews();

  private:
    QList<KOAgendaView*> mAgendaViews;
    QList<QWidget*> mAgendaWidgets;
    KHBox *mTopBox;
    Q3ScrollView *mScrollView;
    TimeLabelsZone *mTimeLabelsZone;
    QSplitter *mLeftSplitter, *mRightSplitter;
    QScrollBar *mScrollBar;
    QWidget *mLeftBottomSpacer, *mRightBottomSpacer;
    QDate mStartDate, mEndDate;
    bool mUpdateOnShow;
    bool mPendingChanges;
    bool mCustomColumnSetupUsed;
    QVector<Akonadi::CollectionSelectionProxyModel*> mCollectionSelectionModels;
    QVector<QString> mCustomColumnTitles;
    int mCustomNumberOfColumns;
};

}

#endif
