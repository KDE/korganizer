// $Id$

#include <kconfig.h>
#include <kstddirs.h>
#include <kglobal.h>
#include <kdebug.h>

#include "kprefs.h"

QString *KPrefsItem::mCurrentGroup = 0;

KPrefsItem::KPrefsItem(const QString &name)
{
  mName = name;
  
  if (mCurrentGroup == 0) mGroup = "No Group";
  else mGroup = *mCurrentGroup;
}

void KPrefsItem::setCurrentGroup(const QString &group)
{
  if (mCurrentGroup) delete mCurrentGroup;
  mCurrentGroup = new QString(group);
}


KPrefsItemBool::KPrefsItemBool(const QString &name,
                               bool *reference,bool defaultValue) :
  KPrefsItem(name)
{
  mReference = reference;
  mDefault = defaultValue;
}

void KPrefsItemBool::setDefault()
{
  *mReference = mDefault;
}

void KPrefsItemBool::writeConfig(KConfig *config)
{
  config->setGroup(mGroup);            
  config->writeEntry(mName,*mReference);
}


void KPrefsItemBool::readConfig(KConfig *config)
{
  config->setGroup(mGroup);
  *mReference = config->readBoolEntry(mName,mDefault);
}


KPrefsItemInt::KPrefsItemInt(const QString &name,
                             int *reference,int defaultValue) :
  KPrefsItem(name)
{
  mReference = reference;
  mDefault = defaultValue;
}

void KPrefsItemInt::setDefault()
{
  *mReference = mDefault;
}

void KPrefsItemInt::writeConfig(KConfig *config)
{
  config->setGroup(mGroup);            
  config->writeEntry(mName,*mReference);
}

void KPrefsItemInt::readConfig(KConfig *config)
{
  config->setGroup(mGroup);
  *mReference = config->readNumEntry(mName,mDefault);
}


KPrefsItemColor::KPrefsItemColor(const QString &name,
                                 QColor *reference,QColor defaultValue) :
  KPrefsItem(name)
{
  mReference = reference;
  mDefault = defaultValue;
}

void KPrefsItemColor::setDefault()
{
  *mReference = mDefault;
}

void KPrefsItemColor::writeConfig(KConfig *config)
{
  config->setGroup(mGroup);            
  config->writeEntry(mName,*mReference);
}

void KPrefsItemColor::readConfig(KConfig *config)
{
  config->setGroup(mGroup);
  *mReference = config->readColorEntry(mName,&mDefault);
}



KPrefs::KPrefs(const QString &configname)
{
  kdDebug() << "KPrefs::KPrefs(): locate: " << locate("config",configname)
            << endl;
  kdDebug() << "KPrefs::KPrefs(): locateLocal: "
            << locateLocal("config",configname) << endl;

  if (!configname.isEmpty()) {
    mConfig = new KConfig(locateLocal("config",configname));
  } else {
    mConfig = KGlobal::config();
  }

  mItems.setAutoDelete(true);
}

KPrefs::~KPrefs()
{
  kdDebug() << "KPrefs::~KPrefs()" << endl;

  if (mConfig != KGlobal::config()) {
    delete mConfig;
  }
}

void KPrefs::setDefaults()
{
  KPrefsItem *item;
  for(item = mItems.first();item;item = mItems.next()) {
    item->setDefault();
  }

  usrSetDefaults();
}

void KPrefs::readConfig()
{
  KPrefsItem *item;
  for(item = mItems.first();item;item = mItems.next()) {
    item->readConfig(mConfig);
  }

  usrReadConfig();
}

void KPrefs::writeConfig()
{
//  kdDebug() << "KPrefs::writeConfig()" << endl;

  KPrefsItem *item;
  for(item = mItems.first();item;item = mItems.next()) {
    item->writeConfig(mConfig);
  }

  usrWriteConfig();
  
  mConfig->sync();

//  kdDebug() << "KPrefs::writeConfig() done" << endl;
}

void KPrefs::addPrefsItem(KPrefsItem *item)
{
//  kdDebug() << "KPrefs::addPrefsItem()" << endl;

  mItems.append(item);

//  kdDebug() << "KPrefs::addPrefsItem(): done" << endl;
}
