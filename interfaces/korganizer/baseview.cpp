/*
  This file is part of the KOrganizer interfaces.

  Copyright (c) 1999,2001,2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2004           Reinhold Kainhofer   <reinhold@kainhofer.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "baseview.h"

#include <akonadi/kcal/akonadicalendar.h>
#include <akonadi/kcal/calendarsearch.h>
#include <akonadi/kcal/collectionselection.h>
#include <akonadi/kcal/collectionselectionproxymodel.h>
#include <akonadi/kcal/entitymodelstatesaver.h>
#include <akonadi/kcal/utils.h>

#include <QItemSelectionModel>

#include <KConfigGroup>
#include <KRandom>

using namespace Akonadi;
using namespace KOrg;

CollectionSelection* BaseView::sGlobalCollectionSelection = 0;

void BaseView::setGlobalCollectionSelection( CollectionSelection* s )
{
  sGlobalCollectionSelection = s;
}

CollectionSelection* BaseView::globalCollectionSelection()
{
  return sGlobalCollectionSelection;
}

class BaseView::Private {
  BaseView *const q;
public:
  explicit Private( BaseView* qq )
    : q( qq )
    , calendar( 0 )
    , customCollectionSelection( 0 )
    , collectionSelectionModel( 0 )
    , stateSaver( 0 ) {
    QByteArray cname = q->metaObject()->className();
    cname.replace( ":", "_" );
    identifier = cname + "_" + KRandom::randomString( 8 ).toLatin1();
    calendarSearch = new CalendarSearch( q );
    connect( calendarSearch->model(), SIGNAL(rowsInserted(QModelIndex,int,int)), q, SLOT(rowsInserted(QModelIndex,int,int)) );
    connect( calendarSearch->model(), SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), q, SLOT(rowsAboutToBeRemoved(QModelIndex,int,int)) );
    connect( calendarSearch->model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)), q, SLOT(dataChanged(QModelIndex,QModelIndex)) );
    connect( calendarSearch->model(), SIGNAL(modelReset()), q, SLOT(calendarReset()) );
  }

  ~Private() {
    delete collectionSelectionModel;
  }

  AkonadiCalendar *calendar;
  CalendarSearch *calendarSearch;
  CollectionSelection *customCollectionSelection;
  CollectionSelectionProxyModel* collectionSelectionModel;
  EntityModelStateSaver* stateSaver;
  QByteArray identifier;
  void setUpModels();
  void reconnectCollectionSelection();
};

void BaseView::Private::setUpModels()
{
  delete stateSaver;
  stateSaver = 0;
  delete customCollectionSelection;
  customCollectionSelection = 0;
  if ( collectionSelectionModel ) {
    customCollectionSelection = new CollectionSelection( collectionSelectionModel->selectionModel() );
    stateSaver = new EntityModelStateSaver( collectionSelectionModel, q );
    stateSaver->addRole( Qt::CheckStateRole, "CheckState" );
    calendarSearch->setSelectionModel( collectionSelectionModel->selectionModel() );
  } else {
    calendarSearch->setSelectionModel( 0 );
  }
  reconnectCollectionSelection();
}

void BaseView::Private::reconnectCollectionSelection()
{
  if ( q->globalCollectionSelection() )
    q->globalCollectionSelection()->disconnect( q );
  if ( customCollectionSelection )
    customCollectionSelection->disconnect( q );
  QObject::connect( q->collectionSelection(), SIGNAL(selectionChanged(Akonadi::Collection::List,Akonadi::Collection::List)), q, SLOT(collectionSelectionChanged()) );
}


BaseView::BaseView( QWidget *parent )
  : QWidget( parent ), mChanger( 0 ), d( new Private( this ) )
{}

BaseView::~BaseView()
{
  delete d;
}

void BaseView::setCalendar( AkonadiCalendar *cal )
{
  if ( d->calendar == cal )
    return;
  d->calendar = cal;
  if ( cal && d->collectionSelectionModel )
    d->collectionSelectionModel->setSourceModel( cal->model() );
}

CalPrinterBase::PrintType BaseView::printType()
{
  return CalPrinterBase::Month;
}

AkonadiCalendar *BaseView::calendar()
{
  return d->calendar;
}

Akonadi::CalendarSearch* BaseView::calendarSearch() const
{
  return d->calendarSearch;
}

bool BaseView::isEventView()
{
  return false;
}

void BaseView::dayPassed( const QDate & )
{
  updateView();
}

void BaseView::setIncidenceChanger( IncidenceChangerBase *changer )
{
  mChanger = changer;
}

void BaseView::flushView()
{}

BaseView* BaseView::viewAt( const QPoint & )
{
  return this;
}

void BaseView::updateConfig()
{}

bool BaseView::hasConfigurationDialog() const
{
  return false;
}

void BaseView::setDateRange( const QDate& start, const QDate& end )
{
  showDates( start, end );
  d->calendarSearch->setStartDate( KDateTime( start ) );
  d->calendarSearch->setEndDate( KDateTime( end ) );
}

void BaseView::showConfigurationDialog( QWidget* )
{
}

QByteArray BaseView::identifier() const
{
  return d->identifier;
}

void BaseView::setIdentifier( const QByteArray& identifier )
{
  d->identifier = identifier;
}

void BaseView::restoreConfig( const KConfigGroup &configGroup )
{
  const bool useCustom = configGroup.readEntry( "UseCustomCollectionSelection", false );
  if ( !d->collectionSelectionModel && !useCustom ) {
    delete d->collectionSelectionModel;
    d->collectionSelectionModel = 0;
    d->setUpModels();
  } else if ( useCustom ) {

    if ( !d->collectionSelectionModel )
    {
      d->collectionSelectionModel = new CollectionSelectionProxyModel( this );
      d->collectionSelectionModel->setDynamicSortFilter( true );
      d->collectionSelectionModel->setSortCaseSensitivity( Qt::CaseInsensitive );
      if ( d->calendar )
        d->collectionSelectionModel->setSourceModel( d->calendar->treeModel() );
      d->setUpModels();
    }

    const KConfigGroup selectionGroup = configGroup.config()->group( configGroup.name() + QLatin1String("_selectionSetup") );
    d->stateSaver->restoreConfig( selectionGroup );
  }
  doRestoreConfig( configGroup );
}

void BaseView::saveConfig( KConfigGroup &configGroup )
{
  configGroup.writeEntry( "UseCustomCollectionSelection", d->collectionSelectionModel != 0 );
  if ( d->stateSaver )
  {
    KConfigGroup selectionGroup = configGroup.config()->group( configGroup.name() + QLatin1String("_selectionSetup") );
    d->stateSaver->saveConfig( selectionGroup );
  }
  doSaveConfig( configGroup );
}

void BaseView::doRestoreConfig( const KConfigGroup & )
{}

void BaseView::doSaveConfig( KConfigGroup & )
{}

CollectionSelection* BaseView::collectionSelection() const
{
  return d->customCollectionSelection ? d->customCollectionSelection : globalCollectionSelection();
}

void BaseView::setCustomCollectionSelectionProxyModel( Akonadi::CollectionSelectionProxyModel* model )
{
  if ( d->collectionSelectionModel == model )
    return;
  delete d->collectionSelectionModel;
  d->collectionSelectionModel = model;
  d->setUpModels();
}

void BaseView::collectionSelectionChanged()
{

}

CollectionSelectionProxyModel *BaseView::customCollectionSelectionProxyModel() const
{
  return d->collectionSelectionModel;
}

CollectionSelectionProxyModel *BaseView::takeCustomCollectionSelectionProxyModel()
{
  CollectionSelectionProxyModel* m = d->collectionSelectionModel;
  d->collectionSelectionModel = 0;
  d->setUpModels();
  return m;
}

CollectionSelection *BaseView::customCollectionSelection() const
{
  return d->customCollectionSelection;
}

void BaseView::clearSelection()
{}

bool BaseView::eventDurationHint( QDateTime &startDt, QDateTime &endDt, bool &allDay )
{
  Q_UNUSED( startDt );
  Q_UNUSED( endDt );
  Q_UNUSED( allDay );
  return false;
}

void BaseView::getHighlightMode( bool &highlightEvents,
                                 bool &highlightTodos,
                                 bool &highlightJournals ) {
  highlightEvents   = true;
  highlightTodos    = false;
  highlightJournals = false;
}

void BaseView::handleBackendError( const QString &errorString )
{
}

void BaseView::backendErrorOccurred()
{
  handleBackendError( d->calendarSearch->errorString() );
}

bool BaseView::usesFullWindow()
{
  return false;
}
void BaseView::incidencesAdded( const Akonadi::Item::List & )
{
}

void BaseView::incidencesAboutToBeRemoved( const Akonadi::Item::List & )
{
}

void BaseView::incidencesChanged( const Akonadi::Item::List& )
{
}

void BaseView::calendarReset()
{
}

void BaseView::dataChanged( const QModelIndex& topLeft, const QModelIndex& bottomRight )
{
  incidencesChanged( Akonadi::itemsFromModel( d->calendarSearch->model(), topLeft.row(), bottomRight.row() ) );
}

void BaseView::rowsInserted( const QModelIndex& parent, int start, int end )
{
  Q_UNUSED( parent );
  incidencesAdded( Akonadi::itemsFromModel( d->calendarSearch->model(), start, end ) );
}

void BaseView::rowsAboutToBeRemoved( const QModelIndex& parent, int start, int end )
{
  Q_UNUSED( parent );
  incidencesAboutToBeRemoved( Akonadi::itemsFromModel( d->calendarSearch->model(), start, end ) );
}

#include "baseview.moc"
