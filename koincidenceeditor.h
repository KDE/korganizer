#ifndef KOINCIDENCEEDITOR_H
#define KOINCIDENCEEDITOR_H
// $Id$	

#include <kdialogbase.h>

#include <qdatetime.h>

#include "calendar.h"
#include "koeditordetails.h"

class CategorySelectDialog;

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
    KOIncidenceEditor(const QString &caption,Calendar *calendar);
    virtual ~KOIncidenceEditor(void);

    /** Initialize editor. This function creates the tab widgets. */
    void init();

  public slots:
    void updateCategoryConfig();

  signals:
    void editCategories();

  protected slots:
    void slotApply();
    void slotOk();

  protected:
    void setupGeneralTab();
    void setupDetailsTab();
    virtual void setupCustomTabs() {};

    virtual QWidget *setupGeneralTabWidget(QWidget *) = 0;

    /**
      Process user input and create or update event. Returns false if input is invalid.
    */
    virtual bool processInput() { return false; }

    Calendar *mCalendar;

    CategorySelectDialog *mCategoryDialog;

    KOEditorDetails      *mDetails;
};

#endif


