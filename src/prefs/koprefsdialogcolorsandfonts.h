/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2000, 2001, 2002, 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#ifndef KOPREFSDIALOGCOLORSANDFONTS_H
#define KOPREFSDIALOGCOLORSANDFONTS_H
#include "kcm_korganizer_export.h"
#include <KCModule>
#include <QWidget>

class QLabel;
class QComboBox;
class KColorButton;
class QCheckBox;
namespace Akonadi {
class CollectionComboBox;
}
class FontPreviewButton : public QWidget
{
    Q_OBJECT
public:
    explicit FontPreviewButton(const QString &labelStr, QWidget *parent = nullptr);
    void setFont(const QFont &font);
    Q_REQUIRED_RESULT QFont font() const;
    void setPreviewText(const QString &str);
Q_SIGNALS:
    void changed();
private:
    void selectFont();
    QLabel *mPreview = nullptr;
};

class KCM_KORGANIZER_EXPORT KOPrefsDialogColorsAndFonts : public KCModule
{
    Q_OBJECT
public:
    explicit KOPrefsDialogColorsAndFonts(QWidget *parent);

    void save() override;
    void load() override;

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
    void slotConfigChanged();
    QComboBox *mCategoryCombo = nullptr;
    KColorButton *mCategoryButton = nullptr;
    QHash<QString, QColor> mCategoryDict;

    QCheckBox *mUseSystemColorCheckBox = nullptr;
    Akonadi::CollectionComboBox *mResourceCombo = nullptr;
    KColorButton *mResourceButton = nullptr;
    QHash<QString, QColor> mResourceDict;
    QList<KColorButton *> mButtonsDisable; // button that are disabled when using system color
    KColorButton *mAgendaBgColorButton = nullptr;
    KColorButton *mViewBgBusyColorButton = nullptr;
    KColorButton *mAgendaGridWorkHoursBackgroundColorButton = nullptr;
    KColorButton *mAgendaMarcusBainsLineLineColorButton = nullptr;
    KColorButton *mAgendaHolidaysBackgroundColorButton = nullptr;
    KColorButton *mTodoDueTodayColorButton = nullptr;
    KColorButton *mTodoOverdueColorButton = nullptr;
    KColorButton *mUnsetCategoryColorButton = nullptr;
    FontPreviewButton *mTimeBarFontButton = nullptr;
    FontPreviewButton *mMonthViewFont = nullptr;
    FontPreviewButton *mAgendaViewFont = nullptr;
    FontPreviewButton *mMarcusBainsFont = nullptr;
};

#endif // KOPREFSDIALOGCOLORSANDFONTS_H
