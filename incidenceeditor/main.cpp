#include <qheaderview.h>
#include <qsplitter.h>
#include <QHBoxLayout>
#include <kapplication.h>
#include <kmainwindow.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kurl.h>
#include <ksystemtimezone.h>

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

#include "akonadicalendar.h"
#include "calendarbase.h"

#include "kotodoeditor.h"

class MainWidget : public QWidget
{
    Q_OBJECT
  public:
    explicit MainWidget(QWidget *parent)
      : QWidget(parent)
      , m_collectionmodel(new Akonadi::CollectionModel(this))
      , m_collectionproxymodel(new Akonadi::CollectionFilterProxyModel(this))
      , m_itemmodel(new Akonadi::ItemModel(this))
    {
      m_collectionproxymodel->setSourceModel(m_collectionmodel);
      m_collectionproxymodel->addMimeTypeFilter( QString::fromLatin1( "text/calendar" ) );

      Akonadi::ItemFetchScope fetchscope;
      fetchscope.fetchFullPayload(true);
      m_itemmodel->setFetchScope(fetchscope);

      QLayout *layout = new QHBoxLayout(this);
      this->setLayout(layout);

      QSplitter *splitter = new QSplitter(this);
      layout->addWidget(splitter);
      m_collectionview = new Akonadi::CollectionView(splitter);
      m_collectionview->header()->hide();
      m_collectionview->setModel(m_collectionproxymodel);
      m_collectionview->setRootIsDecorated(true);

      m_itemview = new Akonadi::ItemView(splitter);
      m_itemview->setModel(m_itemmodel);
      
/*
      //mCollectionview->setSelectionMode( QAbstractItemView::NoSelection );
      KXMLGUIClient *xmlclient = KOCore::self()->xmlguiClient( mFactory->view() );
      if( xmlclient ) {
        mCollectionview->setXmlGuiClient( xmlclient );

        mActionManager = new Akonadi::StandardActionManager( xmlclient->actionCollection(), mCollectionview );
        mActionManager->createAllActions();
        mActionManager->action( Akonadi::StandardActionManager::CreateCollection )->setText( i18n( "Add Calendar..." ) );
        mActionManager->setActionText( Akonadi::StandardActionManager::CopyCollections, ki18np( "Copy Calendar", "Copy %1 Calendars" ) );
        mActionManager->action( Akonadi::StandardActionManager::DeleteCollections )->setText( i18n( "Delete Calendar" ) );
        mActionManager->action( Akonadi::StandardActionManager::SynchronizeCollections )->setText( i18n( "Reload" ) );
        mActionManager->action( Akonadi::StandardActionManager::CollectionProperties )->setText( i18n( "Properties..." ) );
        mActionManager->setCollectionSelectionModel( mCollectionview->selectionModel() );

        mCreateAction = new KAction( mCollectionview );
        mCreateAction->setIcon( KIcon( "appointment-new" ) );
        mCreateAction->setText( i18n( "New Calendar..." ) );
        //mCreateAction->setWhatsThis( i18n( "Create a new contact<p>You will be presented with a dialog where you can add all data about a person, including addresses and phone numbers.</p>" ) );
        xmlclient->actionCollection()->addAction( QString::fromLatin1( "akonadi_calendar_create" ), mCreateAction );
        connect( mCreateAction, SIGNAL( triggered( bool ) ), this, SLOT( newCalendar() ) );

        mDeleteAction = new KAction( mCollectionview );
        mDeleteAction->setIcon( KIcon( "edit-delete" ) );
        mDeleteAction->setText( i18n( "Delete Calendar" ) );
        mDeleteAction->setEnabled( false );
        //mDeleteAction->setWhatsThis( i18n( "Create a new contact<p>You will be presented with a dialog where you can add all data about a person, including addresses and phone numbers.</p>" ) );
        xmlclient->actionCollection()->addAction( QString::fromLatin1( "akonadi_calendar_delete" ), mDeleteAction );
        connect( mDeleteAction, SIGNAL( triggered( bool ) ), this, SLOT( deleteCalendar() ) );
      }
*/
      selectionChanged();
      connect( m_collectionview->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(selectionChanged()) );
      connect( m_itemview, SIGNAL(activated(QModelIndex)), this, SLOT(itemActivated()) );
    }
    virtual ~MainWidget() {}

  private Q_SLOTS:
    void selectionChanged() {
      if(m_collectionview->selectionModel()->hasSelection()) {
        QModelIndex index = m_collectionview->selectionModel()->currentIndex();
        Q_ASSERT( index.isValid() );
        Akonadi::Collection collection = index.model()->data( index, Akonadi::CollectionModel::CollectionRole ).value<Akonadi::Collection>();
        Q_ASSERT( collection.isValid() );
        m_itemmodel->setCollection( collection );
      } else {
        m_itemmodel->setCollection( Akonadi::Collection::root() );
      }
    }
    void itemActivated() {
      QModelIndex index = m_itemview->selectionModel()->currentIndex();
      Q_ASSERT( index.isValid() );
      Akonadi::Item item = index.model()->data( index, Akonadi::ItemModel::ItemRole ).value<Akonadi::Item>();
      //const Akonadi::Item::Id uid = item.id();
      Q_ASSERT( item.isValid() );
      Q_ASSERT( item.hasPayload<KCal::Incidence::Ptr>() );
      const KCal::Incidence::Ptr incidence = item.payload<KCal::Incidence::Ptr>();
      kDebug() << "Add akonadi id=" << item.id() << "uid=" << incidence->uid() << "summary=" << incidence->summary() << "type=" << incidence->type();

      KOrg::CalendarBase *calendar = new KOrg::AkonadiCalendar(KSystemTimeZones::local());
      
      if(incidence->type() == "Event") {
        //KOEventEditor
      } else if(incidence->type() == "Todo") {
        KOTodoEditor *editor = new KOTodoEditor(calendar, this);
        editor->init();
        /*
        createCategoryEditor();
        connect( editor, SIGNAL(deleteIncidenceSignal(Incidence *)), mMainView, SLOT(deleteIncidence(Incidence *)) );
        connect( mCategoryEditDialog, SIGNAL(categoryConfigChanged()), editor, SIGNAL(updateCategoryConfig()) );
        connect( editor, SIGNAL(editCategories()), mCategoryEditDialog, SLOT(show()) );
        connect( editor, SIGNAL(dialogClose(Incidence *)), mMainView, SLOT(dialogClosing(Incidence *)) );
        connect( editor, SIGNAL(editCanceled(Incidence *)), mMainView, SLOT(editCanceled(Incidence *)) );
        connect( mMainView, SIGNAL(closingDown()), editor, SLOT(reject()) );
        connect( editor, SIGNAL(deleteAttendee(Incidence *)), mMainView, SIGNAL(cancelAttendees(Incidence *)) );
           */
        //Todo *todo = dynamic_cast<Todo*>(incidence);
//editor->editIncidence(incidence.get(), calendar);
        editor->show();
        
      } else if(incidence->type() == "Journal") {
        //KOJournalEditor
      } else {
        Q_ASSERT(false);
      }
        
      
    }
    
  private:
    Akonadi::CollectionModel *m_collectionmodel;
    Akonadi::CollectionFilterProxyModel *m_collectionproxymodel;
    Akonadi::CollectionView *m_collectionview;
    Akonadi::ItemModel *m_itemmodel;
    Akonadi::ItemView *m_itemview;
    //AkonadiCalendar m_calendar;
};

int main(int argc, char **argv)
{
    KAboutData about("incidenceeditorapp",
                     "korganizer",
                     ki18n("IncidenceEditorApp"),
                     "0.1",
                     ki18n("KDE application to run the KOrganizer incidenceeditor."),
                     KAboutData::License_LGPL,
                     ki18n("(C) 2009 Sebastian Sauer"),
                     ki18n("Run the KOrganizer incidenceeditor."),
                     "http://kdepim.kde.org",
                     "kdepim@kde.org");
    about.addAuthor(ki18n("Sebastian Sauer"), ki18n("Author"), "sebsauer@kdab.net");

    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineOptions options;
    //options.add("+file", ki18n("Some file"));
    KCmdLineArgs::addCmdLineOptions(options);
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    KApplication app;
    KMainWindow *mainwindow = new KMainWindow();
    mainwindow->setCentralWidget( new MainWidget(mainwindow) );
    mainwindow->resize(QSize(1200, 400).expandedTo(minimumSizeHint()));
    mainwindow->show();
    return app.exec();
}

#include "main.moc"
