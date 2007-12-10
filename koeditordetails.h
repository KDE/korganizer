/*
    This file is part of KOrganizer.
    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef _KOEDITORDETAILS_H
#define _KOEDITORDETAILS_H

#include <klistview.h>
#include "customlistviewitem.h"
#include "koattendeeeditor.h"

#include <libkcal/attendee.h>

class QPushButton;
class QCheckBox;
class QLineEdit;
class QLabel;
class QComboBox;
class QHBox;
class KDateEdit;
class KOEditorFreeBusy;

namespace KCal {
class Attendee;
class Incidence;
}
using namespace KCal;

namespace KPIM {
class AddresseeLineEdit;
}

typedef CustomListViewItem<KCal::Attendee *> AttendeeListItem;


/** KOAttendeeListView is a child class of KListView  which supports
 *  dropping of attendees (e.g. from kaddressbook) onto it. If an attendeee
 *  was dropped, the signal dropped(Attendee*)  is emitted. Valid drop classes
 *   are KVCardDrag and QTextDrag.
 */
class KOAttendeeListView : public KListView
{
Q_OBJECT
public:
  KOAttendeeListView (QWidget *parent=0, const char *name=0);
  virtual ~KOAttendeeListView();
  virtual void addAttendee( const QString& newAttendee );
public slots:
  virtual void contentsDragEnterEvent( QDragEnterEvent *e );
  virtual void dragEnterEvent( QDragEnterEvent *e );
  virtual void contentsDropEvent( QDropEvent *e );
  virtual void dropEvent( QDropEvent *e );
  virtual void contentsDragMoveEvent(QDragMoveEvent *e);
signals:
  void dropped(Attendee*);
};


class KOEditorDetails : public KOAttendeeEditor
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

    /** Returns whether at least one attendee was added */
    bool hasAttendees();

    void insertAttendee( Attendee*, bool goodEmailAddress = true );

  protected slots:
    void removeAttendee();
    void slotInsertAttendee( Attendee *a );

  protected:
    void changeStatusForMe( Attendee::PartStat status );

    KCal::Attendee* currentAttendee() const;
    void updateCurrentItem();

  private:
    bool mDisableItemUpdate;

    KListView *mListView;
//     KOEditorFreeBusy *mFreeBusy;
};

#endif
