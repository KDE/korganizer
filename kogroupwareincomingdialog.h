/*
    This file is part of KOrganizer.

    Copyright (c) 2002 Klarälvdalens Datakonsult AB

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
#ifndef KOGROUPWAREINCOMINGDIALOG_H
#define KOGROUPWAREINCOMINGDIALOG_H

#include "kogroupwareincomingdialog_base.h"

namespace KCal { class Incidence; }


class KOGroupwareIncomingDialog : public KOGroupwareIncomingDialog_base
{
    Q_OBJECT
  public:
    KOGroupwareIncomingDialog( KCal::Incidence*, QWidget* parent = 0,
                               const char* name = 0, bool modal = false,
                               WFlags fl = 0 );
    ~KOGroupwareIncomingDialog();

    bool isAccepted() const { return mAccepted; }
    bool isConditionallyAccepted() const { return mConditionallyAccepted; }
    bool isDeclined() const { return mDeclined; }

  protected slots:
    void slotAcceptEvent();
    void slotAcceptEventConditionally();
    void slotDeclineEvent();

  private:
    bool mAccepted;
    bool mConditionallyAccepted;
    bool mDeclined;
};

#endif
