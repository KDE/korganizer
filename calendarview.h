/*
  This file is part of KOrganizer.

  Copyright (c) 2000,2001,2003,2004 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>

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

#ifndef KORG_CALENDARVIEW_H
#define KORG_CALENDARVIEW_H

#include "korganizer_export.h"
#include "interfaces/korganizer/calendarviewbase.h"

#include <KCalCore/Incidence>
#include <KCalCore/Visitor>
#include <KCalCore/ScheduleMessage>

#include <Akonadi/Calendar/ITIPHandler>

#include <calendarsupport/messagewidget.h>

class DateChecker;
class DateNavigator;
class DateNavigatorContainer;
class KODialogManager;
class KOTodoView;
class KOViewManager;
class NavigatorBar;
class KOCheckableProxyModel;
class AkonadiCollectionView;
namespace KOrg {
  class HTMLExportSettings;
}

namespace CalendarSupport {
  class CalPrinter;
  class IncidenceViewer;
}

namespace IncidenceEditorNG {
  class IncidenceDialog;
}

namespace Akonadi {
  class History;
  class IncidenceChanger;
  class CalendarClipboard;
  class TodoPurger;
}


class QSplitter;
class QStackedWidget;

using namespace KOrg;

class CalendarViewExtension : public QWidget
{
  public:
    explicit CalendarViewExtension( QWidget *parent ) : QWidget( parent ) {}

    class Factory
    {
      public:
        virtual ~Factory() {}
        virtual CalendarViewExtension *create( QWidget *parent ) = 0;
    };
};

/**
  This is the main calendar widget. It provides the different views on the
  calendar data as well as the date navigator. It also handles synchronization
  of the different views and controls the different dialogs like preferences,
  event editor, search dialog etc.

  @short main calendar view widget
  @author Cornelius Schumacher
*/
class KORGANIZERPRIVATE_EXPORT CalendarView : public KOrg::CalendarViewBase,
                                              public Akonadi::ETMCalendar::CalendarObserver
{
  Q_OBJECT
  public:
    /**
      Constructs a new calendar view widget.
      @param parent   parent window
    */
    explicit CalendarView( QWidget *parent=0 );
    virtual ~CalendarView();

    class CalendarViewVisitor : public KCalCore::Visitor
    {
      public:
        CalendarViewVisitor() : mView( 0 )
        {
        }

        bool act( KCalCore::IncidenceBase::Ptr &incidence, CalendarView *view )
        {
          mView = view;
          return incidence->accept( *this, incidence );
        }

      protected:
        CalendarView *mView;
    };

    void setCalendar( const Akonadi::ETMCalendar::Ptr & );
    Akonadi::ETMCalendar::Ptr calendar() const;

    void showMessage(const QString &message, KMessageWidget::MessageType);

    Akonadi::History *history() const;
    void setCheckableProxyModel( KOCheckableProxyModel * );

    KOViewManager *viewManager() const { return mViewManager; }
    KODialogManager *dialogManager() const { return mDialogManager; }

    QStackedWidget *viewStack() const { return mRightFrame; }
    QWidget *leftFrame() const { return mLeftFrame; }
    NavigatorBar *navigatorBar() const { return mNavigatorBar; }
    DateNavigator *dateNavigator() const { return mDateNavigator; }
    // TODO_NG
    //IncidenceEditors::IncidenceEditor *editorDialog( const Akonadi::Item &item ) const;
    virtual Akonadi::IncidenceChanger *incidenceChanger() const { return mChanger; }

    /**
     * Informs the date navigator which incidence types should be used
     * to embolden days, this function is mainly called when the view changes
     * or when the config changes.
     */
    void updateHighlightModes();

    QDate startDate();
    QDate endDate();

    KOrg::BaseView *currentView() const;
    void addView( KOrg::BaseView * );
    void showView( KOrg::BaseView * );

    /**
     * Adds a calendar view extension widget. CalendarView takes ownership of the
     * objects created by the factory.
     */
    void addExtension( CalendarViewExtension::Factory * );

    /**
     * Returns the item selected in the current view (or an invalid one if none selected)
     * @reimp
     */
    Akonadi::Item currentSelection();

    /**
     * Returns a pointer to the incidence selected in the current view. If there
     * is no selection, return the selected todo from the todo list on the left.
     */
    Akonadi::Item selectedIncidence();

    /**
     * Returns true if there's a filter applied.
     */
    bool isFiltered() const;

    /**
     * Returns the name of the current filter.
     */
    QString currentFilterName() const;

  signals:
    /** when change is made to options dialog, the topwidget will catch this
     *  and emit this signal which notifies all widgets which have registered
     *  for notification to update their settings. */
    void configChanged();

    /** Emitted right before we die */
    void closed( QWidget * );

    /** Emitted when state of modified flag changes */
    void modifiedChanged( bool );

    /** Emitted when state of read-only flag changes */
    void readOnlyChanged( bool );

    /** Emitted when the unit of navigation changes */
    void changeNavStringPrev( const QString & );
    void changeNavStringNext( const QString & );

    /** Emitted when state of events selection has changed and user is organizer*/
    void organizerEventsSelected( bool );
    /** Emitted when state of events selection has changed and user is attendee*/
    void groupEventsSelected( bool );
    /**
      Emitted when an incidence gets selected. If the selection is cleared the
      signal is emitted with 0 as argument.
    */
    void incidenceSelected( const Akonadi::Item &incidence, const QDate &date );
    /** Emitted, when a todoitem is selected or deselected.
        the connected slots enables/disables the corresponding menu items */
    void todoSelected( bool );
    void subtodoSelected( bool );

    /** Emitted, when a day changed (i.e. korganizer was running at midnight).
        The argument is the new date */
    void dayPassed( const QDate & );

    /**
      Emitted, when clipboard content changes. Parameter indicates if paste
      is possible or not.
    */
    void pasteEnabled( bool );
    /** Send status message, which can e.g. be displayed in the status bar. */
    void statusMessage( const QString & );

    void calendarViewExpanded( bool );

    /** Emitted when auto-archiving options were modified */
    void autoArchivingSettingsModified();

    void newIncidenceChanger( Akonadi::IncidenceChanger * );
    void exportHTML( KOrg::HTMLExportSettings * );

    void filtersUpdated( const QStringList &, int );
    void filterChanged();

  public slots:
    /** options dialog made a changed to the configuration. we catch this
     *  and notify all widgets which need to update their configuration. */
    void updateConfig();
    void updateConfig( const QByteArray & );

    void handleIncidenceCreated(const Akonadi::Item &item);

    /**
      Save calendar data to file. Return true if calendar could be
      successfully saved.
        @param filename The file name to save the calendar to
    */
    bool saveCalendar( const QString &filename );

    /** Archive old events of calendar */
    void archiveCalendar();

    void newEvent( const QDate & );

    /**
       create new event without having a date hint. Takes current date as
       default hint.
    */
    void newEvent();

    /**
       create an editeventwin with supplied date/time, and if bool is true,
       make the event take all day.
    */
    void newEvent( const QDateTime &startDt );

    void newEvent( const QDateTime &startDt, const QDateTime &EndDt, bool allDay=false );

    /**
      Create new Event from given summary, description, attachment list and
      attendees list
    */
    void newEvent( const QString &summary,
                   const QString &description=QString(),
                   const QStringList &attachment=QStringList(),
                   const QStringList &attendees=QStringList(),
                   const QStringList &attachmentMimetypes=QStringList(),
                   bool inlineAttachment=false );
    void newFloatingEvent();

    /** Create a read-only viewer dialog for the supplied incidence.
        It calls the correct showXXX method */
    void showIncidence( const Akonadi::Item &item );
    bool showIncidence( Akonadi::Item::Id id );
    void showIncidence();

    /**
      Show an incidence in context. This means showing the todo, agenda or
      journal view (as appropriate) and scrolling it to show the incidence.
      @param incidence The incidence to show.
    */
    void showIncidenceContext( const Akonadi::Item &incidence );
    bool showIncidenceContext( Akonadi::Item::Id id );

    /** Create an editor for the supplied incidence. It calls the correct editXXX method*/
    bool editIncidence( const Akonadi::Item &item, bool isCounter = false );
    bool editIncidence( Akonadi::Item::Id id );
    void editIncidence();

    /**
      Delete the supplied incidence. It calls the correct deleteXXX method
      @param force If true, all recurrences and sub-todos (if applicable) will be
                   deleted without prompting for confirmation.
      @param force If true, all recurrences and sub-todos (if applicable) will be
                   deleted without prompting for confirmation.
    */
    bool deleteIncidence( const Akonadi::Item &item, bool force=false );
    bool deleteIncidence( Akonadi::Item::Id id, bool force=false );
    void deleteIncidence();

     /**
      Add an incidence to the active calendar.
      @param ical A calendar in iCalendar format containing the incidence. The
                  calendar must consist of a VCALENDAR component which contains
                  the incidence (VEVENT, VTODO, VJOURNAL or VFREEBUSY) and
                  optionally a VTIMEZONE component. If there is more than one
                  incidence, only the first is added to KOrganizer's calendar.
    */
    bool addIncidence( const QString &ical );
    bool addIncidence( const KCalCore::Incidence::Ptr &incidence );

    /**
      Cuts the selected incidence using the edit_cut() method
    */
    void cutIncidence( const Akonadi::Item & );

    /**
      Copies the selected incidence using the edit_copy() method
    */
    void copyIncidence( const Akonadi::Item & );

    /**
      Pastes the current incidence using the edit_paste() method
    */
    void pasteIncidence();

    /** Delete the supplied todo and all sub-todos */
    void deleteSubTodosIncidence ( const Akonadi::Item &todo );

    /**
      Delete the todo incidence, and its sub-to-dos.
      @param todo The todo to delete.
      @param force If true, all sub-todos will be deleted without prompting for confirmation.
    */
    void deleteTodoIncidence ( const Akonadi::Item &todo, bool force=false );

    /** create new todo */
    void newTodo();

    /** create new todo, due on date */
    void newTodo( const QDate &date );

    /** create new todo **/
    void newTodo( const Akonadi::Collection &collection );

    /** create new todo with a parent todo */
    void newSubTodo();

    /** create new todo with a parent todo */
    void newSubTodo( const Akonadi::Item &todo );

    /** create new todo with parent todo */
    void newSubTodo( const Akonadi::Collection &collection );

    void newTodo( const QString &summary, const QString &description=QString(),
                  const QStringList &attachments=QStringList(),
                  const QStringList &attendees=QStringList(),
                  const QStringList &attachmentMimetypes=QStringList(),
                  bool inlineAttachment=false );

    void newJournal();
    void newJournal( const QDate &date );
    void newJournal( const QString &text, const QDate &date=QDate() );
    void newJournal( const Akonadi::Collection &collection );

    void configureCurrentView();

    void toggleAlarm( const Akonadi::Item &incidence );
    void toggleTodoCompleted( const Akonadi::Item &incidence );
    void copyIncidenceToResource( const Akonadi::Item &incidence, const QString &resourceId );
    void moveIncidenceToResource( const Akonadi::Item &incidence, const QString &resourceId );
    void dissociateOccurrences( const Akonadi::Item &incidence, const QDate &date );

    /**
      Check if clipboard contains vCalendar event. The signal pasteEnabled() is
      emitted as result.
    */
    void checkClipboard();

    /**
      Using the KConfig associated with the kapp variable, read in the
      settings from the config file.

      You have to call setCalendar before calling readSettings.
    */
    void readSettings();

    /** write current state to config file. */
    void writeSettings();

    /** read settings for calendar filters */
    void readFilterSettings( KConfig *config );

    /** write settings for calendar filters */
    void writeFilterSettings( KConfig *config );

    /** passes on the message that an event has changed to the currently
     * activated view so that it can make appropriate display changes. */
    void changeIncidenceDisplay( const Akonadi::Item &incidence,
                                 Akonadi::IncidenceChanger::ChangeType );

    void slotCreateFinished( int changeId,
                             const Akonadi::Item &item,
                             Akonadi::IncidenceChanger::ResultCode resultCode,
                             const QString &errorString );

    void slotModifyFinished( int changeId,
                             const Akonadi::Item &item,
                             Akonadi::IncidenceChanger::ResultCode resultCode,
                             const QString &errorString );

    void slotDeleteFinished( int changeId,
                             const QVector<Akonadi::Item::Id> &itemIdList,
                             Akonadi::IncidenceChanger::ResultCode resultCode,
                             const QString &errorString );

    void startMultiModify( const QString &text );
    void endMultiModify();

    void updateView( const QDate &start, const QDate &end,
                     const QDate &preferredMonth, const bool updateTodos=true );
    void updateView();

    void updateUnmanagedViews();

    /** cut the current appointment to the clipboard */
    void edit_cut();

    /** copy the current appointment(s) to the clipboard */
    void edit_copy();

    /** paste the current vobject(s) in the clipboard buffer into calendar */
    void edit_paste();

    void onCutFinished();

    /** edit viewing and configuration options. */
    void edit_options();

    /**
      Functions for printing, previewing a print, and setting up printing
      parameters.
    */
    void print();
    void printPreview();

    /** Export as HTML file */
    void exportWeb();

    /** Export as iCalendar file */
    void exportICalendar();

    /** Export as vCalendar file */
    void exportVCalendar();

    /** pop up a dialog to show an existing appointment. */
    void appointment_show();
    /**
     * pop up an Appointment Dialog to edit an existing appointment. Get
     * information on the appointment from the list of unique IDs that is
     * currently in the View, called currIds.
     */
    void appointment_edit();

    /**
     * pop up dialog confirming deletion of currently selected event in the
     * View.
     */
    void appointment_delete();

    /** Frees a subtodo from it's relation, update the view */
    void todo_unsub();

    /* Frees an incidence's children from it's relation, without the view update
       Works with any incidence type, although currently we only pass to-dos
    */
    bool incidence_unsub( const Akonadi::Item &item );

    /** Make all sub-to-dos of the selected todo independent, update the view */
    bool makeSubTodosIndependent( );

    /** Make all children of incidence independent, not update the view
        Works with any incidence type, although currently we only pass to-dos
    */
    bool makeChildrenIndependent( const Akonadi::Item &item );

    /** Take ownership of selected event. */
    void takeOverEvent();

    /** query if the calendar is read-only. */
    bool isReadOnly() const;

    /** set state of calendar to read-only
        @param readOnly whether the calendar view should be set read-only or not
    */
    void setReadOnly( bool readOnly=true );

    void eventUpdated( const Akonadi::Item &incidence );

    /* iTIP scheduling actions */
    void schedule_publish( const Akonadi::Item &incidence=Akonadi::Item() );
    void schedule_request( const Akonadi::Item &incidence=Akonadi::Item() );
    void schedule_refresh( const Akonadi::Item &incidence=Akonadi::Item() );
    void schedule_cancel( const Akonadi::Item &incidence=Akonadi::Item() );
    void schedule_add( const Akonadi::Item &incidence=Akonadi::Item() );
    void schedule_reply( const Akonadi::Item &incidence=Akonadi::Item() );
    void schedule_counter( const Akonadi::Item &incidence=Akonadi::Item() );
    void schedule_declinecounter( const Akonadi::Item &incidence=Akonadi::Item() );
    void schedule_forward( const Akonadi::Item &incidence=Akonadi::Item() );
    void mailFreeBusy( int daysToPublish=30 );
    void uploadFreeBusy();

    void openAddressbook();

    void editFilters();

    void updateFilter();

    void showIntro();

    void showDateNavigator( bool );
    void showTodoView( bool );
    void showEventViewer( bool );

    /** Move the current view date to the specified date */
    void goDate( const QDate &date );

    /** Show the given date without changing date selection length. */
    void showDate( const QDate &date );

    /** Move the current view date to today */
    void goToday();

    /** Move to the next date(s) in the current view */
    void goNext();

    /** Move to the previous date(s) in the current view */
    void goPrevious();

    void showLeftFrame( bool show=true );

    void dialogClosing( const Akonadi::Item &incidence );

    void processMainViewSelection( const Akonadi::Item &incidence, const QDate &date );
    void processTodoListSelection( const Akonadi::Item &incidence, const QDate &date );

    void processIncidenceSelection( const Akonadi::Item &incidence, const QDate &date );

    void purgeCompleted();

    void slotAutoArchivingSettingsModified() { emit autoArchivingSettingsModified(); }

    void showErrorMessage( const QString & );
    void schedule( KCalCore::iTIPMethod, const Akonadi::Item &incidence );
    void addIncidenceOn( const Akonadi::Item &incidence, const QDate & );
    void moveIncidenceTo( const Akonadi::Item &incidence, const QDate & );
    void filterActivated( int filterNum );

    void resourcesChanged();

    /**
     * The user clicked on a week number in the date navigator
     *
     * Select a week or a work week depending on the user's config option.
     *
     * @param preferredMonth Holds the month that should be selected when
     * the week crosses months.  It's a QDate instead of uint so it can be
     * easily fed to KCalendarSystem's functions.
    */
    void selectWeek( const QDate &week, const QDate &preferredMonth );

    /**
     * Use as much of the full window as possible for the view.
     *
     * @param fullView if true, expand the view as much as possible within the
     * main view (hiding the sidebar for example); else put back the normal view.
     */
    void changeFullView( bool fullView );

  protected slots:
    /**
     * Select a view or adapt the current view to display the specified dates.
     * @p preferredMonth is useful when the datelist crosses months, if valid,
     * any month-like component should honour this
     */
    void showDates( const KCalCore::DateList &, const QDate &preferredMonth = QDate() );

  public:
    int msgCalModified();

    /**
     * Adapts navigation units according to the current view's navigation step size.
     */
    void adaptNavigationUnits();

    /**
     * Returns the date of the selected incidence.
     *
     * If the selected incidence is recurring, it will return the date
     * of the selected occurrence.
     */
    QDate activeIncidenceDate();

    /**
      Returns the best guess at the current active date in the view.
      This has nothing to do with selected incidences, use activeIncidenceDate()
      for that, for example, agenda supports time selection and incidence selection
      and they can have diferent dates.

      @param fallbackToToday If guessing doesn't work, some views will prefer
      today to be returned instead of the first select date in the day matrix,
      Journal view for example.
    */
    QDate activeDate( bool fallbackToToday = false );

  protected:
    int msgItemDelete( const Akonadi::Item &incidence );

    Akonadi::Item selectedTodo();
    IncidenceEditorNG::IncidenceDialog *incidenceDialog(const Akonadi::Item &);

    void warningChangeFailed( const Akonadi::Item & );
    void checkForFilteredChange( const Akonadi::Item &incidence );

    /**
      Adjust the given date/times by valid defaults (selection or configured
      defaults, if invalid values are given) and allow the view to adjust the type.
    */
    void dateTimesForNewEvent( QDateTime &startDt, QDateTime &endDt, bool &allDay );
    IncidenceEditorNG::IncidenceDialog *newEventEditor( const KCalCore::Event::Ptr &event );

    bool eventFilter( QObject *watched, QEvent *event );

  private Q_SLOTS:
    void onCheckableProxyAboutToToggle( bool newState );
    void onCheckableProxyToggled( bool newState );
    void onTodosPurged(bool success, int numDeleted, int numIgnored);

  private:
    void init();
    Akonadi::Collection selectedCollection() const;
    Akonadi::Collection::List checkedCollections() const;

    void createPrinter();

    void dissociateOccurrence( const Akonadi::Item &incidence, const QDate & , bool futureOccurrences);

    /**
     * Returns the default collection.
     * The view's collection takes precedence, only then the config one is used.
     * If mimeType is set, the collection to return will have to support that mime type.
     * If no valid collection is found, an invalid one is returned.
     */
    Akonadi::Collection defaultCollection(
      const QLatin1String &mimeType = QLatin1String( "" ) ) const;

    /**
     * Creates a new incidence editor and chooses a decent default for the collection
     * in the collection combo.
     */
    IncidenceEditorNG::IncidenceDialog *createIncidenceEditor(
      const Akonadi::Item &item, const Akonadi::Collection &collection = Akonadi::Collection() );

    CalendarSupport::CalPrinter *mCalPrinter;
    Akonadi::TodoPurger *mTodoPurger;

    QSplitter *mPanner;
    QSplitter *mLeftSplitter;
    QWidget *mLeftFrame;
    QStackedWidget *mRightFrame;
    CalendarSupport::MessageWidget *mMessageWidget;

    // This navigator bar is used when in full window month view
    // It has nothing to do with the date navigator
    NavigatorBar *mNavigatorBar;

    DateNavigatorContainer *mDateNavigatorContainer;

    QList<CalendarViewExtension*> mExtensions;

    Akonadi::ETMCalendar::Ptr mCalendar;

    DateNavigator *mDateNavigator;
    DateChecker *mDateChecker;

    QWidget *mEventViewerBox;
    CalendarSupport::IncidenceViewer *mEventViewer;
    KOViewManager *mViewManager;
    KODialogManager *mDialogManager;

    // Calendar filters
    QList<KCalCore::CalFilter*> mFilters;
    KCalCore::CalFilter *mCurrentFilter;

    // various housekeeping variables.
    bool  mReadOnly; // flag indicating if calendar is read-only

    Akonadi::Item mSelectedIncidence;
    QDate mSaveDate;

    KOTodoView *mTodoList;
    Akonadi::IncidenceChanger *mChanger;
    Akonadi::ITIPHandler *mITIPHandler;
    QList<int> mMainSplitterSizes; // temp store for main splitter sizes while left frame is hidden
    bool mSplitterSizesValid;
    bool mCreatingEnabled;

    Akonadi::CalendarClipboard *mCalendarClipboard;
    KOCheckableProxyModel *mCheckableProxyModel;
    AkonadiCollectionView *mETMCollectionView;
};

#endif
