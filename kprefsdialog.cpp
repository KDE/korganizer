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


KPrefsWid::KPrefsWid(KPrefsDialog *prefsDialog)
{
  prefsDialog->addPrefsWid(this);
}


KPrefsWidBool::KPrefsWidBool(const QString &text,bool *reference,
                             KPrefsDialog *prefsDialog,QWidget *parent) :
  KPrefsWid(prefsDialog)
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
                               KPrefsDialog *prefsDialog,QWidget *parent) :
  KPrefsWid(prefsDialog)
{
  mReference = reference;

  mPreview = new QFrame(parent);
  mPreview->setFrameStyle(QFrame::Panel|QFrame::Plain);

  mButton = new QPushButton(text,parent);
  connect(mButton,SIGNAL(clicked()),SLOT(selectColor()));
}

KPrefsWidColor::~KPrefsWidColor()
{
  kdDebug() << "KPrefsWidColor::~KPrefsWidColor()" << endl;
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
  QColor myColor;
  int result = KColorDialog::getColor(myColor);
  if (result == KColorDialog::Accepted) {
    mPreview->setBackgroundColor(myColor);
  }
}


KPrefsWidTime::KPrefsWidTime(const QString &text,int *reference,
                             KPrefsDialog *prefsDialog,QWidget *parent) :
  KPrefsWid(prefsDialog)
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


KPrefsDialog::KPrefsDialog(KPrefs *prefs,QWidget *parent,char *name,bool modal) :
  KDialogBase(IconList,i18n("Preferences"),Ok|Apply|Cancel|Default,Ok,parent,
              name,modal,true)
{
  mPrefs = prefs;

// This seems to cause a crash on exit. Investigate later.
//  mPrefsWids.setAutoDelete(true);

  QObject::connect(this,SIGNAL(defaultClicked()),SLOT(setDefaults()));
  QObject::connect(this,SIGNAL(cancelClicked()),SLOT(reject()));
}

KPrefsDialog::~KPrefsDialog()
{
}

void KPrefsDialog::addPrefsWid(KPrefsWid *wid)
{
  mPrefsWids.append(wid);
}

void KPrefsDialog::setDefaults()
{
  mPrefs->setDefaults();
  
  readConfig();
}

void KPrefsDialog::readConfig()
{
  KPrefsWid *wid;
  for(wid = mPrefsWids.first();wid;wid=mPrefsWids.next()) {
    wid->readConfig();
  }

  usrReadConfig();
}

void KPrefsDialog::writeConfig()
{
  kdDebug() << "KPrefsDialog::writeConfig()" << endl;

  KPrefsWid *wid;
  for(wid = mPrefsWids.first();wid;wid=mPrefsWids.next()) {
    wid->writeConfig();
  }

  usrWriteConfig();

  kdDebug() << "KPrefsDialog::writeConfig() now writing..." << endl;
  
  mPrefs->writeConfig();

  kdDebug() << "KPrefsDialog::writeConfig() done" << endl;
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
