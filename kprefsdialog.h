// $Id$
// (C) 2000 by Cornelius Schumacher

#ifndef _KPREFSDIALOG_H
#define _KPREFSDIALOG_H

#include <kdialogbase.h>

class KPrefs;
class KPrefsDialog;

class QCheckBox;
class QLabel;
class QSpinBox;

class KPrefsWid
{
  public:
    KPrefsWid(KPrefsDialog *prefsDialog);
    
    virtual void readConfig() = 0;
    virtual void writeConfig() = 0;
};

class KPrefsWidBool : public KPrefsWid
{
  public:
    KPrefsWidBool(const QString &text,bool *reference,KPrefsDialog *prefsDialog,
                  QWidget *parent);
    
    QCheckBox *checkBox();
    
    void readConfig();
    void writeConfig();
    
  private:
    bool *mReference;
    
    QCheckBox *mCheck;
};

class KPrefsWidTime : public KPrefsWid
{
  public:
    KPrefsWidTime(const QString &text,int *reference,KPrefsDialog *prefsDialog,
                  QWidget *parent);
    
    QLabel *label();
    QSpinBox *spinBox();
    
    void readConfig();
    void writeConfig();
    
  private:
    int *mReference;
    
    QLabel *mLabel;
    QSpinBox *mSpin;
};


class KPrefsWidColor : public QObject, public KPrefsWid
{
    Q_OBJECT
  public:
    KPrefsWidColor(const QString &text,QColor *reference,
                   KPrefsDialog *prefsDialog,QWidget *parent);
    ~KPrefsWidColor();
    
    QFrame *preview();
    QPushButton *button();
    
    void readConfig();
    void writeConfig();

  protected slots:
    void selectColor();
    
  private:
    QColor *mReference;
    
    QFrame *mPreview;
    QPushButton *mButton;
};

class KPrefsWidRadios : public KPrefsWid
{
  public:
    KPrefsWidRadios(const QString &text,int *reference,
                    KPrefsDialog *prefsDialog,QWidget *parent);
    virtual ~KPrefsWidRadios();

    void addRadio(const QString &text);
    
    QButtonGroup *groupBox();
    
    void readConfig();
    void writeConfig();
    
  private:
    int *mReference;

    QButtonGroup *mBox;
};


/** Preferences dialog base class.
  */
class KPrefsDialog : public KDialogBase
{
    Q_OBJECT
  public:
    /** Initialize dialog and pages */
    KPrefsDialog(KPrefs *prefs,QWidget *parent=0,char *name=0,bool modal=false);
    virtual ~KPrefsDialog();

    void addPrefsWid(KPrefsWid *);

  public slots:
    /** Set all widgets to default values */
    void setDefaults();
  
    /** Read preferences from config file */
    void readConfig();

    /** Write preferences to config file */
    void writeConfig();

  signals:
    void configChanged();

  protected slots:
    /** Apply changes to preferences */
    void slotApply();  

    /** Accept changes to preferences and close dialog */
    void slotOk();
  
    /** Set preferences to default values */
    void slotDefault();
  
  protected:
    virtual void usrReadConfig() {}
    virtual void usrWriteConfig() {}

  private:
    KPrefs *mPrefs;

    QList<KPrefsWid> mPrefsWids;
};

#endif
