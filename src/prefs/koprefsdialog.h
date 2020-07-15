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
#include <Libkdepim/KPrefsDialog>
#include <KColorButton>

class QPushButton;
class QComboBox;

namespace Ui {
class KOGroupwarePrefsPage;
}

namespace Akonadi {
class CollectionComboBox;
}

class QRadioButton;

class KCM_KORGANIZER_EXPORT KOPrefsDialogMain : public KCModule
{
    Q_OBJECT
public:
    explicit KOPrefsDialogMain(QWidget *parent);

    void save() override;
    void load() override;

protected Q_SLOTS:
    void toggleEmailSettings(bool on);

private:
    QWidget *mUserEmailSettings = nullptr;
    QCheckBox *mEmailControlCenterCheckBox = nullptr;
    QLineEdit *mUserName = nullptr;
    QLineEdit *mUserEmail = nullptr;
    QCheckBox *mConfirmCheckBox = nullptr;
    QCheckBox *mDestinationCheckBox = nullptr;
    QCheckBox *mShowReminderDaemonCheckBox = nullptr;
};

class KCM_KORGANIZER_EXPORT KOPrefsDialogColorsAndFonts : public KPIM::KPrefsModule
{
    Q_OBJECT
public:
    explicit KOPrefsDialogColorsAndFonts(QWidget *parent);

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

    QCheckBox *mUseSystemColorCheckBox = nullptr;
    Akonadi::CollectionComboBox *mResourceCombo = nullptr;
    KColorButton *mResourceButton = nullptr;
    QHash<QString, QColor> mResourceDict;
    QList<KColorButton *> mButtonsDisable; // button that are disabled when using system color
};

class KCM_KORGANIZER_EXPORT KOPrefsDialogGroupScheduling : public KCModule
{
    Q_OBJECT
public:
    explicit KOPrefsDialogGroupScheduling(QWidget *parent);

    void save() override;
    void load() override;

private:
    void slotConfigChanged();
    QCheckBox *mUseGroupwareCommunicationCheckBox = nullptr;
    QCheckBox *mBccBox = nullptr;
};

class KOGroupwarePrefsPage;

class KCM_KORGANIZER_EXPORT KOPrefsDialogGroupwareScheduling : public KCModule
{
    Q_OBJECT
public:
    explicit KOPrefsDialogGroupwareScheduling(QWidget *parent);
    ~KOPrefsDialogGroupwareScheduling() override;

    void save() override;
    void load() override;

private:
    void slotConfigChanged();
    Ui::KOGroupwarePrefsPage *mGroupwarePage = nullptr;
};

class KCM_KORGANIZER_EXPORT KOPrefsDialogPlugins : public KCModule
{
    Q_OBJECT
public:
    explicit KOPrefsDialogPlugins(QWidget *parent);
    ~KOPrefsDialogPlugins() override;

    void save() override;
    void load() override;

protected Q_SLOTS:
    void configure();
    void selectionChanged();
    void positioningChanged();

private:
    void slotConfigChanged();
    void buildList();
    QTreeWidget *mTreeWidget = nullptr;
    QLabel *mDescription = nullptr;
    QPushButton *mConfigureButton = nullptr;
    QGroupBox *mPositioningGroupBox = nullptr;

    QRadioButton *mPositionAgendaTop = nullptr;
    QRadioButton *mPositionAgendaBottom = nullptr;
    QTreeWidgetItem *mDecorations = nullptr;
    QTreeWidgetItem *mOthers = nullptr;
    QSet<QString> mDecorationsAtMonthViewTop;
    QSet<QString> mDecorationsAtAgendaViewTop;
    QSet<QString> mDecorationsAtAgendaViewBottom;
};

class KCM_KORGANIZER_EXPORT KOPrefsDesignerFields : public KCMDesignerFields
{
public:
    explicit KOPrefsDesignerFields(QWidget *parent = nullptr);

protected:
    Q_REQUIRED_RESULT QString localUiDir() override;
    Q_REQUIRED_RESULT QString uiPath() override;
    void writeActivePages(const QStringList &) override;
    Q_REQUIRED_RESULT QStringList readActivePages() override;
    Q_REQUIRED_RESULT QString applicationName() override;
};

#ifdef WITH_KUSERFEEDBACK
namespace KUserFeedback {
class FeedbackConfigWidget;
}
class KCM_KORGANIZER_EXPORT KOPrefsUserFeedBack : public KCModule
{
public:
    explicit KOPrefsUserFeedBack(QWidget *parent = nullptr, const QVariantList &args = QVariantList());

protected:
    void load() override;
    void save() override;
private:
    KUserFeedback::FeedbackConfigWidget *mUserFeedbackWidget = nullptr;
};
#endif

#endif
