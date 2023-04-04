/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2000, 2001, 2002, 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once
#include "kcmutils_version.h"
#include "kprefsdialog.h"
#include <QWidget>

class QComboBox;
class KColorButton;
namespace Akonadi
{
class CollectionComboBox;
}

class KOPrefsDialogColorsAndFonts : public Korganizer::KPrefsModule
{
    Q_OBJECT
public:
    explicit KOPrefsDialogColorsAndFonts(QObject *parent, const KPluginMetaData &data, const QVariantList &args = {});

protected:
    void usrWriteConfig() override;
    void usrReadConfig() override;

private:
    void useSystemColorToggle(bool useSystemColor);
    void updateCategories();
    void setCategoryColor();
    void updateCategoryColor();
    void updateResources();
    void setResourceColor();
    void updateResourceColor();
    QComboBox *mCategoryCombo = nullptr;
    KColorButton *mCategoryButton = nullptr;
    QHash<QString, QColor> mCategoryDict;

    Akonadi::CollectionComboBox *mResourceCombo = nullptr;
    KColorButton *mResourceButton = nullptr;
    QHash<QString, QColor> mResourceDict;
    QList<KColorButton *> mButtonsDisable; // button that are disabled when using system color
};
