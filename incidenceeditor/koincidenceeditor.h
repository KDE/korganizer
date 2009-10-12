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

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/
#ifndef KOINCIDENCEEDITOR_H
#define KOINCIDENCEEDITOR_H

#include "incidenceeditor_export.h"

#include <kpagedialog.h>
#include <kurl.h>
#include <QList>

#include <KCal/Incidence>

namespace KPIM {
  class DesignerFields;
  class EmbeddedURLPage;
}

namespace KOrg {
  class CalendarBase;
  class IncidenceChangerBase;
}

class KOEditorDetails;
class KOAttendeeEditor;

namespace Akonadi {
  class Item;
}
namespace KCal {
  class Calendar;
}
using namespace KCal;
using namespace KOrg;

/**
  This is the base class for the calendar component editors.
*/
class INCIDENCEEDITOR_EXPORT KOIncidenceEditor : public KPageDialog
{
  Q_OBJECT
  public:
    /**
      Construct new IncidenceEditor.
    */
    KOIncidenceEditor( const QString &caption, KOrg::CalendarBase *calendar,
                       QWidget *parent );
    virtual ~KOIncidenceEditor();

    /** This incidence has been modified externally */
    virtual void modified( int /*change*/= 0 ) {}

    virtual void reload() = 0;

    virtual void selectInvitationCounterProposal( bool enable );
    virtual void selectCreateTask( bool enable );

  public slots:
    /** Edit an existing todo. */
    virtual void editIncidence( const Akonadi::Item &, KOrg::CalendarBase * ) = 0;
    virtual void setIncidenceChanger( IncidenceChangerBase *changer )
    { mChanger = changer; }

    /** Initialize editor. This function creates the tab widgets. */
    virtual void init() = 0;

    /**
      Adds attachments to the editor
    */
    void addAttachments( const QStringList &attachments,
                         const QStringList &mimeTypes = QStringList(),
                         bool inlineAttachment = false );

    /**
      Adds attendees to the editor
    */
    void addAttendees( const QStringList &attendees );

  signals:
    void buttonClicked( int );
    void deleteAttendee( const Akonadi::Item & );

    void editCategories();
    void updateCategoryConfig();
    void dialogClose( const Akonadi::Item & );
    void editCanceled( const Akonadi::Item & );

    void deleteIncidenceSignal( const Akonadi::Item & );
    void signalAddAttachments( const QStringList &attachments,
                               const QStringList &mimeTypes = QStringList(),
                               bool inlineAttachment = false );

  protected slots:
    void reject();
    void accept();

    void openURL( const KUrl &url );

    virtual void slotButtonClicked( int button );
    virtual void slotManageTemplates();

  protected:
    virtual void closeEvent( QCloseEvent * );

    virtual QString type() { return QString(); }

    void setupAttendeesTab();
    void setupDesignerTabs( const QString &type );

    void readDesignerFields( const Akonadi::Item & );
    void writeDesignerFields( KCal::Incidence* );

    /**
      Returns true if the user made any alteration
    */
    virtual bool incidenceModified();

    // Returns the page widget. To remove the tab, just delete that one.
    QWidget *addDesignerTab( const QString &uifile );

    void setupEmbeddedURLPage( const QString &label, const QString &url,
                               const QString &mimetype );
    void createEmbeddedURLPages( const Incidence* inc );

    /**
      Process user input and create or update event.
      @return false if input is invalid.
    */
    virtual bool processInput() { return false; }

    virtual void processCancel() {}

    void cancelRemovedAttendees( const Akonadi::Item &item );

    KOrg::CalendarBase *mCalendar;

    KOEditorDetails *mDetails;
    KOAttendeeEditor *mAttendeeEditor;
    KOrg::IncidenceChangerBase *mChanger;

    QList<KPIM::DesignerFields*> mDesignerFields;
    QMap<QWidget*, KPIM::DesignerFields*> mDesignerFieldForWidget;
    QList<QWidget*> mEmbeddedURLPages;
    QList<QWidget*> mAttachedDesignerFields;
    bool mIsCounter;
    bool mIsCreateTask;
};

#endif
