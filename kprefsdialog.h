// $Id$
// (C) 2001 by Cornelius Schumacher

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
    virtual void readConfig() = 0;
    virtual void writeConfig() = 0;
};

class KPrefsWidBool : public KPrefsWid
{
  public:
    KPrefsWidBool(const QString &text,bool *reference,QWidget *parent);
    
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
    KPrefsWidTime(const QString &text,int *reference,QWidget *parent);
    
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
    KPrefsWidColor(const QString &text,QColor *reference,QWidget *parent);
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

class KPrefsWidFont : public QObject, public KPrefsWid
{
    Q_OBJECT
  public:
    KPrefsWidFont(const QString &sampleText,const QString &buttonText,
                  QFont *reference,QWidget *parent);
    ~KPrefsWidFont();
    
    QFrame *preview();
    QPushButton *button();
    
    void readConfig();
    void writeConfig();

  protected slots:
    void selectFont();
    
  private:
    QFont *mReference;
    
    QLabel *mPreview;
    QPushButton *mButton;
};

class KPrefsWidRadios : public KPrefsWid
{
  public:
    KPrefsWidRadios(const QString &text,int *reference,QWidget *parent);
    virtual ~KPrefsWidRadios();

    void addRadio(const QString &text);
    
    QButtonGroup *groupBox();
    
    void readConfig();
    void writeConfig();
    
  private:
    int *mReference;

    QButtonGroup *mBox;
};


class KPrefsWidString : public KPrefsWid
{
  public:
    KPrefsWidString(const QString &text,QString *reference,QWidget *parent);
    virtual ~KPrefsWidString();

    QLabel *label();
    QLineEdit *lineEdit();
    
    void readConfig();
    void writeConfig();
    
  private:
    QString *mReference;

    QLabel *mLabel;
    QLineEdit *mEdit;
};


/**
  Preferences dialog base class.
*/
class KPrefsDialog : public KDialogBase
{
    Q_OBJECT
  public:
    /** Initialize dialog and pages */
    KPrefsDialog(KPrefs *prefs,QWidget *parent=0,char *name=0,bool modal=false);
    virtual ~KPrefsDialog();

    void addWid(KPrefsWid *);
    KPrefsWidBool *addWidBool(const QString &text,bool *reference,QWidget *parent);
    KPrefsWidTime *addWidTime(const QString &text,int *reference,QWidget *parent);
    KPrefsWidColor *addWidColor(const QString &text,QColor *reference,QWidget *parent);
    KPrefsWidRadios *addWidRadios(const QString &text,int *reference,QWidget *parent);
    KPrefsWidString *addWidString(const QString &text,QString *reference,QWidget *parent);
    KPrefsWidFont *addWidFont(const QString &sampleText,const QString &buttonText,
                              QFont *reference,QWidget *parent);

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
