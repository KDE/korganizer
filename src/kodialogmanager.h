/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2001, 2004 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#ifndef KORG_KODIALOGMANAGER_H
#define KORG_KODIALOGMANAGER_H

#include "koeventview.h"

class CalendarView;
class FilterEditDialog;
class SearchDialog;

namespace CalendarSupport
{
class ArchiveDialog;
}

namespace IncidenceEditorNG
{
class IncidenceDialog;
}

namespace KCalendarCore
{
class CalFilter;
}
namespace Akonadi
{
class TagManagementDialog;
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
    ~KODialogManager() override;

    IncidenceEditorNG::IncidenceDialog *createDialog(const Akonadi::Item &item);

    // TODO_NG: see if editors-NG have the needed slots.
    void connectEditor(IncidenceEditorNG::IncidenceDialog *editor);

    void updateSearchDialog();

    void connectTypeAhead(IncidenceEditorNG::IncidenceDialog *editor, KOEventView *view);

public Q_SLOTS:
    void showOptionsDialog();
    void showCategoryEditDialog();
    void showSearchDialog();
    void showArchiveDialog();
    void showFilterEditDialog(QList<KCalendarCore::CalFilter *> *filters);

private Q_SLOTS:
    void slotHelp();

private:
    void createCategoryEditor();
    class DialogManagerVisitor;
    class EditorDialogVisitor;

    CalendarView *const mMainView;
    KCMultiDialog *mOptionsDialog = nullptr;
    QPointer<Akonadi::TagManagementDialog> mCategoryEditDialog;
    SearchDialog *mSearchDialog = nullptr;
    CalendarSupport::ArchiveDialog *mArchiveDialog = nullptr;
    FilterEditDialog *mFilterEditDialog = nullptr;
};

#endif
