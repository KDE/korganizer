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

#include "kcm_korganizer_export.h"
#include "kcmdesignerfields.h"
#include <QSet>
#include <libkdepim/prefs/kprefsdialog.h>

class QPushButton;

namespace Ui
{
class KOGroupwarePrefsPage;
}

namespace Akonadi
{
class CollectionComboBox;
}

class QRadioButton;

class KCM_KORGANIZER_EXPORT KOPrefsDialogMain : public KPIM::KPrefsModule
{
    Q_OBJECT
public:
    KOPrefsDialogMain(QWidget *parent);

protected:
    void usrWriteConfig() Q_DECL_OVERRIDE;

protected Q_SLOTS:
    void toggleEmailSettings(bool on);

private:
    QWidget *mUserEmailSettings;
};

class KCM_KORGANIZER_EXPORT KOPrefsDialogColorsAndFonts : public KPIM::KPrefsModule
{
    Q_OBJECT
public:
    KOPrefsDialogColorsAndFonts(QWidget *parent);

protected:
    void usrSave();
    void usrRead();

protected Q_SLOTS:
    void updateCategories();
    void setCategoryColor();
    void updateCategoryColor();

    void updateResources();
    void setResourceColor();
    void updateResourceColor();

private:
    KComboBox     *mCategoryCombo;
    KColorButton  *mCategoryButton;
    QHash<QString, QColor> mCategoryDict;

    Akonadi::CollectionComboBox *mResourceCombo;
    KColorButton  *mResourceButton;
    QHash<QString, QColor> mResourceDict;
};

class KCM_KORGANIZER_EXPORT KOPrefsDialogGroupScheduling : public KPIM::KPrefsModule
{
    Q_OBJECT
public:
    KOPrefsDialogGroupScheduling(QWidget *parent);

protected:
    void usrRead();
    void usrSave();
};

class KOGroupwarePrefsPage;

class KCM_KORGANIZER_EXPORT KOPrefsDialogGroupwareScheduling : public KPIM::KPrefsModule
{
    Q_OBJECT
public:
    KOPrefsDialogGroupwareScheduling(QWidget *parent);
    ~KOPrefsDialogGroupwareScheduling();

protected:
    void usrRead();
    void usrSave();

private:
    Ui::KOGroupwarePrefsPage *mGroupwarePage;
};

class KCM_KORGANIZER_EXPORT KOPrefsDialogPlugins : public KPIM::KPrefsModule
{
    Q_OBJECT
public:
    KOPrefsDialogPlugins(QWidget *parent);

protected Q_SLOTS:
    void usrRead();
    void usrSave();
    void configure();
    void selectionChanged();
    void positioningChanged();

private:
    void buildList();
    QTreeWidget *mTreeWidget;
    QLabel *mDescription;
    QPushButton *mConfigureButton;
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
    explicit KOPrefsDesignerFields(QWidget *parent = Q_NULLPTR);

protected:
    QString localUiDir() Q_DECL_OVERRIDE;
    QString uiPath() Q_DECL_OVERRIDE;
    void writeActivePages(const QStringList &) Q_DECL_OVERRIDE;
    QStringList readActivePages() Q_DECL_OVERRIDE;
    QString applicationName() Q_DECL_OVERRIDE;
};

/*class KCM_KORGANIZER_EXPORT KOPrefsDialogThemes : public KPrefsModule
{
  Q_OBJECT
  public:
    KOPrefsDialogThemes( const KComponentData &inst, QWidget *parent );

  protected Q_SLOTS:
    void usrRead();
    void usrSave();
    void importTheme();
    void exportTheme();
};*/

#endif
