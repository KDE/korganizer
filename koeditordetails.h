/*
    This file is part of KOrganizer.
    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef _KOEDITORDETAILS_H
#define _KOEDITORDETAILS_H

#include <qframe.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qmultilineedit.h>
#include <klistview.h>
#include <qradiobutton.h>
#include <qptrlist.h>

#include <kapplication.h>

#include <libkcal/event.h>

#include "ktimeedit.h"
#include "customlistviewitem.h"

class KDateEdit;

using namespace KCal;

typedef CustomListViewItem<Attendee *> AttendeeListItem;


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

    /** return a clone of the event with attendees to be canceld*/
    void cancelAttendeeEvent(Incidence *);
    /** Check if the input is valid. */
    bool validateInput();

  public slots:
    void insertAttendee(Attendee *);

  protected slots:
    void addNewAttendee();
    void removeAttendee();
    void openAddressBook();
    void updateAttendeeInput();
    void clearAttendeeInput();
    void fillAttendeeInput(AttendeeListItem *);
    void updateAttendeeItem();
    void setEnabledAttendeeInput(bool);

  private:
    bool mDisableItemUpdate;

    QLineEdit *mNameEdit;
    QLineEdit *mUidEdit;
    QLineEdit *mEmailEdit;
    KListView *mListView;
    QComboBox* mRoleCombo;
    QCheckBox* mRsvpButton;
    QComboBox* mStatusCombo;
    QLabel *mOrganizerLabel;

    QPushButton* mAddButton;
    QPushButton* mRemoveButton;
    QPushButton* mAddressBookButton;
	
    QPtrList<Attendee> mdelAttendees;
};

#endif
