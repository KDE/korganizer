/*
    This file is part of KOrganizer.
    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

// $Id$

#include <qcolor.h>

#include <kconfig.h>
#include <kstandarddirs.h>
#include <kglobal.h>
#include <kdebug.h>

#include "kprefs.h"

class KPrefsItemBool : public KPrefsItem {
  public:
    KPrefsItemBool(const QString &group,const QString &name,bool *,bool defaultValue=true);
    virtual ~KPrefsItemBool() {}
    
    void setDefault();
    void readConfig(KConfig *);
    void writeConfig(KConfig *);

  private:
    bool *mReference;
    bool mDefault;
};

class KPrefsItemInt : public KPrefsItem {
  public:
    KPrefsItemInt(const QString &group,const QString &name,int *,int defaultValue=0);
    virtual ~KPrefsItemInt() {}
    
    void setDefault();
    void readConfig(KConfig *);
    void writeConfig(KConfig *);

  private:
    int *mReference;
    int mDefault;
};


class KPrefsItemColor : public KPrefsItem {
  public:
    KPrefsItemColor(const QString &group,const QString &name,QColor *,
                    const QColor &defaultValue=QColor(128,128,128));
    virtual ~KPrefsItemColor() {}
    
    void setDefault();
    void readConfig(KConfig *);
    void writeConfig(KConfig *);    

  private:
    QColor *mReference;
    QColor mDefault;
};


class KPrefsItemFont : public KPrefsItem {
  public:
    KPrefsItemFont(const QString &group,const QString &name,QFont *,
                   const QFont &defaultValue=QFont("helvetica",12));
    virtual ~KPrefsItemFont() {}
    
    void setDefault();
    void readConfig(KConfig *);
    void writeConfig(KConfig *);    

  private:
    QFont *mReference;
    QFont mDefault;
};


class KPrefsItemString : public KPrefsItem {
  public:
    KPrefsItemString(const QString &group,const QString &name,QString *,
                     const QString &defaultValue="");
    virtual ~KPrefsItemString() {}
    
    void setDefault();
    void readConfig(KConfig *);
    void writeConfig(KConfig *);    

  private:
    QString *mReference;
    QString mDefault;
};


class KPrefsItemStringList : public KPrefsItem {
  public:
    KPrefsItemStringList(const QString &group,const QString &name,QStringList *,
                         const QStringList &defaultValue=QStringList());
    virtual ~KPrefsItemStringList() {}
    
    void setDefault();
    void readConfig(KConfig *);
    void writeConfig(KConfig *);    

  private:
    QStringList *mReference;
    QStringList mDefault;
};


class KPrefsItemIntList : public KPrefsItem {
  public:
    KPrefsItemIntList(const QString &group,const QString &name,QValueList<int> *,
                      const QValueList<int> &defaultValue=QValueList<int>());
    virtual ~KPrefsItemIntList() {}
    
    void setDefault();
    void readConfig(KConfig *);
    void writeConfig(KConfig *);    

  private:
    QValueList<int> *mReference;
    QValueList<int> mDefault;
};


KPrefsItemBool::KPrefsItemBool(const QString &group,const QString &name,
                               bool *reference,bool defaultValue) :
  KPrefsItem(group,name)
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


KPrefsItemInt::KPrefsItemInt(const QString &group,const QString &name,
                             int *reference,int defaultValue) :
  KPrefsItem(group,name)
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


KPrefsItemColor::KPrefsItemColor(const QString &group,const QString &name,
                                 QColor *reference,const QColor &defaultValue) :
  KPrefsItem(group,name)
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


KPrefsItemFont::KPrefsItemFont(const QString &group,const QString &name,
                               QFont *reference,const QFont &defaultValue) :
  KPrefsItem(group,name)
{
  mReference = reference;
  mDefault = defaultValue;
}

void KPrefsItemFont::setDefault()
{
  *mReference = mDefault;
}

void KPrefsItemFont::writeConfig(KConfig *config)
{
  config->setGroup(mGroup);
  config->writeEntry(mName,*mReference);
}

void KPrefsItemFont::readConfig(KConfig *config)
{
  config->setGroup(mGroup);
  *mReference = config->readFontEntry(mName,&mDefault);
}


KPrefsItemString::KPrefsItemString(const QString &group,const QString &name,
                                   QString *reference,const QString &defaultValue) :
  KPrefsItem(group,name)
{
  mReference = reference;
  mDefault = defaultValue;
}

void KPrefsItemString::setDefault()
{
  *mReference = mDefault;
}

void KPrefsItemString::writeConfig(KConfig *config)
{
  config->setGroup(mGroup);
  config->writeEntry(mName,*mReference);
}

void KPrefsItemString::readConfig(KConfig *config)
{
  config->setGroup(mGroup);
  *mReference = config->readEntry(mName,mDefault);
}


KPrefsItemStringList::KPrefsItemStringList(const QString &group,const QString &name,
                                           QStringList *reference,const QStringList &defaultValue) :
  KPrefsItem(group,name)
{
  mReference = reference;
  mDefault = defaultValue;
}

void KPrefsItemStringList::setDefault()
{
  *mReference = mDefault;
}

void KPrefsItemStringList::writeConfig(KConfig *config)
{
  config->setGroup(mGroup);
  config->writeEntry(mName,*mReference);
}

void KPrefsItemStringList::readConfig(KConfig *config)
{
  config->setGroup(mGroup);
  *mReference = config->readListEntry(mName);
}


KPrefsItemIntList::KPrefsItemIntList(const QString &group,const QString &name,
                                     QValueList<int> *reference,const QValueList<int> &defaultValue) :
  KPrefsItem(group,name)
{
  mReference = reference;
  mDefault = defaultValue;
}

void KPrefsItemIntList::setDefault()
{
  *mReference = mDefault;
}

void KPrefsItemIntList::writeConfig(KConfig *config)
{
  config->setGroup(mGroup);
  config->writeEntry(mName,*mReference);
}

void KPrefsItemIntList::readConfig(KConfig *config)
{
  config->setGroup(mGroup);
  *mReference = config->readIntListEntry(mName);
}


QString *KPrefs::mCurrentGroup = 0;

KPrefs::KPrefs(const QString &configname)
{
  if (!configname.isEmpty()) {
    mConfig = new KConfig(locateLocal("config",configname));
  } else {
    mConfig = KGlobal::config();
  }

  mItems.setAutoDelete(true);

  // Set default group
  if (mCurrentGroup == 0) mCurrentGroup = new QString("No Group");
}

KPrefs::~KPrefs()
{
  if (mConfig != KGlobal::config()) {
    delete mConfig;
  }
}

void KPrefs::setCurrentGroup(const QString &group)
{
  if (mCurrentGroup) delete mCurrentGroup;
  mCurrentGroup = new QString(group);
}

KConfig *KPrefs::config() const
{
  return mConfig;
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
  KPrefsItem *item;
  for(item = mItems.first();item;item = mItems.next()) {
    item->writeConfig(mConfig);
  }

  usrWriteConfig();

  mConfig->sync();
}


void KPrefs::addItem(KPrefsItem *item)
{
  mItems.append(item);
}

void KPrefs::addItemBool(const QString &key,bool *reference,bool defaultValue)
{
  addItem(new KPrefsItemBool(*mCurrentGroup,key,reference,defaultValue));
}

void KPrefs::addItemInt(const QString &key,int *reference,int defaultValue)
{
  addItem(new KPrefsItemInt(*mCurrentGroup,key,reference,defaultValue));
}

void KPrefs::addItemColor(const QString &key,QColor *reference,const QColor &defaultValue)
{
  addItem(new KPrefsItemColor(*mCurrentGroup,key,reference,defaultValue));
}

void KPrefs::addItemFont(const QString &key,QFont *reference,const QFont &defaultValue)
{
  addItem(new KPrefsItemFont(*mCurrentGroup,key,reference,defaultValue));
}

void KPrefs::addItemString(const QString &key,QString *reference,const QString &defaultValue)
{
  addItem(new KPrefsItemString(*mCurrentGroup,key,reference,defaultValue));
}

void KPrefs::addItemStringList(const QString &key,QStringList *reference,
                               const QStringList &defaultValue)
{
  addItem(new KPrefsItemStringList(*mCurrentGroup,key,reference,defaultValue));
}

void KPrefs::addItemIntList(const QString &key,QValueList<int> *reference,
                            const QValueList<int> &defaultValue)
{
  addItem(new KPrefsItemIntList(*mCurrentGroup,key,reference,defaultValue));
}
