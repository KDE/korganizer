/*
    This file is part of KOrganizer.

    Copyright (c) 2001,2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

namespace KPIM {
class CategorySelectDialog;
class DesignerFields;
}

namespace KOrg { class IncidenceChangerBase; }

class KOEditorDetails;
class KOEditorAttachments;
class KOEditorAlarms;

namespace KCal {
class Calendar;
class Incidence;
}
using namespace KCal;
using namespace KOrg;

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
    virtual void setIncidenceChanger( IncidenceChangerBase *changer ) { 
        mChanger = changer; }


  signals:
    void deleteAttendee( Incidence * );

    void editCategories();
    void dialogClose( Incidence * );
    void editCanceled( Incidence * );

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
    void setupAlarmsTab();
    void setupDesignerTabs( const QString &type );

    QString loadTemplate( Calendar *cal, const QString &type,
                          const QStringList &templates );
    void saveAsTemplate( Incidence *, const QString &name );

    void readDesignerFields( Incidence *i );
    void writeDesignerFields( Incidence *i );

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
    KOEditorAlarms *mAlarms;
    KOrg::IncidenceChangerBase *mChanger;
    
    QPtrList<KPIM::DesignerFields> mDesignerFields;
};

#endif


