// $Id$

#include <qlayout.h>
#include <qlabel.h>
#include <qgroupbox.h>
#include <qbuttongroup.h>
#include <qlineedit.h>
#include <qfont.h>
#include <qslider.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qspinbox.h>
#include <qdatetime.h>
#include <qframe.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>

#include <kapp.h>
#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kfontdialog.h>
#include <kstddirs.h>
#include <kmessagebox.h>
#include <kcolordlg.h>
#include <kiconloader.h>
#include <kiconeffect.h>

#include "kprefs.h"

#include "kprefsdialog.h"
#include "kprefsdialog.moc"


KPrefsWidBool::KPrefsWidBool(const QString &text,bool *reference,
                             QWidget *parent)
{
  mReference = reference;

  mCheck = new QCheckBox(text,parent);
}

void KPrefsWidBool::readConfig()
{
  mCheck->setChecked(*mReference);
}

void KPrefsWidBool::writeConfig()
{
  *mReference = mCheck->isChecked();
}

QCheckBox *KPrefsWidBool::checkBox()
{
  return mCheck;
}


KPrefsWidColor::KPrefsWidColor(const QString &text,QColor *reference,
                               QWidget *parent)
{
  mReference = reference;

  mPreview = new QFrame(parent);
  mPreview->setFrameStyle(QFrame::Panel|QFrame::Plain);

  mButton = new QPushButton(text,parent);
  connect(mButton,SIGNAL(clicked()),SLOT(selectColor()));
}

KPrefsWidColor::~KPrefsWidColor()
{
//  kdDebug() << "KPrefsWidColor::~KPrefsWidColor()" << endl;
}

void KPrefsWidColor::readConfig()
{
  mPreview->setBackgroundColor(*mReference);
}

void KPrefsWidColor::writeConfig()
{
  *mReference = mPreview->backgroundColor();
}

QFrame *KPrefsWidColor::preview()
{
  return mPreview;
}

QPushButton *KPrefsWidColor::button()
{
  return mButton;
}

void KPrefsWidColor::selectColor()
{
  QColor myColor(mPreview->backgroundColor());
  int result = KColorDialog::getColor(myColor);
  if (result == KColorDialog::Accepted) {
    mPreview->setBackgroundColor(myColor);
  }
}


KPrefsWidFont::KPrefsWidFont(const QString &sampleText,const QString &buttonText,
                             QFont *reference,QWidget *parent)
{
  mReference = reference;

  mPreview = new QLabel(sampleText,parent);
  mPreview->setFrameStyle(QFrame::Panel|QFrame::Sunken);

  mButton = new QPushButton(buttonText,parent);
  connect(mButton,SIGNAL(clicked()),SLOT(selectFont()));
}

KPrefsWidFont::~KPrefsWidFont()
{
}

void KPrefsWidFont::readConfig()
{
  mPreview->setFont(*mReference);
}

void KPrefsWidFont::writeConfig()
{
  *mReference = mPreview->font();
}

QFrame *KPrefsWidFont::preview()
{
  return mPreview;
}

QPushButton *KPrefsWidFont::button()
{
  return mButton;
}

void KPrefsWidFont::selectFont()
{
  QFont myFont(mPreview->font());
  int result = KFontDialog::getFont(myFont);
  if (result == KFontDialog::Accepted) {
    mPreview->setFont(myFont);
  }
}


KPrefsWidTime::KPrefsWidTime(const QString &text,int *reference,
                             QWidget *parent)
{
  mReference = reference;

  mLabel = new QLabel(text,parent);
  mSpin = new QSpinBox(0,23,1,parent);
  mSpin->setSuffix(":00");
}

void KPrefsWidTime::readConfig()
{
  mSpin->setValue(*mReference);
}

void KPrefsWidTime::writeConfig()
{
  *mReference = mSpin->value();
}

QLabel *KPrefsWidTime::label()
{
  return mLabel;
}

QSpinBox *KPrefsWidTime::spinBox()
{
  return mSpin;
}


KPrefsWidRadios::KPrefsWidRadios(const QString &text,int *reference,
                QWidget *parent)
{
  mReference = reference;

  mBox = new QButtonGroup(1,Qt::Horizontal,text,parent);
}

KPrefsWidRadios::~KPrefsWidRadios()
{
}

void KPrefsWidRadios::addRadio(const QString &text)
{
  new QRadioButton(text,mBox);
}

QButtonGroup *KPrefsWidRadios::groupBox()
{
  return mBox;
}

void KPrefsWidRadios::readConfig()
{
  mBox->setButton(*mReference);
}

void KPrefsWidRadios::writeConfig()
{
  *mReference = mBox->id(mBox->selected());
}


KPrefsWidString::KPrefsWidString(const QString &text,QString *reference,
                                 QWidget *parent)
{
  mReference = reference;
  
  mLabel = new QLabel(text,parent);
  mEdit = new QLineEdit(parent);
}

KPrefsWidString::~KPrefsWidString()
{
}

void KPrefsWidString::readConfig()
{
  mEdit->setText(*mReference);
}

void KPrefsWidString::writeConfig()
{
  *mReference = mEdit->text();
}

QLabel *KPrefsWidString::label()
{
  return mLabel;
}

QLineEdit *KPrefsWidString::lineEdit()
{
  return mEdit;
}


KPrefsDialog::KPrefsDialog(KPrefs *prefs,QWidget *parent,char *name,bool modal) :
  KDialogBase(IconList,i18n("Preferences"),Ok|Apply|Cancel|Default,Ok,parent,
              name,modal,true)
{
  mPrefs = prefs;

// This seems to cause a crash on exit. Investigate later.
//  mPrefsWids.setAutoDelete(true);

  connect(this,SIGNAL(defaultClicked()),SLOT(setDefaults()));
  connect(this,SIGNAL(cancelClicked()),SLOT(reject()));
}

KPrefsDialog::~KPrefsDialog()
{
}

void KPrefsDialog::addWid(KPrefsWid *wid)
{
  mPrefsWids.append(wid);
}

KPrefsWidBool *KPrefsDialog::addWidBool(const QString &text,bool *reference,QWidget *parent)
{
  KPrefsWidBool *w = new KPrefsWidBool(text,reference,parent);
  addWid(w);
  return w;
}

KPrefsWidTime *KPrefsDialog::addWidTime(const QString &text,int *reference,QWidget *parent)
{
  KPrefsWidTime *w = new KPrefsWidTime(text,reference,parent);
  addWid(w);
  return w;
}

KPrefsWidColor *KPrefsDialog::addWidColor(const QString &text,QColor *reference,QWidget *parent)
{
  KPrefsWidColor *w = new KPrefsWidColor(text,reference,parent);
  addWid(w);
  return w;
}

KPrefsWidRadios *KPrefsDialog::addWidRadios(const QString &text,int *reference,QWidget *parent)
{
  KPrefsWidRadios *w = new KPrefsWidRadios(text,reference,parent);
  addWid(w);
  return w;
}

KPrefsWidString *KPrefsDialog::addWidString(const QString &text,QString *reference,QWidget *parent)
{
  KPrefsWidString *w = new KPrefsWidString(text,reference,parent);
  addWid(w);
  return w;
}

KPrefsWidFont *KPrefsDialog::addWidFont(const QString &sampleText,const QString &buttonText,
                                        QFont *reference,QWidget *parent)
{
  KPrefsWidFont *w = new KPrefsWidFont(sampleText,buttonText,reference,parent);
  addWid(w);
  return w;
}

void KPrefsDialog::setDefaults()
{
  mPrefs->setDefaults();
  
  readConfig();
}

void KPrefsDialog::readConfig()
{
//  kdDebug() << "KPrefsDialog::readConfig()" << endl;

  KPrefsWid *wid;
  for(wid = mPrefsWids.first();wid;wid=mPrefsWids.next()) {
    wid->readConfig();
  }

  usrReadConfig();
}

void KPrefsDialog::writeConfig()
{
//  kdDebug() << "KPrefsDialog::writeConfig()" << endl;

  KPrefsWid *wid;
  for(wid = mPrefsWids.first();wid;wid=mPrefsWids.next()) {
    wid->writeConfig();
  }

  usrWriteConfig();

//  kdDebug() << "KPrefsDialog::writeConfig() now writing..." << endl;
  
  mPrefs->writeConfig();

//  kdDebug() << "KPrefsDialog::writeConfig() done" << endl;
}


void KPrefsDialog::slotApply()
{
  writeConfig();
  emit configChanged();
}

void KPrefsDialog::slotOk()
{
  slotApply();
  accept();
}

void KPrefsDialog::slotDefault()
{
  if (KMessageBox::warningContinueCancel(this,
      i18n("You are about to set all preferences to default values. All "
      "custom modifications will be lost."),i18n("Setting Default Preferences"),
      i18n("Continue"))
    == KMessageBox::Continue) setDefaults(); 
}
