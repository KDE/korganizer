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

#include "multiagendaview.h"
#include "koprefs.h"
#include "timelabelszone.h"
#include "views/agendaview/koagenda.h"
#include "views/agendaview/koagendaview.h"
#include "akonadicollectionview.h"

#include "ui_multiagendaviewconfigwidget.h"

#include <Akonadi/EntityTreeView>
#include <akonadi/kcal/calendar.h>
#include <akonadi/kcal/collectionselection.h>
#include <akonadi/kcal/collectionselectionproxymodel.h>
#include <akonadi/kcal/entitymodelstatesaver.h>
#include <akonadi/kcal/utils.h>


#include <KGlobalSettings>
#include <KHBox>
#include <KVBox>

#include <Q3ScrollView>
#include <QHBoxLayout>
#include <QLabel>
#include <QResizeEvent>
#include <QSplitter>
#include <QStandardItem>
#include <QStandardItemModel>

#include <algorithm>

using namespace Akonadi;
using namespace KOrg;

static QString generateColumnLabel( int c )
{
  return i18n( "Agenda %1", c + 1 );
}

MultiAgendaView::MultiAgendaView( QWidget *parent )
  : AgendaView( parent ),
    mUpdateOnShow( true ),
    mPendingChanges( true ),
    mCustomColumnSetupUsed( false ),
    mCustomNumberOfColumns( 2 )
{
  QHBoxLayout *topLevelLayout = new QHBoxLayout( this );
  topLevelLayout->setSpacing( 0 );
  topLevelLayout->setMargin( 0 );

  QFontMetrics fm( font() );
  int topLabelHeight = 2 * fm.height() + fm.lineSpacing();

  KVBox *topSideBox = new KVBox( this );

  QWidget *topSideSpacer = new QWidget( topSideBox );
  topSideSpacer->setFixedHeight( topLabelHeight );

  mLeftSplitter = new QSplitter( Qt::Vertical, topSideBox );
  mLeftSplitter->setOpaqueResize( KGlobalSettings::opaqueResize() );

  QLabel *label = new QLabel( i18n( "All Day" ), mLeftSplitter );
  label->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
  label->setWordWrap( true );

  KVBox *sideBox = new KVBox( mLeftSplitter );

  EventIndicator *eiSpacer = new EventIndicator( EventIndicator::Top, sideBox );
  eiSpacer->changeColumns( 0 );

  // compensate for the frame the agenda views but not the timelabels have
  QWidget *timeLabelTopAlignmentSpacer = new QWidget( sideBox );

  mTimeLabelsZone = new TimeLabelsZone( sideBox );

  QWidget *timeLabelBotAlignmentSpacer = new QWidget( sideBox );

  eiSpacer = new EventIndicator( EventIndicator::Bottom, sideBox );
  eiSpacer->changeColumns( 0 );

  mLeftBottomSpacer = new QWidget( topSideBox );

  topLevelLayout->addWidget( topSideBox );

  mScrollView = new Q3ScrollView( this );
  mScrollView->setResizePolicy( Q3ScrollView::Manual );
  mScrollView->setVScrollBarMode( Q3ScrollView::AlwaysOff );

  // asymetric since the timelabels
  timeLabelTopAlignmentSpacer->setFixedHeight( mScrollView->frameWidth() - 1 );
  // have 25 horizontal lines
  timeLabelBotAlignmentSpacer->setFixedHeight( mScrollView->frameWidth() - 2 );

  mScrollView->setFrameShape( QFrame::NoFrame );
  topLevelLayout->addWidget( mScrollView, 100 );
  mTopBox = new KHBox( mScrollView->viewport() );
  mScrollView->addChild( mTopBox );

  topSideBox = new KVBox( this );

  topSideSpacer = new QWidget( topSideBox );
  topSideSpacer->setFixedHeight( topLabelHeight );

  mRightSplitter = new QSplitter( Qt::Vertical, topSideBox );
  mRightSplitter->setOpaqueResize( KGlobalSettings::opaqueResize() );

  new QWidget( mRightSplitter );
  sideBox = new KVBox( mRightSplitter );

  eiSpacer = new EventIndicator( EventIndicator::Top, sideBox );
  eiSpacer->setFixedHeight( eiSpacer->minimumHeight() );
  eiSpacer->changeColumns( 0 );
  mScrollBar = new QScrollBar( Qt::Vertical, sideBox );
  eiSpacer = new EventIndicator( EventIndicator::Bottom, sideBox );
  eiSpacer->setFixedHeight( eiSpacer->minimumHeight() );
  eiSpacer->changeColumns( 0 );

  mRightBottomSpacer = new QWidget( topSideBox );
  topLevelLayout->addWidget( topSideBox );
}

void MultiAgendaView::setCalendar( Akonadi::Calendar *cal )
{
  AgendaView::setCalendar( cal );
  Q_FOREACH( CollectionSelectionProxyModel* const i, mCollectionSelectionModels )
      i->setSourceModel( cal->treeModel() );
  recreateViews();
}

void MultiAgendaView::recreateViews()
{
  if ( !mPendingChanges ) {
    return;
  }
  mPendingChanges = false;

  deleteViews();

  if ( mCustomColumnSetupUsed ) {
    Q_ASSERT( mCollectionSelectionModels.size() == mCustomNumberOfColumns );
    for ( int i = 0; i < mCustomNumberOfColumns; ++i )
      addView( mCollectionSelectionModels[i], mCustomColumnTitles[i] );
  } else {
    Q_FOREACH( const Collection &i, collectionSelection()->selectedCollections() )
      addView( i );
  }
  // no resources activated, so stop here to avoid crashing somewhere down the line
  // TODO: show a nice message instead
  if ( mAgendaViews.isEmpty() ) {
    return;
  }

  setupViews();
  QTimer::singleShot( 0, this, SLOT(slotResizeScrollView()) );
  mTimeLabelsZone->updateAll();

  TimeLabels *timeLabel = mTimeLabelsZone->timeLabels().first();
  connect( timeLabel->verticalScrollBar(), SIGNAL(valueChanged(int)),
           mScrollBar, SLOT(setValue(int)) );
  connect( mScrollBar, SIGNAL(valueChanged(int)),
           timeLabel, SLOT(positionChanged(int)) );

  connect( mLeftSplitter, SIGNAL(splitterMoved(int,int)), SLOT(resizeSplitters()) );
  connect( mRightSplitter, SIGNAL(splitterMoved(int,int)), SLOT(resizeSplitters()) );
  QTimer::singleShot( 0, this, SLOT(resizeSplitters()) );
  QTimer::singleShot( 0, this, SLOT(setupScrollBar()) );
  foreach ( TimeLabels *label, mTimeLabelsZone->timeLabels() ) {
    label->positionChanged();
  }
}

void MultiAgendaView::deleteViews()
{
  Q_FOREACH( KOAgendaView *const i, mAgendaViews ) {
    CollectionSelectionProxyModel* proxy = i->takeCustomCollectionSelectionProxyModel();
    if ( proxy && !mCollectionSelectionModels.contains( proxy ) )
      delete proxy;
    delete i;
  }

  mAgendaViews.clear();
  qDeleteAll( mAgendaWidgets );
  mAgendaWidgets.clear();
}

void MultiAgendaView::setupViews()
{
  foreach ( KOAgendaView *agenda, mAgendaViews ) {
    connect( agenda, SIGNAL(newEventSignal(Akonadi::Collection::List)),
             SIGNAL(newEventSignal(Akonadi::Collection::List)) );
    connect( agenda, SIGNAL(editIncidenceSignal(Akonadi::Item)),
             SIGNAL(editIncidenceSignal(Akonadi::Item)) );
    connect( agenda, SIGNAL(showIncidenceSignal(Akonadi::Item)),
             SIGNAL(showIncidenceSignal(Akonadi::Item)) );
    connect( agenda, SIGNAL(deleteIncidenceSignal(Akonadi::Item)),
             SIGNAL(deleteIncidenceSignal(Akonadi::Item)) );
    connect( agenda, SIGNAL(startMultiModify(const QString &)),
             SIGNAL(startMultiModify(const QString &)) );
    connect( agenda, SIGNAL(endMultiModify()),
             SIGNAL(endMultiModify()) );

    connect( agenda, SIGNAL(incidenceSelected(Akonadi::Item, const QDate &)),
             SIGNAL(incidenceSelected(Akonadi::Item, const QDate &)) );

    connect( agenda, SIGNAL(cutIncidenceSignal(Akonadi::Item)),
             SIGNAL(cutIncidenceSignal(Akonadi::Item)) );
    connect( agenda, SIGNAL(copyIncidenceSignal(Akonadi::Item)),
             SIGNAL(copyIncidenceSignal(Akonadi::Item)) );
    connect( agenda, SIGNAL(pasteIncidenceSignal()),
             SIGNAL(pasteIncidenceSignal()) );
    connect( agenda, SIGNAL(toggleAlarmSignal(Akonadi::Item)),
             SIGNAL(toggleAlarmSignal(Akonadi::Item)) );
    connect( agenda, SIGNAL(dissociateOccurrencesSignal(Akonadi::Item, const QDate&)),
             SIGNAL(dissociateOccurrencesSignal(Akonadi::Item, const QDate&)) );

    connect( agenda, SIGNAL(newEventSignal(Akonadi::Collection::List,const QDate&)),
             SIGNAL(newEventSignal(Akonadi::Collection::List,const QDate&)) );
    connect( agenda, SIGNAL(newEventSignal(Akonadi::Collection::List,const QDateTime&)),
             SIGNAL(newEventSignal(Akonadi::Collection::List,const QDateTime&)) );
    connect( agenda, SIGNAL(newEventSignal(Akonadi::Collection::List,const QDateTime&, const QDateTime&)),
             SIGNAL(newEventSignal(Akonadi::Collection::List,const QDateTime&, const QDateTime&)) );

    connect( agenda, SIGNAL(newTodoSignal(const QDate&)),
             SIGNAL(newTodoSignal(const QDate&)) );

    connect( agenda, SIGNAL(incidenceSelected(Akonadi::Item, const QDate &)),
             SLOT(slotSelectionChanged()) );

    connect( agenda, SIGNAL(timeSpanSelectionChanged()),
             SLOT(slotClearTimeSpanSelection()) );

    disconnect( agenda->agenda(),
                SIGNAL(zoomView(const int,const QPoint&,const Qt::Orientation)),
                agenda, 0 );
    connect( agenda->agenda(),
             SIGNAL(zoomView(const int,const QPoint&,const Qt::Orientation)),
             SLOT(zoomView(const int,const QPoint&,const Qt::Orientation)) );
  }

  KOAgendaView *lastView = mAgendaViews.last();
  foreach ( KOAgendaView *agenda, mAgendaViews ) {
    if ( agenda != lastView ) {
      connect( agenda->agenda()->verticalScrollBar(), SIGNAL(valueChanged(int)),
               lastView->agenda()->verticalScrollBar(), SLOT(setValue(int)) );
    }
  }

  foreach ( KOAgendaView *agenda, mAgendaViews ) {
    agenda->readSettings();
  }

  int minWidth = 0;
  foreach ( QWidget *widget, mAgendaWidgets ) {
    minWidth = qMax( minWidth, widget->minimumSizeHint().width() );
  }
  foreach ( QWidget *widget, mAgendaWidgets ) {
    widget->setMinimumWidth( minWidth );
  }
}

MultiAgendaView::~MultiAgendaView()
{
}

Akonadi::Item::List MultiAgendaView::selectedIncidences()
{
  Akonadi::Item::List list;
  foreach ( KOAgendaView *agendaView, mAgendaViews ) {
    list += agendaView->selectedIncidences();
  }
  return list;
}

DateList MultiAgendaView::selectedDates()
{
  DateList list;
  foreach ( KOAgendaView *agendaView, mAgendaViews ) {
    list += agendaView->selectedDates();
  }
  return list;
}

int MultiAgendaView::currentDateCount()
{
  foreach ( KOAgendaView *agendaView, mAgendaViews ) {
    return agendaView->currentDateCount();
  }
  return 0;
}

void MultiAgendaView::showDates( const QDate &start, const QDate &end )
{
  mStartDate = start;
  mEndDate = end;
  recreateViews();
  foreach ( KOAgendaView *agendaView, mAgendaViews ) {
    agendaView->showDates( start, end );
  }
}

void MultiAgendaView::showIncidences( const Item::List &incidenceList, const QDate &date )
{
  foreach ( KOAgendaView *agendaView, mAgendaViews ) {
    agendaView->showIncidences( incidenceList, date );
  }
}

void MultiAgendaView::updateView()
{
  recreateViews();
  foreach ( KOAgendaView *agendaView, mAgendaViews ) {
    agendaView->updateView();
  }
}

void MultiAgendaView::changeIncidenceDisplay( const Item& incidence, int mode )
{
  foreach ( KOAgendaView *agendaView, mAgendaViews ) {
    agendaView->changeIncidenceDisplay( incidence, mode );
  }
}

int MultiAgendaView::maxDatesHint()
{
  foreach ( KOAgendaView *agendaView, mAgendaViews ) {
    return agendaView->maxDatesHint();
  }
  return 0;
}

void MultiAgendaView::slotSelectionChanged()
{
  foreach ( KOAgendaView *agenda, mAgendaViews ) {
    if ( agenda != sender() ) {
      agenda->clearSelection();
    }
  }
}

bool MultiAgendaView::eventDurationHint( QDateTime &startDt, QDateTime &endDt, bool &allDay )
{
  foreach ( KOAgendaView *agenda, mAgendaViews ) {
    bool valid = agenda->eventDurationHint( startDt, endDt, allDay );
    if ( valid ) {
      return true;
    }
  }
  return false;
}

void MultiAgendaView::slotClearTimeSpanSelection()
{
  foreach ( KOAgendaView *agenda, mAgendaViews ) {
    if ( agenda != sender() ) {
      agenda->clearTimeSpanSelection();
    }
  }
}

KOAgendaView* MultiAgendaView::createView( const QString &title )
{
  QWidget *box = new QWidget( mTopBox );
  QVBoxLayout* layout = new QVBoxLayout( box );
  layout->setMargin( 0 );
  QLabel *l = new QLabel( title );
  layout->addWidget( l );
  l->setAlignment( Qt::AlignVCenter | Qt::AlignHCenter );
  KOAgendaView *av = new KOAgendaView( 0, true );
  layout->addWidget( av );
  av->setCalendar( calendar() );
  av->setIncidenceChanger( mChanger );
  av->agenda()->setVScrollBarMode( Q3ScrollView::AlwaysOff );
  mAgendaViews.append( av );
  mAgendaWidgets.append( box );
  box->show();
  mTimeLabelsZone->setAgendaView( av );
  connect( av->splitter(), SIGNAL(splitterMoved(int,int)), SLOT(resizeSplitters()) );
  return av;
}

void MultiAgendaView::addView( const Collection &collection )
{
  KOAgendaView* av = createView( Akonadi::displayName( collection ) );
  av->setCollection( collection.id() );
}

void MultiAgendaView::addView( CollectionSelectionProxyModel* sm, const QString &title )
{
  KOAgendaView* av = createView( title );
  av->setCustomCollectionSelectionProxyModel( sm );
}

void MultiAgendaView::resizeEvent( QResizeEvent *ev )
{
  resizeScrollView( ev->size() );
  AgendaView::resizeEvent( ev );
}

void MultiAgendaView::resizeScrollView( const QSize &size )
{
  const int widgetWidth = size.width() - mTimeLabelsZone->width() - mScrollBar->width();
  const int width = qMax( mTopBox->sizeHint().width(), widgetWidth );
  int height = size.height();
  if ( width > widgetWidth ) {
    const int sbHeight = mScrollView->horizontalScrollBar()->height();
    height -= sbHeight;
    mLeftBottomSpacer->setFixedHeight( sbHeight );
    mRightBottomSpacer->setFixedHeight( sbHeight );
  } else {
    mLeftBottomSpacer->setFixedHeight( 0 );
    mRightBottomSpacer->setFixedHeight( 0 );
  }
  mScrollView->resizeContents( width, height );
  mTopBox->resize( width, height );
}

void MultiAgendaView::setIncidenceChanger( IncidenceChangerBase *changer )
{
  AgendaView::setIncidenceChanger( changer );
  foreach ( KOAgendaView *agenda, mAgendaViews ) {
    agenda->setIncidenceChanger( changer );
  }
}

void MultiAgendaView::updateConfig()
{
  AgendaView::updateConfig();
  mTimeLabelsZone->updateAll();
  foreach ( KOAgendaView *agenda, mAgendaViews ) {
    agenda->updateConfig();
  }
}

void MultiAgendaView::resizeSplitters()
{
  if ( mAgendaViews.isEmpty() )
    return;
  QSplitter *lastMovedSplitter = qobject_cast<QSplitter*>( sender() );
  if ( !lastMovedSplitter ) {
    lastMovedSplitter = mAgendaViews.first()->splitter();
  }
  foreach ( KOAgendaView *agenda, mAgendaViews ) {
    if ( agenda->splitter() == lastMovedSplitter ) {
      continue;
    }
    agenda->splitter()->setSizes( lastMovedSplitter->sizes() );
  }
  if ( lastMovedSplitter != mLeftSplitter ) {
    mLeftSplitter->setSizes( lastMovedSplitter->sizes() );
  }
  if ( lastMovedSplitter != mRightSplitter ) {
    mRightSplitter->setSizes( lastMovedSplitter->sizes() );
  }
}

void MultiAgendaView::zoomView( const int delta, const QPoint &pos, const Qt::Orientation ori )
{
  if ( ori == Qt::Vertical ) {
    if ( delta > 0 ) {
      if ( KOPrefs::instance()->mHourSize > 4 ) {
        KOPrefs::instance()->mHourSize--;
      }
    } else {
      KOPrefs::instance()->mHourSize++;
    }
  }

  foreach ( KOAgendaView *agenda, mAgendaViews ) {
    agenda->zoomView( delta, pos, ori );
  }

  mTimeLabelsZone->updateAll();
}

void MultiAgendaView::slotResizeScrollView()
{
  resizeScrollView( size() );
}

void MultiAgendaView::showEvent( QShowEvent *event )
{
  AgendaView::showEvent( event );
  if ( mUpdateOnShow ) {
    mUpdateOnShow = false;
    mPendingChanges = true; // force a full view recreation
    showDates( mStartDate, mEndDate );
  }
}

void MultiAgendaView::setUpdateNeeded()
{
  mPendingChanges = true;
  foreach ( KOAgendaView *agenda, mAgendaViews ) {
    agenda->setUpdateNeeded();
  }
}

void MultiAgendaView::setupScrollBar()
{
  if ( mAgendaViews.isEmpty() )
    return;
  QScrollBar *scrollBar = mAgendaViews.first()->agenda()->verticalScrollBar();
  mScrollBar->setMinimum( scrollBar->minimum() );
  mScrollBar->setMaximum( scrollBar->maximum() );
  mScrollBar->setSingleStep( scrollBar->singleStep() );
  mScrollBar->setPageStep( scrollBar->pageStep() );
  mScrollBar->setValue( scrollBar->value() );
}

void MultiAgendaView::collectionSelectionChanged()
{
  recreateViews();
}

bool MultiAgendaView::hasConfigurationDialog() const
{
  return true;
}

void MultiAgendaView::showConfigurationDialog( QWidget* parent )
{
  QPointer<MultiAgendaViewConfigDialog> dlg( new MultiAgendaViewConfigDialog( calendar()->treeModel(), parent ) );
  dlg->setUseCustomColumns( mCustomColumnSetupUsed );
  dlg->setNumberOfColumns( mCustomNumberOfColumns );
  for ( int i = 0; i < mCollectionSelectionModels.size(); ++i )
      dlg->setSelectionModel( i, mCollectionSelectionModels[i] );
  for ( int i = 0; i < mCustomColumnTitles.size(); ++i )
    dlg->setColumnTitle( i, mCustomColumnTitles[i] );
  if ( dlg->exec() == QDialog::Accepted )
  {
    mCustomColumnSetupUsed = dlg->useCustomColumns();
    mCustomNumberOfColumns = dlg->numberOfColumns();
    QVector<CollectionSelectionProxyModel*> newModels;
    newModels.resize( mCustomNumberOfColumns );
    mCustomColumnTitles.resize( mCustomNumberOfColumns );
    for ( int i = 0; i < mCustomNumberOfColumns; ++i ) {
      newModels[i] = dlg->takeSelectionModel( i );
      mCustomColumnTitles[i] = dlg->columnTitle( i );
    }
    mCollectionSelectionModels = newModels;
    mPendingChanges = true;
    recreateViews();
  }
  delete dlg;
}

void MultiAgendaView::doRestoreConfig( const KConfigGroup &configGroup )
{
  mCustomColumnSetupUsed = configGroup.readEntry( "UseCustomColumnSetup", false );
  mCustomNumberOfColumns = configGroup.readEntry( "CustomNumberOfColumns", 2 );
  mCustomColumnTitles =  configGroup.readEntry( "ColumnTitles", QStringList() ).toVector();
  if ( mCustomColumnTitles.size() != mCustomNumberOfColumns ) {
    const int orig = mCustomColumnTitles.size();
    mCustomColumnTitles.resize( mCustomNumberOfColumns );
    for ( int i = orig; i < mCustomNumberOfColumns; ++i )
      mCustomColumnTitles[i] = generateColumnLabel( i );
  }
  QVector<CollectionSelectionProxyModel*> oldModels = mCollectionSelectionModels;
  mCollectionSelectionModels.clear();
  mCollectionSelectionModels.resize( mCustomNumberOfColumns );
  for ( int i = 0; i < mCustomNumberOfColumns; ++i ) {
    const KConfigGroup g = configGroup.config()->group( configGroup.name() + "_subView_" + QByteArray::number( i ) );
    CollectionSelectionProxyModel* selection = new CollectionSelectionProxyModel;
    selection->setDynamicSortFilter( true );
    if ( calendar() )
      selection->setSourceModel( calendar()->treeModel() );
    QItemSelectionModel* qsm = new QItemSelectionModel( selection, selection );
    selection->setSelectionModel( qsm );
    EntityModelStateSaver* saver = new EntityModelStateSaver( selection, selection );
    saver->addRole( Qt::CheckStateRole, "CheckState" );
    saver->restoreConfig( g );
    mCollectionSelectionModels[i] = selection;
  }
  mPendingChanges = true;
  recreateViews();
  qDeleteAll( oldModels );
}

void MultiAgendaView::doSaveConfig( KConfigGroup &configGroup )
{
  configGroup.writeEntry( "UseCustomColumnSetup", mCustomColumnSetupUsed );
  configGroup.writeEntry( "CustomNumberOfColumns", mCustomNumberOfColumns );
  const QStringList titleList = mCustomColumnTitles.toList();
  configGroup.writeEntry( "ColumnTitles", titleList );
  int idx = 0;
  Q_FOREACH( CollectionSelectionProxyModel* i, mCollectionSelectionModels ) {
    KConfigGroup g = configGroup.config()->group( configGroup.name() + "_subView_" + QByteArray::number( idx ) );
    ++idx;
    EntityModelStateSaver saver( i );
    saver.addRole( Qt::CheckStateRole, "CheckState" );
    saver.saveConfig( g );
  }
}

class MultiAgendaViewConfigDialog::Private {
public:
  MultiAgendaViewConfigDialog *const q;
  explicit Private( QAbstractItemModel* base, MultiAgendaViewConfigDialog* qq ) : q( qq ), baseModel( base ), currentColumn( 0 ) {

  }

  ~Private() {
    qDeleteAll( newlyCreated );
  }

  void setUpColumns( int n );
  AkonadiCollectionView* createView( CollectionSelectionProxyModel* model );
  AkonadiCollectionView* view( int index ) const;
  QVector<CollectionSelectionProxyModel*> newlyCreated;
  QVector<CollectionSelectionProxyModel*> selections;
  QVector<QString> titles;
  Ui::MultiAgendaViewConfigWidget ui;
  QStandardItemModel listModel;
  QAbstractItemModel* baseModel;
  int currentColumn;
};

MultiAgendaViewConfigDialog::MultiAgendaViewConfigDialog( QAbstractItemModel* baseModel, QWidget* parent ) : KDialog( parent ), d( new Private( baseModel, this ) )
{
  setWindowTitle( i18n("Configure Side-By-Side View") );
  QWidget* widget = new QWidget;
  d->ui.setupUi( widget );
  setMainWidget( widget );
  d->ui.columnList->setModel( &d->listModel );
  connect( d->ui.columnList->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(currentChanged(QModelIndex)) );
  connect( d->ui.useCustomRB, SIGNAL(toggled(bool)), this, SLOT(useCustomToggled(bool)) );
  connect( d->ui.columnNumberSB, SIGNAL(valueChanged(int)), this, SLOT(numberOfColumnsChanged(int)) );
  connect( d->ui.titleLE, SIGNAL(textEdited(QString)), this, SLOT(titleEdited(QString)) );
  d->setUpColumns( numberOfColumns() );
  useCustomToggled( false );
}

void MultiAgendaViewConfigDialog::currentChanged( const QModelIndex &index )
{
  if ( !index.isValid() )
    return;
  const int idx = index.data( Qt::UserRole ).toInt();
  d->ui.titleLE->setText( index.data( Qt::DisplayRole ).toString() );
  d->ui.selectionStack->setCurrentIndex( idx );
  d->currentColumn = idx;
}

void MultiAgendaViewConfigDialog::useCustomToggled( bool on ) {
  d->ui.columnList->setEnabled( on );
  d->ui.columnNumberLabel->setEnabled( on );
  d->ui.columnNumberSB->setEnabled( on );
  d->ui.selectedCalendarsLabel->setEnabled( on );
  d->ui.selectionStack->setEnabled( on );
  d->ui.titleLabel->setEnabled( on );
  d->ui.titleLE->setEnabled( on );
  // this explicit enabling/disabling of the ETV is necessary, as the stack widget state is not propagated to the collection views
  // probably because the Akonadi error overlays enable/disable the ETV explicitely and thus override the parent-child relationship?
  for ( int i = 0; i < d->ui.selectionStack->count(); ++i )
    d->view( i )->view()->setEnabled( on );

}

AkonadiCollectionView* MultiAgendaViewConfigDialog::Private::createView( CollectionSelectionProxyModel* model )
{
  AkonadiCollectionView* cview = new AkonadiCollectionView( 0, q );
  cview->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding );
  cview->setCollectionSelectionProxyModel( model );
  return cview;
}

void MultiAgendaViewConfigDialog::Private::setUpColumns( int n )
{
  Q_ASSERT( n > 0 );
  const int oldN = selections.size();
  if ( oldN == n )
    return;
  if ( n < oldN ) {
    for ( int i = oldN - 1; i >= n; --i ) {
      QWidget* w = ui.selectionStack->widget( i );
      ui.selectionStack->removeWidget( w );
      delete w;
      qDeleteAll( listModel.takeRow( i ) );
      CollectionSelectionProxyModel* const m = selections[i];
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
      QStandardItem* item = new QStandardItem;
      item->setEditable( false );
      if ( titles.count() <= i ) {
        titles.resize( i + 1 );
        titles[i] = generateColumnLabel( i );
      }
      item->setText( titles[i] );
      item->setData( i, Qt::UserRole );
      listModel.appendRow( item );
      CollectionSelectionProxyModel* selection = new CollectionSelectionProxyModel;
      selection->setDynamicSortFilter( true );
      selection->setSourceModel( baseModel );
      QItemSelectionModel* qsm = new QItemSelectionModel( selection, selection );
      selection->setSelectionModel( qsm );
      AkonadiCollectionView* cview = createView( selection );
      const int idx = ui.selectionStack->addWidget( cview );
      Q_ASSERT( i == idx );
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
  if ( custom )
    d->ui.useCustomRB->setChecked( true );
  else
    d->ui.useDefaultRB->setChecked( true );
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

CollectionSelectionProxyModel* MultiAgendaViewConfigDialog::takeSelectionModel( int column )
{
  if ( column < 0 || column >= d->selections.size() )
    return 0;
  CollectionSelectionProxyModel* const m = d->selections[column];
  d->newlyCreated.erase( std::remove( d->newlyCreated.begin(), d->newlyCreated.end(), m ), d->newlyCreated.end() );
  return m;
}

AkonadiCollectionView* MultiAgendaViewConfigDialog::Private::view( int index ) const
{
  return qobject_cast<AkonadiCollectionView*>( ui.selectionStack->widget( index ) );
}

void MultiAgendaViewConfigDialog::setSelectionModel( int column, CollectionSelectionProxyModel* model )
{
  Q_ASSERT( column >= 0 && column < d->selections.size() );

  CollectionSelectionProxyModel* const m = d->selections[column];
  if ( m == model )
    return;
  AkonadiCollectionView* cview = d->view( column );
  Q_ASSERT( cview );
  cview->setCollectionSelectionProxyModel( model );

  if ( d->newlyCreated.contains( m ) ) {
    d->newlyCreated.erase( std::remove( d->newlyCreated.begin(), d->newlyCreated.end(), m ), d->newlyCreated.end() );
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
  if ( QStandardItem *const item = d->listModel.item( column ) )
    item->setText( title );
  //TODO update LE if item is selected
}

void MultiAgendaViewConfigDialog::accept()
{
  d->newlyCreated.clear();
  KDialog::accept();
}

MultiAgendaViewConfigDialog::~MultiAgendaViewConfigDialog()
{
  delete d;
}

#include "multiagendaview.moc"
