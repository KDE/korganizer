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

    /** Set spacing for layouts */
    void setSpacing(int);

  public slots:
    virtual void setEnabled(bool);
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

  protected:
    void initAttendee();
    void initAttach();
    void initMisc();

    void clearAttendeeInput();

    QGridLayout *topLayout;

    QLabel* attendeeLabel;
    QLineEdit *attendeeEdit;
    QLineEdit *emailEdit;
    QListView *attendeeListBox;
    QPushButton* addAttendeeButton;
    QPushButton* modifyAttendeeButton;
    QPushButton* removeAttendeeButton;
    QGroupBox* attachGroupBox;
    QPushButton* attachFileButton;
    QPushButton* removeFileButton;
    QPushButton* saveFileAsButton;
    QPushButton* addressBookButton;
    QLabel* attendeeRoleLabel;
    QComboBox* attendeeRoleCombo;
    QCheckBox* attendeeRSVPButton;
    QLabel* statusLabel;
    QComboBox* statusCombo;
    QLabel* locationLabel;
    QLabel* priorityLabel;
    QComboBox* priorityCombo;
    QPushButton* resourceButton;
    QLineEdit* resourcesEdit;
    QLabel* transparencyLabel;
    QLabel* transparencyAmountLabel;

  private:
    int mSpacing;
};

#endif
