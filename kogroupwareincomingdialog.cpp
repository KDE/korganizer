/*
    This file is part of KOrganizer.

    Copyright (c) 2002-2004 Klarälvdalens Datakonsult AB
        <info@klaralvdalens-datakonsult.se>

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

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
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
  // FIXME: use a visitor here
  // FIXME: Treat todos and other incidences here
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
