/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2000, 2001, 2002, 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#ifndef KOPREFSDIALOGPLUGINS_H
#define KOPREFSDIALOGPLUGINS_H

#include "kprefsdialog.h"
#include <QSet>
class QTreeWidget;
class QLabel;
class QPushButton;
class QRadioButton;
class QGroupBox;
class QTreeWidgetItem;

class KOPrefsDialogPlugins : public Korganizer::KPrefsModule
{
    Q_OBJECT
public:
    explicit KOPrefsDialogPlugins(QWidget *parent);
    ~KOPrefsDialogPlugins() override;

protected Q_SLOTS:
    void usrWriteConfig() override;
    void usrReadConfig() override;
    void configure();
    void selectionChanged();
    void positioningChanged();

private:
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

#endif // KOPREFSDIALOGPLUGINS_H
