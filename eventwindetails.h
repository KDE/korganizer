// 	$Id$	

#ifndef _EVENTWINDETAILS_H
#define _EVENTWINDETAILS_H

#include <qframe.h>
#include <qlabel.h>
#include <qchkbox.h>
#include <qpushbt.h>
#include <qgrpbox.h>
#include <qlined.h>
//#include <ktablistbox.h>
#include <qlistview.h>
#include <qcombo.h>
#include <qlayout.h>
#include <kapp.h>

#include "koevent.h"

class EventWin;
class EditEventWin;
class TodoEventWin;


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


class EventWinDetails : public QFrame
{
  Q_OBJECT

  friend EventWin;
  friend EditEventWin;
  friend TodoEventWin;

public:

  EventWinDetails(QWidget* parent = 0, const char* name = 0, bool isTodo = FALSE);
  virtual ~EventWinDetails();

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
  void setModified();

signals:
  void nullSignal(QWidget *);
  void modifiedEvent();

protected:
  bool isTodo;
  void initAttendee();
  void initAttach();
  void initMisc();

  QVBoxLayout *topLayout;

  QGroupBox* attendeeGroupBox;
  QLabel* attendeeLabel;
  QLineEdit *attendeeEdit;
  QLineEdit *emailEdit;
  QListView *attendeeListBox;
  QPushButton* addAttendeeButton;
  QPushButton* removeAttendeeButton;
  QGroupBox* attachGroupBox;
  QPushButton* attachFileButton;
  QPushButton* removeFileButton;
//  KTabListBox* attachListBox;
  QPushButton* saveFileAsButton;
  QPushButton* addressBookButton;
  QPushButton* categoriesButton;
  QLabel* categoriesLabel;
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

  QList<AttendeeListItem> mAttendeeList; // list of attendee items
};

#endif
