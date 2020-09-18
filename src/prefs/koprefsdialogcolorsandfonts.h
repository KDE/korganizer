/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2000, 2001, 2002, 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#ifndef KOPREFSDIALOGCOLORSANDFONTS_H
#define KOPREFSDIALOGCOLORSANDFONTS_H
#include "kcm_korganizer_export.h"
#include "kprefsdialog.h"
#include <QWidget>

class QLabel;
class QComboBox;
class KColorButton;
class QCheckBox;
namespace Akonadi {
class CollectionComboBox;
}

class KOPrefsDialogColorsAndFonts : public Korganizer::KPrefsModule
{
    Q_OBJECT
public:
    KOPrefsDialogColorsAndFonts(QWidget *parent);

protected:
    void usrWriteConfig() override;
    void usrReadConfig() override;

private Q_SLOTS:
    void useSystemColorToggle(bool useSystemColor);

protected Q_SLOTS:
    void updateCategories();
    void setCategoryColor();
    void updateCategoryColor();

    void updateResources();
    void setResourceColor();
    void updateResourceColor();

private:
    QComboBox *mCategoryCombo = nullptr;
    KColorButton *mCategoryButton = nullptr;
    QHash<QString, QColor> mCategoryDict;

    Akonadi::CollectionComboBox *mResourceCombo = nullptr;
    KColorButton *mResourceButton = nullptr;
    QHash<QString, QColor> mResourceDict;
    QList<KColorButton *> mButtonsDisable; // button that are disabled when using system color
};

#endif // KOPREFSDIALOGCOLORSANDFONTS_H
