/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef _KPREFS_H
#define _KPREFS_H
// $Id$

#include <qptrlist.h>
#include <qcolor.h>
#include <qfont.h>
#include <qstringlist.h>

class KConfig;

/**
  @short Class for storing a preferences setting
  @author Cornelius Schumache
  @see KPref
  
  This class represents one preferences setting as used by @ref KPrefs.
  Subclasses of KPrefsItem implement storage functions for a certain type of
  setting. Normally you don't have to use this class directly. Use the special
  addItem() functions of KPrefs instead. If you subclass this class you will
  have to register instances with the function KPrefs::addItem().
*/
class KPrefsItem {
  public:
    /**
      Constructor.
      
      @param group Config file group.
      @param name Config file key.
    */
    KPrefsItem(const QString &group,const QString &name) :
      mGroup(group),mName(name) {}
    /**
      Destructor.
    */
    virtual ~KPrefsItem() {}
    
    /**
      This function is called by @ref KPrefs to set this setting to its default
      value.
    */
    virtual void setDefault() = 0;
    /**
      This function is called by @ref KPrefs to read the value for this setting
      from a config file.
      value.
    */
    virtual void readConfig(KConfig *) = 0;
    /**
      This function is called by @ref KPrefs to write the value of this setting
      to a config file.
    */
    virtual void writeConfig(KConfig *) = 0;

  protected:
    QString mGroup;
    QString mName;
};

/**
  @short Class for handling preferences settings for an application.
  @author Cornelius Schumacher
  @see KPrefsItem

  This class provides an interface to preferences settings. Preferences items
  can be registered by the addItem() function corresponding to the data type of
  the seetting. KPrefs then handles reading and writing of config files and
  setting of default values.

  Normally you will subclass KPrefs, add data members for the preferences
  settings and register the members in the constructor of the subclass.
  
  Example:
  <pre>
  class MyPrefs : public KPrefs {
    public:
      MyPrefs()
      {
        setCurrentGroup("MyGroup");
        addItemBool("MySetting1",&mMyBool,false);
        addItemColor("MySetting2",&mMyColor,QColor(1,2,3));
        
        setCurrentGroup("MyOtherGroup");
        addItemFont("MySetting3",&mMyFont,QFont("helvetica",12));
      }
      
      bool mMyBool;
      QColor mMyColor;
      QFont mMyFont;
  }
  </pre>
  
  It might be convenient in many cases to make this subclass of KPrefs a
  singleton for global access from all over the application without passing
  references to the KPrefs object around.

  You can set all values to default values by calling @ref setDefaults(), write
  the data to the configuration file by calling @ref writeConfig() and read the
  data from the configuration file by calling @ref readConfig().

  If you have items, which are not covered by the existing addItem() functions
  you can add customized code for reading, writing and default setting by
  implementing the functions @ref usrSetDefaults(), @ref usrReadConfig() and
  @ref usrWriteConfig().
  
  Internally preferences settings are stored in instances of subclasses of
  @ref KPrefsItem. You can also add KPrefsItem subclasses for your own types
  and call the generic @ref addItem() to register them.
*/
  
class KPrefs {
  public:
    /**
      Constructor.
      
      @param configname name of config file. If no name is given, the default
                        config file as returned by kapp()->config() is used.
    */
    KPrefs(const QString &configname=QString::null);
    /**
      Destructor
    */
    virtual ~KPrefs();
  
    /**
      Set preferences to default values. All registered items are set to their
      default values.
    */
    void setDefaults();
  
    /**
      Read preferences from config file. All registered items are set to the
      values read from disk.
    */
    void readConfig();

    /**
      Write preferences to config file. The values of all registered items are
      written to disk.
    */
    void writeConfig();

    /**
      Set the config file group for subsequent addItem() calls. It is valid
      until setCurrentGroup() is called with a new argument. Call this before
      you add any items. The default value is "No Group".
    */
    static void setCurrentGroup(const QString &group);

    /**
      Register a custom @ref KPrefsItem.
    */
    void addItem(KPrefsItem *);

    /**
      Register an item of type bool.
      
      @param key Key used in config file.
      @param reference Pointer to the variable, which is set by readConfig()
                       and setDefaults() calls and read by writeConfig() calls.
      @param defaultValue Default value, which is used by setDefaults() and
                          when the config file does not yet contain the key of
                          this item.
    */
    void addItemBool(const QString &key,bool *reference,
                     bool defaultValue=false);
    /**
      Register an item of type int.
      
      @param key Key used in config file.
      @param reference Pointer to the variable, which is set by readConfig()
                       and setDefaults() calls and read by writeConfig() calls.
      @param defaultValue Default value, which is used by setDefaults() and
                          when the config file does not yet contain the key of
                          this item.
    */
    void addItemInt(const QString &key,int *reference,
                    int defaultValue=0);
    /**
      Register an item of type QColor.
      
      @param key Key used in config file.
      @param reference Pointer to the variable, which is set by readConfig()
                       and setDefaults() calls and read by writeConfig() calls.
      @param defaultValue Default value, which is used by setDefaults() and
                          when the config file does not yet contain the key of
                          this item.
    */
    void addItemColor(const QString &key,QColor *reference,
                      const QColor &defaultValue=QColor(128,128,128));
    /**
      Register an item of type QFont.
      
      @param key Key used in config file.
      @param reference Pointer to the variable, which is set by readConfig()
                       and setDefaults() calls and read by writeConfig() calls.
      @param defaultValue Default value, which is used by setDefaults() and
                          when the config file does not yet contain the key of
                          this item.
    */
    void addItemFont(const QString &key,QFont *reference,
                     const QFont &defaultValue=QFont("helvetica",12));
    /**
      Register an item of type QString.
      
      @param key Key used in config file.
      @param reference Pointer to the variable, which is set by readConfig()
                       and setDefaults() calls and read by writeConfig() calls.
      @param defaultValue Default value, which is used by setDefaults() and
                          when the config file does not yet contain the key of
                          this item.
    */
    void addItemString(const QString &key,QString *reference,
                       const QString &defaultValue="");
    /**
      Register an item of type QStringList.
      
      @param key Key used in config file.
      @param reference Pointer to the variable, which is set by readConfig()
                       and setDefaults() calls and read by writeConfig() calls.
      @param defaultValue Default value, which is used by setDefaults() and
                          when the config file does not yet contain the key of
                          this item.
    */
    void addItemStringList(const QString &key,QStringList *reference,
                           const QStringList &defaultValue=QStringList());

    /**
      Register an item of type QValueList<int>.
      
      @param key Key used in config file.
      @param reference Pointer to the variable, which is set by readConfig()
                       and setDefaults() calls and read by writeConfig() calls.
      @param defaultValue Default value, which is used by setDefaults() and
                          when the config file does not yet contain the key of
                          this item.
    */
    void addItemIntList(const QString &key,QValueList<int> *reference,
                        const QValueList<int> &defaultValue=QValueList<int>());

  protected:
    /**
      Implemented by subclasses that use special defaults.
    */
    virtual void usrSetDefaults() {};
    /**
      Implemented by subclasses that read special config values.
    */
    virtual void usrReadConfig() {};
    /**
      Implemented by subclasses that write special config values.
    */
    virtual void usrWriteConfig() {};

    /**
      Return the @ref KConfig object used for reading and writing the settings.
    */
    KConfig *config() const;

  private:
    static QString *mCurrentGroup;

    KConfig *mConfig;  // pointer to KConfig object

    QPtrList<KPrefsItem> mItems;
};

#endif
