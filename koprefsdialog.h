/*
  This file is part of KOrganizer.

  Copyright (c) 2000,2001,2002,2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/
#ifndef KORG_KOPREFSDIALOG_H
#define KORG_KOPREFSDIALOG_H

#include "korganizer_export.h"
#include "kcmdesignerfields.h"
#include "ui_accountscalendarwidget.h"

#include <libkdepim/prefs/kprefsdialog.h>

class KPushButton;

namespace Ui {
  class KOGroupwarePrefsPage;
}

namespace Akonadi {
  class CollectionComboBox;
}

class QRadioButton;

class KCM_KORGANIZER_EXPORT KOPrefsDialogMain : public KPIM::KPrefsModule
{
  Q_OBJECT
  public:
    KOPrefsDialogMain( const KComponentData &inst, QWidget *parent );

  protected slots:
    void toggleEmailSettings( bool on );
    void slotAccountSelected();
    void slotAddAccount();
    void slotModifySelectedAccount();
    void slotRemoveSelectedAccount();

  private:
    QWidget *mUserEmailSettings;
    Ui_AccountsCalendarWidget mAccountsCalendar;
};

class KCM_KORGANIZER_EXPORT KOPrefsDialogColorsAndFonts : public KPIM::KPrefsModule
{
  Q_OBJECT
  public:
    KOPrefsDialogColorsAndFonts( const KComponentData &inst, QWidget *parent );

  protected:
    void usrWriteConfig();
    void usrReadConfig();

  protected slots:
    void updateCategories();
    void setCategoryColor();
    void updateCategoryColor();

    void updateResources();
    void setResourceColor();
    void updateResourceColor();

  private:
    KComboBox     *mCategoryCombo;
    KColorButton  *mCategoryButton;
    QHash<QString,QColor> mCategoryDict;

    Akonadi::CollectionComboBox *mResourceCombo;
    KColorButton  *mResourceButton;
    QHash<QString, QColor> mResourceDict;
};

class KCM_KORGANIZER_EXPORT KOPrefsDialogGroupScheduling : public KPIM::KPrefsModule
{
  Q_OBJECT
  public:
    KOPrefsDialogGroupScheduling( const KComponentData &inst, QWidget *parent );

  protected:
    void usrReadConfig();
    void usrWriteConfig();
};

class KOGroupwarePrefsPage;

class KCM_KORGANIZER_EXPORT KOPrefsDialogGroupwareScheduling : public KPIM::KPrefsModule
{
  Q_OBJECT
  public:
    KOPrefsDialogGroupwareScheduling( const KComponentData &inst, QWidget *parent );
    ~KOPrefsDialogGroupwareScheduling();

  protected:
    void usrReadConfig();
    void usrWriteConfig();

  private:
    Ui::KOGroupwarePrefsPage *mGroupwarePage;
};

class KCM_KORGANIZER_EXPORT KOPrefsDialogPlugins : public KPIM::KPrefsModule
{
  Q_OBJECT
  public:
    KOPrefsDialogPlugins( const KComponentData &inst, QWidget *parent );

  protected slots:
    void usrReadConfig();
    void usrWriteConfig();
    void configure();
    void selectionChanged();
    void positioningChanged();

  private:
    void buildList();
    QTreeWidget *mTreeWidget;
    QLabel *mDescription;
    KPushButton *mConfigureButton;
    QGroupBox *mPositioningGroupBox;

    //Decorations are not implemented in month view yet
    //QCheckBox *mPositionMonthTop;

    QRadioButton *mPositionAgendaTop;
    QRadioButton *mPositionAgendaBottom;
    QSet<QString> mDecorationsAtMonthViewTop;
    QSet<QString> mDecorationsAtAgendaViewTop;
    QSet<QString> mDecorationsAtAgendaViewBottom;
};

class KCM_KORGANIZER_EXPORT KOPrefsDesignerFields : public KCMDesignerFields
{
  public:
    explicit KOPrefsDesignerFields( const KComponentData &inst, QWidget *parent = 0 );

  protected:
    QString localUiDir();
    QString uiPath();
    void writeActivePages( const QStringList & );
    QStringList readActivePages();
    QString applicationName();
};

/*class KCM_KORGANIZER_EXPORT KOPrefsDialogThemes : public KPrefsModule
{
  Q_OBJECT
  public:
    KOPrefsDialogThemes( const KComponentData &inst, QWidget *parent );

  protected slots:
    void usrReadConfig();
    void usrWriteConfig();
    void importTheme();
    void exportTheme();
};*/

#endif
