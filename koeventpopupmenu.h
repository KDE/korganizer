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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef KOEVENTPOPUPMENU_H
#define KOEVENTPOPUPMENU_H
// $Id$
//
// Context menu for event views with standard event actions
//

#include <qpopupmenu.h>

#include <libkcal/event.h>

using namespace KCal;

class KOEventPopupMenu : public QPopupMenu {
    Q_OBJECT
  public:
    KOEventPopupMenu();
  
    void addAdditionalItem(const QIconSet &icon,const QString &text,
                           const QObject *receiver, const char *member,
                           bool editOnly=false);


  public slots:
    void showEventPopup(Event *);

  protected slots:
    void popupShow();
    void popupEdit();
    void popupDelete();

  signals:
    void editEventSignal(Event *);
    void showEventSignal(Event *);
    void deleteEventSignal(Event *);
    
  private:
    Event *mCurrentEvent;
    
    bool mHasAdditionalItems;
    QValueList<int> mEditOnlyItems;
};

#endif
