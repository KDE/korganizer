#ifndef _KPREFS_H
#define _KPREFS_H
// $Id$
// (C) 2000 by Cornelius Schumacher

#include <qlist.h>
#include <qdict.h>
#include <qcolor.h>

class KConfig;

class KPrefsItem {
  public:
    KPrefsItem(const QString &name);
    virtual ~KPrefsItem() {}
    
    virtual void setDefault() = 0;
    virtual void readConfig(KConfig *) = 0;
    virtual void writeConfig(KConfig *) = 0;

    static void setCurrentGroup(const QString &group);
    
  protected:
    static QString *mCurrentGroup;
    
    QString mGroup;
    QString mName;
};

class KPrefsItemBool : public KPrefsItem {
  public:
    KPrefsItemBool(const QString &name,bool *,bool defaultValue=true);
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
    KPrefsItemInt(const QString &name,int *,int defaultValue=0);
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
    KPrefsItemColor(const QString &name,QColor *,
                    QColor defaultValue=QColor(128,128,128));
    virtual ~KPrefsItemColor() {}
    
    void setDefault();
    void readConfig(KConfig *);
    void writeConfig(KConfig *);    

  private:
    QColor *mReference;
    QColor mDefault;
};


class KPrefsItemString : public KPrefsItem {
  public:
    KPrefsItemString(const QString &name,QString *,
                     const QString &defaultValue="");
    virtual ~KPrefsItemString() {}
    
    void setDefault();
    void readConfig(KConfig *);
    void writeConfig(KConfig *);    

  private:
    QString *mReference;
    QString mDefault;
};


class KPrefs {
  public:
    KPrefs(const QString &configname=QString::null);
    virtual ~KPrefs();
  
    /** Set preferences to default values */
    void setDefaults();
  
    /** Read preferences from config file */
    void readConfig();

    /** Write preferences to config file */
    void writeConfig();

  protected:
    /** Implemented by subclasses that use special defaults. */
    virtual void usrSetDefaults() {};
    /** Implemented by subclasses that read special config values */
    virtual void usrReadConfig() {};
    /** Implemented by subclasses that write special config values */
    virtual void usrWriteConfig() {};

//    void addPrefsItem(const QString &group,KPrefsItem *);
    void addPrefsItem(KPrefsItem *);

    KConfig *mConfig;  // pointer to KConfig object

    /** Constructor disabled for public. Use instance() to create a KPrefs
    object. */
    KPrefs();

  private:
//    QDict<QList<KPrefsItem> > mGroups;
    QList<KPrefsItem> mItems;
};

#endif
