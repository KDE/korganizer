#include "kogroupwareincomingdialog.h"

#include <libkcal/event.h>
#include <qlabel.h>
#include <qtextedit.h>


/*
 *  Constructs a KOGroupwareIncomingDialog which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
KOGroupwareIncomingDialog::KOGroupwareIncomingDialog( KCal::Incidence* evt,
						      QWidget* parent,  
						      const char* name, 
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
