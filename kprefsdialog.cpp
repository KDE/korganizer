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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

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
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>

#include <kcolorbutton.h>
#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kfontdialog.h>
#include <kmessagebox.h>
#include <kcolordialog.h>
#include <kiconloader.h>

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

  mButton = new KColorButton(parent);
  mLabel = new QLabel(mButton, text, parent);
}

KPrefsWidColor::~KPrefsWidColor()
{
//  kdDebug() << "KPrefsWidColor::~KPrefsWidColor()" << endl;
}

void KPrefsWidColor::readConfig()
{
  mButton->setColor(*mReference);
}

void KPrefsWidColor::writeConfig()
{
  *mReference = mButton->color();
}

QLabel *KPrefsWidColor::label()
{
  return mLabel;
}

KColorButton *KPrefsWidColor::button()
{
  return mButton;
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
