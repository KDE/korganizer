/*
    This file is part of KOrganizer.

    Copyright (c) 2001,2003 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KOINCIDENCEEDITOR_H
#define KOINCIDENCEEDITOR_H

#include <kdialogbase.h>

class QDateTime;

namespace KPIM { class CategorySelectDialog; }

class KOEditorDetails;
class KOEditorAttachments;

namespace KCal {
class Calendar;
class Incidence;
}
using namespace KCal;

/**
  This is the base class for the calendar component editors.
*/
class KOIncidenceEditor : public KDialogBase
{
    Q_OBJECT
  public:
    /**
      Construct new IncidenceEditor.
    */
    KOIncidenceEditor( const QString &caption, Calendar *calendar,
                       QWidget *parent );
    virtual ~KOIncidenceEditor();

    /** Initialize editor. This function creates the tab widgets. */
    virtual void init() = 0;
    /** This incidence has been modified externally */
    virtual void modified (int /*change*/=0) {}

    virtual void reload() = 0;

  public slots:
    void updateCategoryConfig();
    /** Edit an existing todo. */
    virtual void editIncidence(Incidence *) = 0;


  signals:
    void deleteAttendee( Incidence * );

    void editCategories();
    void dialogClose( Incidence * );
    void editCanceled( Incidence * );

    void incidenceAdded( Incidence * );
    void incidenceChanged( Incidence *oldTodo, Incidence *newTodo );
    void deleteIncidenceSignal( Incidence * );

  protected slots:
    void slotApply();
    void slotOk();
    void slotCancel();

    virtual void slotLoadTemplate();
    virtual void slotSaveTemplate();
    
    virtual void saveTemplate( const QString & ) = 0;

  protected:
    virtual QString type() { return QString::null; }
    
    void setupAttendeesTab();
    void setupAttachmentsTab();

    QString loadTemplate( Calendar *cal, const QString &type,
                          const QStringList &templates );
    void saveAsTemplate( Incidence *, const QString &name );

    /**
      Process user input and create or update event. Returns false if input is invalid.
    */
    virtual bool processInput() { return false; }

    virtual void processCancel() {}
    
    void cancelRemovedAttendees( Incidence *incidence );

    Calendar *mCalendar;

    KPIM::CategorySelectDialog *mCategoryDialog;

    KOEditorDetails *mDetails;
    KOEditorAttachments *mAttachments;
};

#endif


