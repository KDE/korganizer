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

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/
#ifndef KOEDITORDETAILS_H
#define KOEDITORDETAILS_H

#include "customlistviewitem.h"
#include "koattendeeeditor.h"

#include <K3ListView>

namespace KCal {
  class Attendee;
  class Incidence;
}
using namespace KCal;

typedef CustomListViewItem<KCal::Attendee *> AttendeeListItem;

/** KOAttendeeListView is a child class of K3ListView  which supports
 *  dropping of attendees (e.g. from kcontactmanager) onto it. If an attendeee
 *  was dropped, the signal dropped(Attendee*)  is emitted.
 */
class KOAttendeeListView : public K3ListView
{
  Q_OBJECT
  public:
    KOAttendeeListView( QWidget *parent=0 );
    virtual ~KOAttendeeListView();
    virtual void addAttendee( const QString &newAttendee );

#ifndef KORG_NODND
  public slots:
    virtual void contentsDragEnterEvent( QDragEnterEvent *e );
    virtual void dragEnterEvent( QDragEnterEvent *e );
    virtual void contentsDropEvent( QDropEvent *e );
    virtual void dropEvent( QDropEvent *e );
    virtual void contentsDragMoveEvent( QDragMoveEvent *e );

  signals:
    void dropped( Attendee * );
#endif
};

class KOEditorDetails : public KOAttendeeEditor
{
  Q_OBJECT
  public:
    explicit KOEditorDetails( int spacing = 8, QWidget *parent = 0 );
    virtual ~KOEditorDetails();

    /** Set widgets to default values */
    void setDefaults();

    /** Read incidence and setup widgets accordingly */
    void readIncidence( Incidence *incidence );

    /** Write settings to incidence */
    void fillIncidence( Incidence *incidence );

    /** Check if the input is valid. */
    bool validateInput();

    /** Returns whether at least one attendee was added */
    bool hasAttendees();

    void insertAttendee( Attendee *, bool goodEmailAddress=true );

  protected slots:
    void removeAttendee();
    void slotInsertAttendee( Attendee *a );

  protected:
    void changeStatusForMe( Attendee::PartStat status );

    KCal::Attendee *currentAttendee() const;
    /* reimpl */
    Q3ListViewItem *hasExampleAttendee() const;
    void updateCurrentItem();

  private:
    bool mDisableItemUpdate;

    K3ListView *mListView;
};

#endif
