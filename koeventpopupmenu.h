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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef KOEVENTPOPUPMENU_H
#define KOEVENTPOPUPMENU_H
//
// Context menu for event views with standard event actions
//

#include <qpopupmenu.h>
#include <qdatetime.h>

namespace KCal { 
class Incidence; 
}
using namespace KCal;

class KOEventPopupMenu : public QPopupMenu {
    Q_OBJECT
  public:
    KOEventPopupMenu();
  
    void addAdditionalItem(const QIconSet &icon,const QString &text,
                           const QObject *receiver, const char *member,
                           bool editOnly=false);


  public slots:
    void showIncidencePopup( Incidence *, const QDate & );

  protected slots:
    void popupShow();
    void popupEdit();
    void popupDelete();
    void popupAlarm();
    void dissociateOccurrence();
    void dissociateFutureOccurrence();

  signals:
    void editIncidenceSignal(Incidence *);
    void showIncidenceSignal(Incidence *);
    void deleteIncidenceSignal(Incidence *);
    void toggleAlarmSignal(Incidence *);
    void dissociateOccurrenceSignal( Incidence *, const QDate & );
    void dissociateFutureOccurrenceSignal( Incidence *, const QDate & );
    
  private:
    Incidence *mCurrentIncidence;
    QDate mCurrentDate;
    
    bool mHasAdditionalItems;
    QValueList<int> mEditOnlyItems;
    QValueList<int> mRecurrenceItems;
};

#endif
