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

// Dialog for selecting and configuring KOrganizer plugins

#include <qlayout.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistview.h>

#include <klocale.h>
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
                i18n("Configure..."))
{
  QFrame *topFrame = plainPage();
  QVBoxLayout *topLayout = new QVBoxLayout(topFrame,0,spacingHint());

  mListView = new QListView(topFrame);
  mListView->addColumn(i18n("Name"));
  topLayout->addWidget(mListView);
  connect(mListView,SIGNAL(selectionChanged()),SLOT(checkSelection()));

  KTrader::OfferList plugins = KOCore::self()->availablePlugins("Calendar/Plugin");
  plugins += KOCore::self()->availablePlugins("KOrganizer/Part");
  
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
  mMainView = dynamic_cast<CalendarView *>(parent);
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
  emit configChanged();
  if ( mMainView ) mMainView->updateView();

  accept();

}

void PluginDialog::configure()
{
  PluginItem *item = (PluginItem *)mListView->selectedItem();
  if ( !item ) return;

  KOrg::Plugin *plugin = KOCore::self()->loadPlugin( item->service() );

  if ( plugin ) {
    plugin->configure( this );
    delete plugin;
  } else {
    KMessageBox::sorry( this, i18n( "Unable to configure this plugin" ) );
  }

}

void PluginDialog::checkSelection()
{
  if (mListView->selectedItem()) {
    enableButton(User1,true);
  } else {
    enableButton(User1,false);
  }
}
