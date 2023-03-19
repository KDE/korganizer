/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2004 Tobias Koenig <tokoe@kde.org>
  SPDX-FileCopyrightText: 2004 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once
#include "kcmutils_version.h"
#include <KCModule>
#if KCMUTILS_VERSION >= QT_VERSION_CHECK(5, 240, 0)
#include <KPluginMetaData>
#endif
class QLabel;
class QPushButton;
class QTreeWidget;
class QTreeWidgetItem;

class KCMDesignerFields : public KCModule
{
    Q_OBJECT
public:
#if KCMUTILS_VERSION < QT_VERSION_CHECK(5, 240, 0)
    explicit KCMDesignerFields(QWidget *parent = nullptr, const QVariantList &args = QVariantList());
#else
    explicit KCMDesignerFields(QObject *parent, const KPluginMetaData &data, const QVariantList &args = QVariantList());
#endif

    void load() override;
    void save() override;
    void defaults() override;

protected:
    void loadUiFiles();
    void loadActivePages(const QStringList &);
    Q_REQUIRED_RESULT QStringList saveActivePages();

    virtual QString localUiDir() = 0;
    virtual QString uiPath() = 0;
    virtual void writeActivePages(const QStringList &) = 0;
    virtual QStringList readActivePages() = 0;
    virtual QString applicationName() = 0;

private Q_SLOTS:
    void updatePreview();
    void itemClicked(QTreeWidgetItem *);
    void startDesigner();
    void rebuildList();
    void deleteFile();
    void importFile();
    void delayedInit();
    void showWhatsThis(const QString &href);

private:
    void initGUI();

    QTreeWidget *mPageView = nullptr;
    QLabel *mPagePreview = nullptr;
    QLabel *mPageDetails = nullptr;
    QPushButton *mDeleteButton = nullptr;
    QPushButton *mImportButton = nullptr;
    QPushButton *mDesignerButton = nullptr;
};
