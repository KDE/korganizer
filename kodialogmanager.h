/*
  This file is part of KOrganizer.
  Copyright (c) 2001
  Cornelius Schumacher <schumacher@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/
#ifndef KODIALOGMANAGER_H
#define KODIALOGMANAGER_H

#include <qobject.h>
#include <qptrlist.h>

#include <libkcal/calfilter.h>

class CalendarView;
class OutgoingDialog;
class IncomingDialog;
class KOPrefsDialog;
class CategoryEditDialog;
class KOEventEditor;
class KOTodoEditor;
class SearchDialog;
class ArchiveDialog;
class PluginDialog;
class FilterEditDialog;

using namespace KCal;

/**
  This class manages the dialogs used by the calendar view. It owns the objects
  and handles creation and selection.
*/
class KODialogManager : public QObject
{
    Q_OBJECT
  public:
    KODialogManager( CalendarView * );
    virtual ~KODialogManager();

    /** Get an editor dialog for an Event. */
    KOEventEditor *getEventEditor();
    
    /** Get an editor dialog for a Todo. */
    KOTodoEditor *getTodoEditor();

    OutgoingDialog *outgoingDialog();
    
    IncomingDialog *incomingDialog();

    void updateSearchDialog();
    
  public slots:
    void showOptionsDialog();
    void showIncomingDialog();
    void showOutgoingDialog();
    void showCategoryEditDialog();
    void showSearchDialog();
    void showArchiveDialog();
    void showFilterEditDialog(QPtrList<CalFilter> *filters);
    void showPluginDialog();

  private:
    void createOutgoingDialog();
    void createIncomingDialog();

    CalendarView *mMainView;
    
    OutgoingDialog *mOutgoingDialog;
    IncomingDialog *mIncomingDialog;
    KOPrefsDialog *mOptionsDialog;
    CategoryEditDialog *mCategoryEditDialog;
    SearchDialog *mSearchDialog;
    ArchiveDialog *mArchiveDialog;
    FilterEditDialog *mFilterEditDialog;
    PluginDialog *mPluginDialog;
};

#endif
