#ifndef _KOEDITORDETAILS_H
#define _KOEDITORDETAILS_H
// $Id$

#include <qframe.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qmultilineedit.h>
#include <qlistview.h>
#include <qradiobutton.h>

#include <kapp.h>
#include <krestrictedline.h>

#include <event.h>

#include "ktimeedit.h"
#include "kdateedit.h"

using namespace KCal;

class AttendeeListItem : public QListViewItem
{
  public:
    AttendeeListItem(Attendee *a, QListView *parent);
    ~AttendeeListItem();

    Attendee *attendee() { return mAttendee; }
    void updateItem();
    
  private:
    Attendee *mAttendee;
};

class KOEditorDetails : public QWidget
{
    Q_OBJECT
  public:
    KOEditorDetails (int spacing = 8,QWidget* parent = 0, const char* name = 0);
    virtual ~KOEditorDetails();

    /** Set widgets to default values */
    void setDefaults();
    /** Read event object and setup widgets accordingly */
    void readEvent(Incidence *);
    /** Write event settings to event object */
    void writeEvent(Incidence *);

    /** Check if the input is valid. */
    bool validateInput();

  public slots:
    void insertAttendee(Attendee *);

  protected slots:
    void addNewAttendee();
    void updateAttendee();
    void removeAttendee();
    void attendeeListHilite(QListViewItem *);
    void attendeeListAction(QListViewItem *);
    void openAddressBook();
    void checkLineEdits();
    void checkAttendeeSelection();

  private:
    void clearAttendeeInput();

    QLineEdit *mNameEdit;
    QLineEdit *mEmailEdit;
    QListView *mListView;
    QPushButton* mAddButton;
    QPushButton* mModifyButton;
    QPushButton* mRemoveButton;
    QPushButton* mAddressBookButton;
    QComboBox* mRoleCombo;
    QCheckBox* mRsvpButton;
    QComboBox* mStatusCombo;

};

#endif
