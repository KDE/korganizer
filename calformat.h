// $Id$

#ifndef _CALFORMAT_H
#define _CALFORMAT_H

#include <qstring.h>
#include <qdatetime.h>

#include "koexceptions.h"
#include "event.h"

#define _PRODUCT_ID "-//K Desktop Environment//NONSGML KOrganizer//EN"

class VCalDrag;
class Calendar;

/**
  This is the base class for calendar formats. It provides an interface for the
  generation/interpretation of a textual representation of a calendar.

  @short class providing in interface to a calendar format
  @author Cornelius Schumacher
  @version $Revision$
*/
class CalFormat {
  public:
    /** Constructs a new format for the calendar given as argument. */
    CalFormat(Calendar *);
    virtual ~CalFormat();

    /** Associate a widget with this format */
    void setTopwidget(QWidget *topWidget);
    
    /**
      loads a calendar on disk into the calendar associated with this format.
      Returns TRUE if successful,else returns FALSE.
      @param fileName the name of the calendar on disk.
    */
    virtual bool load(const QString &fileName) = 0;
    /** writes out the calendar to disk. Returns true if
     * successful and false on error.
     * @param fileName the name of the file
     */
    virtual bool save(const QString &fileName) = 0;
  
    /** create an object to be used with the Xdnd Drag And Drop protocol. */
    virtual VCalDrag *createDrag(Event *selectedEv, QWidget *owner) = 0;
    /** create an object to be used with the Xdnd Drag And Drop protocol. */
    virtual VCalDrag *createDragTodo(Todo *selectedEv, QWidget *owner) = 0;

    /** Create Todo object from drop event */
    virtual Todo *createDropTodo(QDropEvent *de) = 0;
    /** Create Event object from drop event */
    virtual Event *createDrop(QDropEvent *de) = 0;
  
    /** cut, copy, and paste operations follow. */
    virtual bool copyEvent(Event *) = 0;
    /** pastes the event and returns a pointer to the new event pasted. */
    virtual Event *pasteEvent(const QDate *, const QTime *newTime = 0L) = 0;
    
    void showDialogs(bool);

    /** Clear exception status of this format object */
    void clearException();
    /**
      Return exception, if there is any, containing information about the last
      error that occured.
    */
    KOErrorFormat *exception();

    static QString createUniqueId();
  
  protected:  
    /** shows an error dialog box. */
    void loadError(const QString &fileName);
  
    /**
      Set exception for this object. This is used by the functions of this
      class to report errors.
    */
    void setException(KOErrorFormat *error);
  
    QWidget *mTopWidget;      // topWidget used for message boxes
    bool mEnableDialogs;      // display various GUI dialogs?

    Calendar *mCalendar;
  
  private:
    QList<Event> mEventsRelate;           // events with relations
    QList<Event> mTodosRelate;            // todos with relations
    
    KOErrorFormat *mException;
};

#endif
