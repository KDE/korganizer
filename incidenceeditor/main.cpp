#include <qheaderview.h>
#include <qsplitter.h>
#include <QHBoxLayout>
#include <kapplication.h>
#include <kmainwindow.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kurl.h>
#include <ksystemtimezone.h>
#include <QLineEdit>

#include <akonadi/collection.h>
#include <akonadi/collectionview.h>
#include <akonadi/collectionfilterproxymodel.h>
#include <akonadi/collectionmodel.h>
#include <akonadi/collectionview.h>
#include <akonadi/collectiondeletejob.h>
#include <akonadi/itemmodel.h>
#include <akonadi/itemview.h>
#include <akonadi/standardactionmanager.h>
#include <akonadi/agenttypedialog.h>
#include <akonadi/agentinstancewidget.h>
#include <akonadi/agentmanager.h>
#include <akonadi/agentinstancecreatejob.h>
#include <akonadi/agentfilterproxymodel.h>
#include <akonadi/control.h>
#include <akonadi/itemfetchscope.h>

#include <KCal/Incidence>

#include "kotodoeditor.h"
#include "koeventeditor.h"
#include "kojournaleditor.h"

class CalItemModel : public Akonadi::ItemModel
{
  public:
    explicit CalItemModel( QObject *parent = 0 ) : Akonadi::ItemModel( parent )
    {
      fetchScope().fetchFullPayload();
    }

    virtual ~CalItemModel() {}

    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const
    {
      return ItemModel::rowCount(parent);
    }

    virtual int columnCount( const QModelIndex &parent = QModelIndex() ) const
    {
      return 4;
    }

    virtual QVariant data( const QModelIndex & index, int role = Qt::DisplayRole ) const
    {
      if ( role == Qt::DisplayRole ) {
        const Akonadi::Item item = itemForIndex( index );
        const Incidence::Ptr incidence = item.hasPayload<Incidence::Ptr>() ?
                                         item.payload<Incidence::Ptr>() :
                                         Incidence::Ptr();
        if ( !incidence ) {
          return QVariant();
        }
        switch( index.column() ) {
        case 0:
          return incidence->type();
          break;
        case 1:
          return incidence->dtStart().toString();
          break;
        case 2:
          return incidence->dtEnd().toString();
          break;
        case 3:
          return incidence->summary();
          break;
        }
      }
      return Akonadi::ItemModel::data( index, role );
    }

    virtual QVariant headerData( int section, Qt::Orientation orientation,
                                 int role = Qt::DisplayRole ) const
    {
      if ( role == Qt::DisplayRole ) {
        switch( section ) {
        case 0:
          return QLatin1String( "Type" );
          break;
        case 1:
          return QLatin1String( "Start" );
          break;
        case 2:
          return QLatin1String( "End" );
          break;
        case 3:
          return QLatin1String( "Summary" );
          break;
        }
      }
      return Akonadi::ItemModel::headerData( section, orientation, role );
    }
    virtual QStringList mimeTypes() const
    {
      return QStringList()
        << QLatin1String( "text/uri-list" )
        << QLatin1String( "application/x-vnd.akonadi.calendar.event" )
        << QLatin1String( "application/x-vnd.akonadi.calendar.todo" )
        << QLatin1String( "application/x-vnd.akonadi.calendar.journal" )
        << QLatin1String( "application/x-vnd.akonadi.calendar.freebusy" );
    }
};

class MainWidget : public QWidget
{
  Q_OBJECT
  public:
    explicit MainWidget( QWidget *parent )
      : QWidget(parent),
        m_collectionmodel( new Akonadi::CollectionModel( this ) ),
        m_collectionproxymodel( new Akonadi::CollectionFilterProxyModel( this ) ),
        m_itemmodel( new CalItemModel( this ) )
    {
      m_collectionproxymodel->setSourceModel(m_collectionmodel);
      m_collectionproxymodel->addMimeTypeFilter( QString::fromLatin1( "text/calendar" ) );

      Akonadi::ItemFetchScope fetchscope;
      fetchscope.fetchFullPayload( true );
      m_itemmodel->setFetchScope( fetchscope );

      QLayout *layout = new QVBoxLayout( this );
      layout->setMargin( 0 );
      layout->setSpacing( 0 );
      this->setLayout( layout );

      QSplitter *splitter = new QSplitter( this );
      layout->addWidget( splitter );
      m_collectionview = new Akonadi::CollectionView( splitter );
      m_collectionview->header()->hide();
      m_collectionview->setModel( m_collectionproxymodel );
      m_collectionview->setRootIsDecorated( true );

      QWidget *mainwidget = new QWidget(splitter);
      QLayout *mainlayout = new QVBoxLayout(mainwidget);
      mainlayout->setMargin( 0 );
      mainlayout->setSpacing( 0 );
      mainwidget->setLayout( mainlayout );

      QLineEdit *edit = new QLineEdit( this );
      connect( edit, SIGNAL(textChanged(QString)),
               this, SLOT(filterChanged(QString)) );
      mainlayout->addWidget( edit );

      m_itemview = new Akonadi::ItemView( mainwidget );
      mainlayout->addWidget( m_itemview );
      m_itemproxymodel = new QSortFilterProxyModel( m_itemview );
      m_itemproxymodel->setFilterKeyColumn( 3 );
      m_itemproxymodel->setSourceModel( m_itemmodel );
      m_itemview->setModel( m_itemproxymodel );

      splitter->setStretchFactor( 1, 1 );
      selectionChanged();
      connect( m_collectionview->selectionModel(),
               SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
               this, SLOT(selectionChanged()) );
      connect( m_itemview, SIGNAL(activated(QModelIndex)),
               this, SLOT(itemActivated()) );
    }
    virtual ~MainWidget() {}

  private Q_SLOTS:

    void filterChanged( const QString &text )
    {
      m_itemproxymodel->setFilterWildcard( text );
    }

    void selectionChanged()
    {
      if ( m_collectionview->selectionModel()->hasSelection() ) {
        QModelIndex index = m_collectionview->selectionModel()->currentIndex();
        Q_ASSERT( index.isValid() );
        Akonadi::Collection collection =
          index.model()->data(
            index, Akonadi::CollectionModel::CollectionRole ).value<Akonadi::Collection>();
        Q_ASSERT( collection.isValid() );
        m_itemmodel->setCollection( collection );
      } else {
        m_itemmodel->setCollection( Akonadi::Collection::root() );
      }
    }

    void itemActivated()
    {
      QModelIndex index = m_itemview->selectionModel()->currentIndex();
      Q_ASSERT( index.isValid() );
      Akonadi::Item item =
        index.model()->data(
          index, Akonadi::ItemModel::ItemRole ).value<Akonadi::Item>();
      //const Akonadi::Item::Id uid = item.id();
      Q_ASSERT( item.isValid() );
      Q_ASSERT( item.hasPayload<KCal::Incidence::Ptr>() );
      const KCal::Incidence::Ptr incidence = item.payload<KCal::Incidence::Ptr>();
      kDebug() << "Add akonadi id=" << item.id() << "uid=" << incidence->uid()
               << "summary=" << incidence->summary() << "type=" << incidence->type();

      if ( incidence->type() == "Event" ) {
        KOEventEditor *editor = new KOEventEditor( this );
        editor->init();
        editor->editIncidence( item, QDate() );
        editor->show();
      } else if( incidence->type() == "Todo" ) {
        KOTodoEditor *editor = new KOTodoEditor( this );
        editor->init();
        /*
        createCategoryEditor();
        connect( editor, SIGNAL(deleteIncidenceSignal(Incidence *)),
                 mMainView, SLOT(deleteIncidence(Incidence *)) );
        connect( mCategoryEditDialog, SIGNAL(categoryConfigChanged()),
                 editor, SIGNAL(updateCategoryConfig()) );
        connect( editor, SIGNAL(editCategories()),
                 mCategoryEditDialog, SLOT(show()) );
        connect( editor, SIGNAL(dialogClose(Incidence *)),
                 mMainView, SLOT(dialogClosing(Incidence *)) );
        connect( mMainView, SIGNAL(closingDown()), editor, SLOT(reject()) );
        connect( editor, SIGNAL(deleteAttendee(Incidence *)),
                 mMainView, SIGNAL(cancelAttendees(Incidence *)) );
        */
        editor->editIncidence( item, QDate() );
        editor->show();

      } else if( incidence->type() == "Journal" ) {
        KOJournalEditor *editor = new KOJournalEditor( this );
        editor->init();
        editor->editIncidence( item, QDate() );
        editor->show();
      } else {
        Q_ASSERT( false );
      }
    }

  private:
    Akonadi::CollectionModel *m_collectionmodel;
    Akonadi::CollectionFilterProxyModel *m_collectionproxymodel;
    Akonadi::CollectionView *m_collectionview;
    CalItemModel *m_itemmodel;
    QSortFilterProxyModel *m_itemproxymodel;
    Akonadi::ItemView *m_itemview;
};

int main( int argc, char **argv )
{
  KAboutData about( "incidenceeditorapp",
                    "korganizer",
                    ki18n( "IncidenceEditorApp" ),
                    "0.1",
                    ki18n( "KDE application to run the KOrganizer incidenceeditor." ),
                    KAboutData::License_LGPL,
                    ki18n( "(C) 2009 Sebastian Sauer" ),
                    ki18n( "Run the KOrganizer incidenceeditor." ),
                    "http://kdepim.kde.org",
                    "kdepim@kde.org" );
  about.addAuthor( ki18n( "Sebastian Sauer" ), ki18n( "Author" ), "sebsauer@kdab.net" );
  KCmdLineArgs::init( argc, argv, &about );
  KApplication app;
  KMainWindow *mainwindow = new KMainWindow();
  mainwindow->setCentralWidget( new MainWidget( mainwindow ) );
  mainwindow->resize( QSize( 800, 600 ).expandedTo( mainwindow->minimumSizeHint() ) );
  mainwindow->show();
  return app.exec();
}

#include "main.moc"
