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

#include <Akonadi/Item>

#include <KCal/Incidence>

#include <KDialog>

#include <QList>

class KOAttendeeEditor;
class KOEditorDetails;

namespace KOrg {
  class IncidenceChangerBase;
}
using namespace KOrg;

namespace KPIM {
  class DesignerFields;
  class EmbeddedURLPage;
}

namespace Akonadi {
  class CollectionComboBox;
  class Monitor;
}
using namespace Akonadi;

using namespace KCal;

class KUrl;

class QTabWidget;

/**
  This is the base class for the calendar component editors.
*/
class KORG_INCIDENCEEDITOR_EXPORT KOIncidenceEditor : public KDialog
{
  Q_OBJECT
  public:
    /**
      Construct new IncidenceEditor.
    */
    KOIncidenceEditor( const QString &caption, const QStringList &mimetypes, QWidget *parent );
    virtual ~KOIncidenceEditor();

    /** This incidence has been modified externally */
    virtual void modified() {}

    /** Read incidence. */
    virtual void readIncidence( const Akonadi::Item &item, const QDate &date, bool tmpl = false );
    /** Edit an existing incidence. */
    virtual void editIncidence( const Akonadi::Item &item, const QDate &date );

    /** Calls readIncidence(mIncidence) */
    void reload();

    void selectCollection( const Akonadi::Collection &collection );
    virtual void selectInvitationCounterProposal( bool enable );
    virtual void selectCreateTask( bool enable );

  public slots:
    /** Edit an existing todo. */
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

    void deleteIncidenceSignal( const Akonadi::Item & );
    void signalAddAttachments( const QStringList &attachments,
                               const QStringList &mimeTypes = QStringList(),
                               bool inlineAttachment = false );

  protected slots:
    void reject();
    void accept();
    void openURL( const KUrl &url );
    void slotButtonClicked( int button );

  private slots:
    void slotSelectedCollectionChanged();
    void slotItemChanged( const Akonadi::Item &item );
    void slotItemRemoved( const Akonadi::Item &item );

    void slotManageTemplates();
    void slotLoadTemplate( const QString &templateName );
    void slotSaveTemplate( const QString &templateName );
    void slotTemplatesChanged( const QStringList &templateNames );

  protected:
    virtual bool read( const Akonadi::Item &item, const QDate &date, bool tmpl = false ) = 0;
    virtual void closeEvent( QCloseEvent * );

    virtual QString type() = 0;

    void setupAttendeesTab();
    void setupDesignerTabs( const QString &type );

    void readDesignerFields( const Akonadi::Item &item );
    void writeDesignerFields( Incidence * );

    /**
      Returns true if the user made any alteration
    */
    virtual bool incidenceModified();

    // Returns the page widget. To remove the tab, just delete that one.
    QWidget *addDesignerTab( const QString &uifile );

    void setupEmbeddedURLPage( const QString &label, const QString &url,
                               const QString &mimetype );
    void createEmbeddedURLPages( const Incidence *inc );

    /**
      Process user input and create or update event.
      @return false if input is invalid.
    */
    virtual bool processInput() { return false; }

    virtual void processCancel() {}

    void cancelRemovedAttendees( const Akonadi::Item &item );

    QTabWidget *mTabWidget;
    Akonadi::CollectionComboBox *mCalSelector;
    KOEditorDetails *mDetails;
    KOAttendeeEditor *mAttendeeEditor;
    IncidenceChangerBase *mChanger;

    QList<KPIM::DesignerFields*> mDesignerFields;
    QMap<QWidget*, KPIM::DesignerFields*> mDesignerFieldForWidget;
    QList<QWidget*> mEmbeddedURLPages;
    QList<QWidget*> mAttachedDesignerFields;
    bool mIsCounter;
    bool mIsCreateTask;

    Akonadi::Item mIncidence;
    Akonadi::Monitor *mMonitor;
};

#endif
