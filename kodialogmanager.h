/*
  This file is part of KOrganizer.

  Copyright (c) 2001,2004 Cornelius Schumacher <schumacher@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/
#ifndef KODIALOGMANAGER_H
#define KODIALOGMANAGER_H

#include <qobject.h>
#include <qptrlist.h>

namespace KCal{class CalFilter; }
class CalendarView;
class OutgoingDialog;
class IncomingDialog;
class KCMultiDialog;
class KConfigureDialog;
namespace KPIM { class CategoryEditDialog; }
class KOIncidenceEditor;
class KOEventEditor;
class KOTodoEditor;
class KOJournalEditor;
class SearchDialog;
class ArchiveDialog;
class FilterEditDialog;
class KOAgendaView;

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

    /** Get the appropriate editor for the given incidence */
    KOIncidenceEditor *getEditor( Incidence * );
    /** Get an editor dialog for an Event. */
    KOEventEditor *getEventEditor();
    /** Get an editor dialog for a Todo. */
    KOTodoEditor *getTodoEditor();
    /** Get an editor dialog for a Journal. */
    KOJournalEditor *getJournalEditor();
    void connectEditor( KOIncidenceEditor*editor );

    OutgoingDialog *outgoingDialog();

    IncomingDialog *incomingDialog();

    void updateSearchDialog();
    void setDocumentId( const QString &id );

    void connectTypeAhead( KOEventEditor *editor, KOAgendaView *agenda );

    static void errorSaveIncidence( QWidget *parent, Incidence *incidence );

  public slots:
    void showOptionsDialog();
    void showIncomingDialog();
    void showOutgoingDialog();
    void showCategoryEditDialog();
    void showSearchDialog();
    void showArchiveDialog();
    void showFilterEditDialog(QPtrList<CalFilter> *filters);

  private:
    void createOutgoingDialog();
    void createIncomingDialog();

    class DialogManagerVisitor;
    class EditorDialogVisitor;
    
    CalendarView *mMainView;

    OutgoingDialog *mOutgoingDialog;
    IncomingDialog *mIncomingDialog;
    KCMultiDialog *mOptionsDialog;
//    KConfigureDialog *mOptionsDialog;
    KPIM::CategoryEditDialog *mCategoryEditDialog;
    SearchDialog *mSearchDialog;
    ArchiveDialog *mArchiveDialog;
    FilterEditDialog *mFilterEditDialog;
};

#endif
