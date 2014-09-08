/*
  This file is part of KOrganizer.

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

#include "multiagendaview.h"
#include "akonadicollectionview.h"
#include "koeventpopupmenu.h"
#include "koprefs.h"
#include "ui_multiagendaviewconfigwidget.h"

#include <akonadi_next/kcolumnfilterproxymodel.h>

#include <calendarviews/agenda/agendaview.h>
#include <calendarviews/multiagenda/multiagendaview.h>

#include <AkonadiWidgets/EntityTreeView>
#include <AkonadiCore/EntityTreeModel>

#include <KCheckableProxyModel>

#include <QDialogButtonBox>
#include <QPushButton>

#include <QHBoxLayout>
#include <QSortFilterProxyModel>
#include <QStandardItem>
#include <KConfigGroup>

using namespace Future;
using namespace KOrg;

static QString generateColumnLabel( int c )
{
  return i18n( "Agenda %1", c + 1 );
}

class MultiAgendaView::Private
{
  public:
    Private( MultiAgendaView *qq ) : q( qq )
    {
      QHBoxLayout *layout = new QHBoxLayout( q );
      mMultiAgendaView = new EventViews::MultiAgendaView( q );
      mMultiAgendaView->setPreferences( KOPrefs::instance()->eventViewsPreferences() );
      layout->addWidget( mMultiAgendaView );

      mPopup = q->eventPopup();
    }

    EventViews::MultiAgendaView *mMultiAgendaView;
    KOEventPopupMenu *mPopup;

  private:
    MultiAgendaView * const q;
};

MultiAgendaView::MultiAgendaView( QWidget *parent )
  : KOEventView( parent ), d( new Private( this ) )
{
  connect( d->mMultiAgendaView, SIGNAL(datesSelected(KCalCore::DateList)),
           SIGNAL(datesSelected(KCalCore::DateList)) );

  connect( d->mMultiAgendaView, SIGNAL(shiftedEvent(QDate,QDate)),
           SIGNAL(shiftedEvent(QDate,QDate)) );

  connect( d->mMultiAgendaView, SIGNAL(showIncidencePopupSignal(Akonadi::Item,QDate)),
           d->mPopup, SLOT(showIncidencePopup(Akonadi::Item,QDate)) );

  connect( d->mMultiAgendaView, SIGNAL(showNewEventPopupSignal()),
           SLOT(showNewEventPopup()) );

  connect( d->mMultiAgendaView, SIGNAL(incidenceSelected(Akonadi::Item,QDate)),
           SIGNAL(incidenceSelected(Akonadi::Item,QDate)) );

  connect( d->mMultiAgendaView, SIGNAL(showIncidenceSignal(Akonadi::Item)),
           SIGNAL(showIncidenceSignal(Akonadi::Item)) );

  connect( d->mMultiAgendaView, SIGNAL(editIncidenceSignal(Akonadi::Item)),
           SIGNAL(editIncidenceSignal(Akonadi::Item)) );

  connect( d->mMultiAgendaView, SIGNAL(deleteIncidenceSignal(Akonadi::Item)),
           SIGNAL(deleteIncidenceSignal(Akonadi::Item)) );

  connect( d->mMultiAgendaView, SIGNAL(cutIncidenceSignal(Akonadi::Item)),
           SIGNAL(cutIncidenceSignal(Akonadi::Item)) );

  connect( d->mMultiAgendaView, SIGNAL(copyIncidenceSignal(Akonadi::Item)),
           SIGNAL(copyIncidenceSignal(Akonadi::Item)) );

  connect( d->mMultiAgendaView, SIGNAL(pasteIncidenceSignal()),
           SIGNAL(pasteIncidenceSignal()) );

  connect( d->mMultiAgendaView, SIGNAL(toggleAlarmSignal(Akonadi::Item)),
           SIGNAL(toggleAlarmSignal(Akonadi::Item)) );

  connect( d->mMultiAgendaView, SIGNAL(toggleTodoCompletedSignal(Akonadi::Item)),
           SIGNAL(toggleTodoCompletedSignal(Akonadi::Item)) );

  connect( d->mMultiAgendaView, SIGNAL(copyIncidenceToResourceSignal(Akonadi::Item,QString)),
           SIGNAL(copyIncidenceToResourceSignal(Akonadi::Item,QString)) );

  connect( d->mMultiAgendaView, SIGNAL(moveIncidenceToResourceSignal(Akonadi::Item,QString)),
           SIGNAL(moveIncidenceToResourceSignal(Akonadi::Item,QString)) );

  connect( d->mMultiAgendaView, SIGNAL(dissociateOccurrencesSignal(Akonadi::Item,QDate)),
           SIGNAL(dissociateOccurrencesSignal(Akonadi::Item,QDate)) );

  connect( d->mMultiAgendaView, SIGNAL(newEventSignal()),
           SIGNAL(newEventSignal()) );

  connect( d->mMultiAgendaView, SIGNAL(newEventSignal(QDate)),
           SIGNAL(newEventSignal(QDate)) );

  connect( d->mMultiAgendaView, SIGNAL(newEventSignal(QDateTime)),
           SIGNAL(newEventSignal(QDateTime)) );

  connect( d->mMultiAgendaView, SIGNAL(newEventSignal(QDateTime,QDateTime)),
           SIGNAL(newEventSignal(QDateTime,QDateTime)) );

  connect( d->mMultiAgendaView, SIGNAL(newTodoSignal(QDate)),
           SIGNAL(newTodoSignal(QDate)) );

  connect( d->mMultiAgendaView, SIGNAL(newSubTodoSignal(Akonadi::Item)),
           SIGNAL(newSubTodoSignal(Akonadi::Item)) );

  connect( d->mMultiAgendaView, SIGNAL(newJournalSignal(QDate)),
           SIGNAL(newJournalSignal(QDate)) );

}

void MultiAgendaView::setCalendar( const Akonadi::ETMCalendar::Ptr &cal )
{
  d->mMultiAgendaView->setCalendar( cal );
  d->mPopup->setCalendar( cal );
}

MultiAgendaView::~MultiAgendaView()
{
  delete d;
}

Akonadi::Item::List MultiAgendaView::selectedIncidences()
{
  return d->mMultiAgendaView->selectedIncidences();
}

KCalCore::DateList MultiAgendaView::selectedIncidenceDates()
{
  return d->mMultiAgendaView->selectedIncidenceDates();
}

int MultiAgendaView::currentDateCount() const
{
  return d->mMultiAgendaView->currentDateCount();
}

void MultiAgendaView::showDates( const QDate &start, const QDate &end, const QDate & )
{
  d->mMultiAgendaView->showDates( start, end );
}

void MultiAgendaView::showIncidences( const Akonadi::Item::List &incidenceList, const QDate &date )
{
  d->mMultiAgendaView->showIncidences( incidenceList, date );
}

void MultiAgendaView::updateView()
{
  d->mMultiAgendaView->updateView();
}

Akonadi::Collection::Id MultiAgendaView::collectionId() const
{
  return d->mMultiAgendaView->collectionId();
}

void MultiAgendaView::changeIncidenceDisplay( const Akonadi::Item &, Akonadi::IncidenceChanger::ChangeType )
{
}

int MultiAgendaView::maxDatesHint() const
{
  return EventViews::AgendaView::MAX_DAY_COUNT;
}

void MultiAgendaView::setDateRange( const KDateTime &start, const KDateTime &end, const QDate & )
{
  d->mMultiAgendaView->setDateRange( start, end );
}

bool MultiAgendaView::eventDurationHint( QDateTime &startDt, QDateTime &endDt,
                                         bool &allDay )
{
  return d->mMultiAgendaView->eventDurationHint( startDt, endDt, allDay );
}

void MultiAgendaView::setIncidenceChanger( Akonadi::IncidenceChanger *changer )
{
  d->mMultiAgendaView->setIncidenceChanger( changer );
}

void MultiAgendaView::updateConfig()
{
  d->mMultiAgendaView->updateConfig();
}

void MultiAgendaView::setChanges( EventViews::EventView::Changes changes )
{
  // Only ConfigChanged and FilterChanged should go from korg->AgendaView
  // All other values are already detected inside AgendaView.
  // We could just pass "changes", but korganizer does a very bad job at
  // determining what changed, for example if you move an incidence
  // the BaseView::setDateRange(...) is called causing DatesChanged
  // flag to be on, when no dates changed.
  EventViews::EventView::Changes c;
  if ( changes.testFlag( EventViews::EventView::ConfigChanged ) ) {
    c = EventViews::EventView::ConfigChanged;
  }

  if ( changes.testFlag( EventViews::EventView::FilterChanged ) ) {
    c |= EventViews::EventView::FilterChanged;
  }

  d->mMultiAgendaView->setChanges( c | d->mMultiAgendaView->changes() );
}

bool MultiAgendaView::hasConfigurationDialog() const
{
  // It has. And it's implemented in korg, not libeventviews.
  return true;
}

void MultiAgendaView::showConfigurationDialog( QWidget *parent )
{
  QPointer<MultiAgendaViewConfigDialog> dlg(
    new MultiAgendaViewConfigDialog( d->mMultiAgendaView->calendar()->entityTreeModel(),
                                     parent ) );

  dlg->setUseCustomColumns( d->mMultiAgendaView->customColumnSetupUsed() );
  dlg->setNumberOfColumns( d->mMultiAgendaView->customNumberOfColumns() );

  QVector<KCheckableProxyModel*> models = d->mMultiAgendaView->collectionSelectionModels();
  for ( int i = 0; i < models.size(); ++i ) {
    dlg->setSelectionModel( i, models[i] );
  }

  QVector<QString> customColumnTitles = d->mMultiAgendaView->customColumnTitles();
  for ( int i = 0; i < customColumnTitles.size(); ++i ) {
    dlg->setColumnTitle( i, customColumnTitles[i] );
  }

  if ( dlg->exec() == QDialog::Accepted ) {
    d->mMultiAgendaView->customCollectionsChanged( dlg );
  }

  delete dlg;
}

KCheckableProxyModel *MultiAgendaView::takeCustomCollectionSelectionProxyModel()
{
  return d->mMultiAgendaView->takeCustomCollectionSelectionProxyModel();
}

void MultiAgendaView::setCustomCollectionSelectionProxyModel( KCheckableProxyModel *model )
{
  d->mMultiAgendaView->setCustomCollectionSelectionProxyModel( model );
}

class MultiAgendaViewConfigDialog::Private
{
  public:
    MultiAgendaViewConfigDialog *const q;
    explicit Private( QAbstractItemModel *base, MultiAgendaViewConfigDialog *qq )
      : q( qq ), baseModel( base ), currentColumn( 0 )
    {}

    ~Private() { qDeleteAll( newlyCreated ); }

    void setUpColumns( int n );
    AkonadiCollectionView *createView( KCheckableProxyModel *model );
    AkonadiCollectionView *view( int index ) const;
    QVector<KCheckableProxyModel*> newlyCreated;
    QVector<KCheckableProxyModel*> selections;
    QVector<QString> titles;
    Ui::MultiAgendaViewConfigWidget ui;
    QStandardItemModel listModel;
    QAbstractItemModel *baseModel;
    int currentColumn;
};

void MultiAgendaView::restoreConfig( const KConfigGroup &configGroup )
{
  d->mMultiAgendaView->restoreConfig( configGroup );
}

void MultiAgendaView::saveConfig( KConfigGroup &configGroup )
{
  d->mMultiAgendaView->saveConfig( configGroup );
}

MultiAgendaViewConfigDialog::MultiAgendaViewConfigDialog( QAbstractItemModel *baseModel,
                                                          QWidget *parent )
  : QDialog( parent ), d( new Private( baseModel, this ) )
{
  setWindowTitle( i18n( "Configure Side-By-Side View" ) );
  QVBoxLayout *mainLayout = new QVBoxLayout;
  setLayout(mainLayout);
  QWidget *widget = new QWidget;
  d->ui.setupUi( widget );
  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
  QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
  okButton->setDefault(true);
  okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  mainLayout->addWidget(buttonBox);
  mainLayout->addWidget(widget);

  d->ui.columnList->setModel( &d->listModel );
  connect( d->ui.columnList->selectionModel(),
           SIGNAL(currentChanged(QModelIndex,QModelIndex)),
           this, SLOT(currentChanged(QModelIndex)) );
  connect( d->ui.useCustomRB, SIGNAL(toggled(bool)),
           this, SLOT(useCustomToggled(bool)) );
  connect( d->ui.columnNumberSB, SIGNAL(valueChanged(int)),
           this, SLOT(numberOfColumnsChanged(int)) );
  connect( d->ui.titleLE, SIGNAL(textEdited(QString)),
           this, SLOT(titleEdited(QString)) );
  d->setUpColumns( numberOfColumns() );
  useCustomToggled( false );
}

void MultiAgendaViewConfigDialog::currentChanged( const QModelIndex &index )
{
  if ( !index.isValid() ) {
    return;
  }

  const int idx = index.data( Qt::UserRole ).toInt();
  d->ui.titleLE->setText( index.data( Qt::DisplayRole ).toString() );
  d->ui.selectionStack->setCurrentIndex( idx );
  d->currentColumn = idx;
}

void MultiAgendaViewConfigDialog::useCustomToggled( bool on )
{
  d->ui.columnList->setEnabled( on );
  d->ui.columnNumberLabel->setEnabled( on );
  d->ui.columnNumberSB->setEnabled( on );
  d->ui.selectedCalendarsLabel->setEnabled( on );
  d->ui.selectionStack->setEnabled( on );
  d->ui.titleLabel->setEnabled( on );
  d->ui.titleLE->setEnabled( on );
  // this explicit enabling/disabling of the ETV is necessary, as the stack
  // widget state is not propagated to the collection views.  pprobably because
  // the Akonadi error overlays enable/disable the ETV explicitly and thus
  // override the parent-child relationship?
  for ( int i = 0; i < d->ui.selectionStack->count(); ++i ) {
    d->view( i )->view()->setEnabled( on );
  }
}

AkonadiCollectionView *MultiAgendaViewConfigDialog::Private::createView(
  KCheckableProxyModel *model )
{
  AkonadiCollectionView *cview = new AkonadiCollectionView( 0, false, q );
  cview->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding );
  cview->setCollectionSelectionProxyModel( model );
  return cview;
}

void MultiAgendaViewConfigDialog::Private::setUpColumns( int n )
{
  Q_ASSERT( n > 0 );
  const int oldN = selections.size();
  if ( oldN == n ) {
    return;
  }

  if ( n < oldN ) {
    for ( int i = oldN - 1; i >= n; --i ) {
      QWidget *w = ui.selectionStack->widget( i );
      ui.selectionStack->removeWidget( w );
      delete w;
      qDeleteAll( listModel.takeRow( i ) );
      KCheckableProxyModel *const m = selections[i];
      selections.remove( i );
      const int pos = newlyCreated.indexOf( m );
      if ( pos != -1 ) {
        delete m;
        newlyCreated.remove( pos );
      }
    }
  } else {
    selections.resize( n );
    for ( int i = oldN; i < n; ++i ) {
      QStandardItem *item = new QStandardItem;
      item->setEditable( false );
      if ( titles.count() <= i ) {
        titles.resize( i + 1 );
        titles[i] = generateColumnLabel( i );
      }
      item->setText( titles[i] );
      item->setData( i, Qt::UserRole );
      listModel.appendRow( item );

      QSortFilterProxyModel *sortProxy = new QSortFilterProxyModel;
      sortProxy->setDynamicSortFilter( true );
      sortProxy->setSourceModel( baseModel );

      KColumnFilterProxyModel *columnFilterProxy = new KColumnFilterProxyModel( sortProxy );
      columnFilterProxy->setVisibleColumn( Akonadi::ETMCalendar::CollectionTitle );
      columnFilterProxy->setSourceModel( sortProxy );

      QItemSelectionModel *qsm = new QItemSelectionModel( columnFilterProxy, columnFilterProxy );

      KCheckableProxyModel *selection = new KCheckableProxyModel;
      selection->setSourceModel( columnFilterProxy );
      selection->setSelectionModel( qsm );

      AkonadiCollectionView *cview = createView( selection );
      const int idx = ui.selectionStack->addWidget( cview );
      Q_ASSERT( i == idx );
      Q_UNUSED( idx );
      selections[i] = selection;
      newlyCreated.push_back( selection );
    }
  }
}

bool MultiAgendaViewConfigDialog::useCustomColumns() const
{
  return d->ui.useCustomRB->isChecked();
}

void MultiAgendaViewConfigDialog::setUseCustomColumns( bool custom )
{
  if ( custom ) {
    d->ui.useCustomRB->setChecked( true );
  } else {
    d->ui.useDefaultRB->setChecked( true );
  }
}

int MultiAgendaViewConfigDialog::numberOfColumns() const
{
  return d->ui.columnNumberSB->value();
}

void MultiAgendaViewConfigDialog::setNumberOfColumns( int n )
{
  d->ui.columnNumberSB->setValue( n );
  d->setUpColumns( n );
}

KCheckableProxyModel *MultiAgendaViewConfigDialog::takeSelectionModel( int column )
{
  if ( column < 0 || column >= d->selections.size() ) {
    return 0;
  }

  KCheckableProxyModel *const m = d->selections[column];
  d->newlyCreated.erase( std::remove( d->newlyCreated.begin(),
                                      d->newlyCreated.end(), m ),
                         d->newlyCreated.end() );
  return m;
}

AkonadiCollectionView *MultiAgendaViewConfigDialog::Private::view( int index ) const
{
  return qobject_cast<AkonadiCollectionView*>( ui.selectionStack->widget( index ) );
}

void MultiAgendaViewConfigDialog::setSelectionModel( int column, KCheckableProxyModel *model )
{
  Q_ASSERT( column >= 0 && column < d->selections.size() );

  KCheckableProxyModel *const m = d->selections[column];
  if ( m == model ) {
    return;
  }

  AkonadiCollectionView *cview = d->view( column );
  Q_ASSERT( cview );
  cview->setCollectionSelectionProxyModel( model );

  if ( d->newlyCreated.contains( m ) ) {
    d->newlyCreated.erase( std::remove( d->newlyCreated.begin(),
                                        d->newlyCreated.end(), m ),
                           d->newlyCreated.end() );
    delete m;
  }

  d->selections[column] = model;
}

void MultiAgendaViewConfigDialog::titleEdited( const QString &text )
{
  d->titles[d->currentColumn] = text;
  d->listModel.item( d->currentColumn )->setText( text );
}

void MultiAgendaViewConfigDialog::numberOfColumnsChanged( int number )
{
  d->setUpColumns( number );
}

QString MultiAgendaViewConfigDialog::columnTitle( int column ) const
{
  Q_ASSERT( column >= 0 );
  return column >= d->titles.count() ? QString() : d->titles[column];
}

void MultiAgendaViewConfigDialog::setColumnTitle( int column, const QString &title )
{
  Q_ASSERT( column >= 0 );
  d->titles.resize( qMax( d->titles.size(), column + 1 ) );
  d->titles[column] = title;
  if ( QStandardItem *const item = d->listModel.item( column ) ) {
    item->setText( title );
  }
  //TODO update LE if item is selected
}

void MultiAgendaViewConfigDialog::accept()
{
  d->newlyCreated.clear();
  QDialog::accept();
}

MultiAgendaViewConfigDialog::~MultiAgendaViewConfigDialog()
{
  delete d;
}

