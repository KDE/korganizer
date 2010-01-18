/*
  Copyright (c) 2010 Kevin Ottens <ervin@kde.org>

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
*/

#include "kogroupwareintegration.h"

#include <kglobal.h>
#include <ksystemtimezone.h>

#include <akonadi/changerecorder.h>
#include <akonadi/collection.h>
#include <akonadi/item.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/session.h>

#include <akonadi/kcal/calendar.h>
#include <akonadi/kcal/calendarmodel.h>
#include <akonadi/kcal/groupware.h>
#include <akonadi/kcal/incidencemimetypevisitor.h>
#include <akonadi/kcal/utils.h>

#include <kcal/incidence.h>

#include "koprefs.h"
#include "koeditorconfig.h"
#include "koeventeditor.h"
#include "kojournaleditor.h"
#include "kotodoeditor.h"

class KOrganizerEditorConfig : public KOEditorConfig
{
  public:
    explicit KOrganizerEditorConfig() : KOEditorConfig() {}
    virtual ~KOrganizerEditorConfig() {}

    virtual KConfigSkeleton* config() const
    {
      return KOPrefs::instance();
    }

    virtual QString fullName() const {
      return KOPrefs::instance()->fullName();
    }
    virtual QString email() const {
      return KOPrefs::instance()->email();
    }
    virtual bool thatIsMe( const QString &email ) const {
      return KOPrefs::instance()->thatIsMe(email);
    }
    virtual QStringList allEmails() const {
      return KOPrefs::instance()->allEmails();
    }
    virtual QStringList fullEmails() const {
      return KOPrefs::instance()->fullEmails();
    }
    virtual bool showTimeZoneSelectorInIncidenceEditor() const {
      return KOPrefs::instance()->showTimeZoneSelectorInIncidenceEditor();
    }
    virtual QDateTime defaultDuration() const {
      return KOPrefs::instance()->defaultDuration();
    }
    virtual QDateTime startTime() const {
      return KOPrefs::instance()->startTime();
    }
    virtual int reminderTime() const {
      return KOPrefs::instance()->reminderTime();
    }
    virtual int reminderTimeUnits() const {
      return KOPrefs::instance()->reminderTimeUnits();
    }
    virtual bool defaultTodoReminders() const {
      return KOPrefs::instance()->defaultTodoReminders();
    }
    virtual bool defaultEventReminders() const {
      return KOPrefs::instance()->defaultEventReminders();
    }
    virtual QStringList activeDesignerFields() const {
      return KOPrefs::instance()->activeDesignerFields();
    }
    virtual QStringList& templates(const QString &type) {
      if(type == "Event") //TODO remove mEventTemplates+etc from KOPrefs::instance()
        return KOPrefs::instance()->mEventTemplates;
      if(type == "Todo")
        return KOPrefs::instance()->mTodoTemplates;
      if(type == "Journal")
        return KOPrefs::instance()->mJournalTemplates;
      return KOEditorConfig::templates(type);
    }
};

class EditorDialogVisitor : public IncidenceBase::Visitor
{
  public:
    EditorDialogVisitor() : IncidenceBase::Visitor(), mEditor( 0 ) {}
    KOIncidenceEditor *editor() const { return mEditor; }

  protected:
    bool visit( Event * )
    {
      mEditor = new KOEventEditor( 0 );
      return mEditor;
    }
    bool visit( Todo * )
    {
      mEditor = new KOTodoEditor( 0 );
      return mEditor;
    }
    bool visit( Journal * )
    {
      mEditor = new KOJournalEditor( 0 );
      return mEditor;
    }
    bool visit( FreeBusy * ) // to inhibit hidden virtual compile warning
    {
      return 0;
    }

    KOIncidenceEditor *mEditor;
};


class KOGroupwareUiDelegate : public QObject, public Akonadi::GroupwareUiDelegate
{
  public:
    KOGroupwareUiDelegate()
    {
      Akonadi::Session *session = new Akonadi::Session( "KOGroupwareIntegration", this );
      Akonadi::ChangeRecorder *monitor = new Akonadi::ChangeRecorder( this );

      Akonadi::ItemFetchScope scope;
      scope.fetchFullPayload( true );

      monitor->setSession( session );
      monitor->setCollectionMonitored( Akonadi::Collection::root() );
      monitor->fetchCollection( true );
      monitor->setItemFetchScope( scope );
      monitor->setMimeTypeMonitored( Akonadi::IncidenceMimeTypeVisitor::eventMimeType(), true );
      monitor->setMimeTypeMonitored( Akonadi::IncidenceMimeTypeVisitor::todoMimeType(), true );
      monitor->setMimeTypeMonitored( Akonadi::IncidenceMimeTypeVisitor::journalMimeType(), true );

      Akonadi::CalendarModel *calendarModel = new Akonadi::CalendarModel( monitor, this );

      mCalendar = new Akonadi::Calendar( calendarModel, calendarModel,
                                         KSystemTimeZones::local() );
      mCalendar->setOwner( Person( KOPrefs::instance()->fullName(),
                                   KOPrefs::instance()->email() ) );
    }

    void requestIncidenceEditor( const Akonadi::Item &item )
    {
      const Incidence::Ptr incidence = Akonadi::incidence( item );
      if ( !incidence ) {
        return;
      }

      EditorDialogVisitor v;
      if ( !incidence.get()->accept( v ) ) {
        return;
      }

      KOIncidenceEditor *editor = v.editor();
      editor->editIncidence( item, QDate::currentDate() );
      editor->selectInvitationCounterProposal( true );
      editor->show();
    }

    Akonadi::Calendar *mCalendar;
};

K_GLOBAL_STATIC( KOGroupwareUiDelegate, globalDelegate )

bool KOGroupwareIntegration::sActivated = false;

bool KOGroupwareIntegration::isActive()
{
  return sActivated;
}

void KOGroupwareIntegration::activate()
{
  KOEditorConfig::setKOEditorConfig( new KOrganizerEditorConfig );
  Akonadi::Groupware::create( globalDelegate->mCalendar, &*globalDelegate );
  sActivated = true;
}

