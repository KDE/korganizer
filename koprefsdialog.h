/*
    This file is part of KOrganizer.
    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef _KOPREFSDIALOG_H
#define _KOPREFSDIALOG_H

#include <qframe.h>
#include <qdict.h>
#include <qcolor.h>
#include <qlistview.h>

#include <kdialogbase.h>

#include <libkdepim/kprefsdialog.h>

class KColorButton;
class QSpinBox;
class QSlider;
class KURLRequester;
class QComboBox;
class QLineEdit;
class QStringList;

/** Dialog to change the korganizer configuration.
  */
class KOPrefsDialog : public KPrefsDialog
{
    Q_OBJECT
  public:
    /** Initialize dialog and pages */
    KOPrefsDialog(QWidget *parent=0,char *name=0,bool modal=false);
    ~KOPrefsDialog();

  public slots:
    void showPrinterTab();
    
    /** Update controls for categories */
    void updateCategories();

  protected slots:
    void setCategoryColor();
    void updateCategoryColor();

    void warningExperimental(bool on);
    void warningGroupScheduling();
    void warningProjectView();

    void toggleEmailSettings(bool);
    
    //additional emails
    void addItem();
    void removeItem();
    void updateItem();
    void updateInput();

  protected:
    void usrReadConfig();
    void usrWriteConfig();

    void setupMainTab();
    void setupTimeTab();
    void setupFontsTab();
    void setupColorsTab();
    void setupViewsTab();
    void setupDisplayTab();
    void setupPrinterTab();
    void setupGroupSchedulingTab();
    void setupGroupAutomationTab();

    void setCombo(QComboBox *combo,const QString & text, const QStringList *tags = 0);


  private:
    KPrefsWidBool *mEnableGroupScheduling;
    KPrefsWidBool *mEnableProjectView;

    QFrame *mPrinterTab;

    QLineEdit *nameEdit;
    QLineEdit *emailEdit;

    QComboBox *timeCombo;
    QComboBox *tzCombo;

    // widgets holding preferences data
    QLineEdit *mNameEdit;
    QLineEdit *mEmailEdit;
    QLabel *mNameLabel;
    QLabel *mEmailLabel;
    QLineEdit *mAdditionalEdit;
    QSpinBox  *mAutoSaveIntervalSpin;
    QListView *mAMails;
    QLineEdit *aEmailsEdit;

    QComboBox    *mTimeZoneCombo;
    QStringList  tzonenames;
    QSpinBox     *mStartTimeSpin;
    QSpinBox     *mDefaultDurationSpin;
    QComboBox    *mAlarmTimeCombo;

    QComboBox     *mCategoryCombo;
    KColorButton  *mCategoryButton;
    QDict<QColor> mCategoryDict;

    QSlider   *mHourSizeSlider;

    QSpinBox  *mNextXDaysSpin;
};

#endif
