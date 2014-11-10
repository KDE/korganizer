/*
  This file is part of KOrganizer.

  Copyright (c) 2001,2004 Cornelius Schumacher <schumacher@kde.org>
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

#ifndef KORG_KODIALOGMANAGER_H
#define KORG_KODIALOGMANAGER_H

#include "koeventview.h"

class CalendarView;
class FilterEditDialog;
class SearchDialog;
class QDialog;

namespace CalendarSupport
{
class ArchiveDialog;
}

namespace IncidenceEditorNG
{
class IncidenceDialog;
}

namespace KCalCore
{
class CalFilter;
}

class KCMultiDialog;

/**
  This class manages the dialogs used by the calendar view. It owns the objects
  and handles creation and selection.
*/
class KODialogManager : public QObject
{
    Q_OBJECT
public:
    explicit KODialogManager(CalendarView *);
    virtual ~KODialogManager();

    IncidenceEditorNG::IncidenceDialog *createDialog(const Akonadi::Item &item);

    // TODO_NG: see if editors-NG have the needed slots.
    void connectEditor(IncidenceEditorNG::IncidenceDialog *editor);

    void updateSearchDialog();

    void connectTypeAhead(IncidenceEditorNG::IncidenceDialog *editor, KOEventView *view);

public slots:
    void showOptionsDialog();
    void showCategoryEditDialog();
    void showSearchDialog();
    void showArchiveDialog();
    void showFilterEditDialog(QList<KCalCore::CalFilter *> *filters);

private:
    void createCategoryEditor();
    class DialogManagerVisitor;
    class EditorDialogVisitor;

    CalendarView *mMainView;
    KCMultiDialog *mOptionsDialog;
    QPointer<QDialog> mCategoryEditDialog;
    SearchDialog *mSearchDialog;
    CalendarSupport::ArchiveDialog *mArchiveDialog;
    FilterEditDialog *mFilterEditDialog;
};

#endif
