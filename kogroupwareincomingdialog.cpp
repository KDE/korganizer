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
#include "kogroupwareincomingdialog.h"

#include <libkcal/event.h>
#include <qlabel.h>
#include <qtextedit.h>


/*
 *  Constructs a KOGroupwareIncomingDialog which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
KOGroupwareIncomingDialog::KOGroupwareIncomingDialog( KCal::Incidence *evt,
                                                      QWidget *parent,
                                                      const char *name,
                                                      bool modal,
                                                      WFlags fl )
  : KOGroupwareIncomingDialog_base( parent, name, modal, fl )
{
  mAccepted = false;
  mConditionallyAccepted = false;
  mDeclined = false;

  fromLA->setText( evt->dtStartDateStr() + ", " + evt->dtStartTimeStr() );
  if( evt->type() == "Event" ) {
    KCal::Event* event = static_cast<KCal::Event*>(evt);
    toLA->setText( event->dtEndDateStr() + ", " + event->dtEndTimeStr() );
  }
  locationLA->setText( evt->location() );
  summaryLA->setText( evt->summary() );
  descriptionTE->setText( evt->description() );
}

KOGroupwareIncomingDialog::~KOGroupwareIncomingDialog()
{
}

/*
 * protected slot
 */
void KOGroupwareIncomingDialog::slotAcceptEvent()
{
  mAccepted = true;
  mConditionallyAccepted = false;
  mDeclined = false;
}

/*
 * protected slot
 */
void KOGroupwareIncomingDialog::slotAcceptEventConditionally()
{
  mAccepted = false;
  mConditionallyAccepted = true;
  mDeclined = false;
}

/*
 * protected slot
 */
void KOGroupwareIncomingDialog::slotDeclineEvent()
{
  mAccepted = false;
  mConditionallyAccepted = false;
  mDeclined = true;
}

#include "kogroupwareincomingdialog.moc"
