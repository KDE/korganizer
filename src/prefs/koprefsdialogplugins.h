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

#ifndef KOPREFSDIALOGPLUGINS_H
#define KOPREFSDIALOGPLUGINS_H

#include <KCModule>
#include <QSet>
class QTreeWidget;
class QLabel;
class QPushButton;
class QRadioButton;
class QGroupBox;
class QTreeWidgetItem;

class KOPrefsDialogPlugins : public KCModule
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

#endif // KOPREFSDIALOGPLUGINS_H
