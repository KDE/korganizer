// $Id$
// Dialog for selecting and configuring KOrganizer plugins
// (c) 2001 Cornelius Schumacher

#include <qlayout.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistview.h>

#include <klocale.h>
#include <kbuttonbox.h>
#include <kmessagebox.h>
#include <ktrader.h>

#include "kocore.h"
#include "koprefs.h"

#include "plugindialog.h"
#include "plugindialog.moc"

class PluginItem : public QCheckListItem {
  public:
    PluginItem( QListView *parent, KService::Ptr service ) :
      QCheckListItem( parent, service->name(), QCheckListItem::CheckBox ), mService( service )
    {
    }
    
    KService::Ptr service() { return mService; } 
    
  private:
    KService::Ptr mService;
};


PluginDialog::PluginDialog(QWidget *parent)
  : KDialogBase(Plain,i18n("Configure Plugins"),Ok|Cancel|User1,Ok,parent,0,false,false,
                i18n("Configure.."))
{
  QFrame *topFrame = plainPage();
  QVBoxLayout *topLayout = new QVBoxLayout(topFrame,0,spacingHint());

  mListView = new QListView(topFrame);
  mListView->addColumn(i18n("Name"));
  topLayout->addWidget(mListView);
  connect(mListView,SIGNAL(selectionChanged()),SLOT(checkSelection()));

  KTrader::OfferList plugins = KOCore::self()->availablePlugins("Calendar/Plugin");
  
  QStringList selectedPlugins = KOPrefs::instance()->mSelectedPlugins;
  
  KTrader::OfferList::ConstIterator it;
  for(it = plugins.begin(); it != plugins.end(); ++it) {
    QCheckListItem *item = new PluginItem(mListView,*it);
    if ( selectedPlugins.find((*it)->desktopEntryName()) != selectedPlugins.end() ) {
      item->setOn(true);
    }
  }

  checkSelection();
  
  connect(this,SIGNAL(user1Clicked()),SLOT(configure()));
}

PluginDialog::~PluginDialog()
{
}

void PluginDialog::slotOk()
{
  QStringList selectedPlugins;

  PluginItem *item = (PluginItem *)mListView->firstChild();
  while(item) {
    if(item->isOn()) {
      selectedPlugins.append(item->service()->desktopEntryName());
    }
    item = (PluginItem *)item->nextSibling();
  }
  
  KOPrefs::instance()->mSelectedPlugins = selectedPlugins;
  KOCore::self()->reloadPlugins();
  
  accept();
  
  emit configChanged();
}

void PluginDialog::configure()
{
  PluginItem *item = (PluginItem *)mListView->selectedItem();
  if (!item) return;

  KOrg::Plugin *plugin = KOCore::self()->loadPlugin(item->service());

  plugin->configure(this);
}

void PluginDialog::checkSelection()
{
  if (mListView->selectedItem()) {
    enableButton(User1,true);
  } else {
    enableButton(User1,false);
  }
}
