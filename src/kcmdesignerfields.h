/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2004 Tobias Koenig <tokoe@kde.org>
  SPDX-FileCopyrightText: 2004 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once
#include <KCModule>
#include <KPluginMetaData>
class QLabel;
class QPushButton;
class QTreeWidget;
class QTreeWidgetItem;

class KCMDesignerFields : public KCModule
{
    Q_OBJECT
public:
    explicit KCMDesignerFields(QObject *parent, const KPluginMetaData &data = {});

    void load() override;
    void save() override;
    void defaults() override;

protected:
    void loadUiFiles();
    void loadActivePages(const QStringList &);
    [[nodiscard]] QStringList saveActivePages();

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
