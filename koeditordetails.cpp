// 	$Id$	

#include <qtooltip.h>
#include <qfiledialog.h>
#include <qlayout.h>
#include <qvbox.h>
#include <qbuttongroup.h>
#include <qvgroupbox.h>
#include <qwidgetstack.h>
#include <qdatetime.h>

#include <kapp.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kstddirs.h>
#include <kabapi.h>
#include <kmessagebox.h>

#include "koevent.h"
#include "koprefs.h"

#include "koeditordetails.h"
#include "koeditordetails.moc"


AttendeeListItem::AttendeeListItem(Attendee *a, QListView *parent) :
  QListViewItem(parent)
{
  mAttendee = a;
  updateItem();
}

AttendeeListItem::~AttendeeListItem()
{
  delete mAttendee;
}

void AttendeeListItem::updateItem()
{
  setText(0,mAttendee->getName());
  setText(1,(!mAttendee->getEmail().isEmpty()) ? mAttendee->getEmail() :
                                                 QString::fromLatin1(" "));
  setText(2,mAttendee->getRoleStr());
  setText(3,mAttendee->getStatusStr());
  if (mAttendee->RSVP() && !mAttendee->getEmail().isEmpty())
    setPixmap(4,SmallIcon("mailappt"));
  else
    setPixmap(4,SmallIcon("nomailappt"));
}


KOEditorDetails::KOEditorDetails (int spacing,QWidget* parent,const char* name)
  : QWidget( parent, name)
{
  mSpacing = spacing;

  topLayout = new QGridLayout(this);
  topLayout->setSpacing(mSpacing);

  initAttendee();
// Disabled because the features associated with htis controls aren't
// implemented.
  //initAttach();
  //initMisc();

  checkAttendeeSelection();
}

void KOEditorDetails::initAttendee()
{
  attendeeListBox = new QListView(this,"attendeeListBox");
  attendeeListBox->addColumn(i18n("Name"),180);
  attendeeListBox->addColumn(i18n("Email"),180);
  attendeeListBox->addColumn(i18n("Role"),60);
  attendeeListBox->addColumn(i18n("Status"),100);
  attendeeListBox->addColumn(i18n("RSVP"),35);
  topLayout->addMultiCellWidget(attendeeListBox,0,0,0,5);

  connect(attendeeListBox, SIGNAL(clicked(QListViewItem *)),
	  this, SLOT(attendeeListHilite(QListViewItem *)));
  connect(attendeeListBox, SIGNAL(doubleClicked(QListViewItem *)),
	  this, SLOT(attendeeListAction(QListViewItem *)));
  connect(attendeeListBox,SIGNAL(selectionChanged()),
          SLOT(checkAttendeeSelection()));

  attendeeLabel = new QLabel(this);
  attendeeLabel->setText(i18n("Attendee Name:"));
  topLayout->addWidget(attendeeLabel,1,0);

  attendeeEdit = new QLineEdit(this);
  attendeeEdit->setText("");
  topLayout->addMultiCellWidget(attendeeEdit,1,1,1,4);
  connect(attendeeEdit,SIGNAL(textChanged(const QString &)),
          SLOT(checkLineEdits()));

  QLabel *emailLabel = new QLabel(this);
  emailLabel->setText(i18n("Email Address:"));
  topLayout->addWidget(emailLabel,2,0);
  
  emailEdit = new QLineEdit(this);
  emailEdit->setText("");
  topLayout->addMultiCellWidget(emailEdit,2,2,1,4);
  
  attendeeRoleLabel = new QLabel(this);
  attendeeRoleLabel->setText(i18n("Role:"));
  topLayout->addWidget(attendeeRoleLabel,3,0);

  attendeeRoleCombo = new QComboBox(false,this);
  attendeeRoleCombo->insertItem( i18n("Attendee") );
  attendeeRoleCombo->insertItem( i18n("Organizer") );
  attendeeRoleCombo->insertItem( i18n("Owner") );
  attendeeRoleCombo->insertItem( i18n("Delegate") );
  topLayout->addWidget(attendeeRoleCombo,3,1);

  topLayout->setColStretch(2,1);

  statusLabel = new QLabel(this);
  statusLabel->setText( i18n("Status:") );
  topLayout->addWidget(statusLabel,3,3);

  statusCombo = new QComboBox(false,this);
  statusCombo->insertItem( i18n("Needs Action") );
  statusCombo->insertItem( i18n("Accepted") );
  statusCombo->insertItem( i18n("Sent") );
  statusCombo->insertItem( i18n("Tentative") );
  statusCombo->insertItem( i18n("Confirmed") );
  statusCombo->insertItem( i18n("Declined") );
  statusCombo->insertItem( i18n("Completed") );
  statusCombo->insertItem( i18n("Delegated") );
  topLayout->addWidget(statusCombo,3,4);

  attendeeRSVPButton = new QCheckBox(this);
  attendeeRSVPButton->setText(i18n("Request Response"));
  topLayout->addMultiCellWidget(attendeeRSVPButton,4,4,0,4);

  QWidget *buttonBox = new QWidget(this);
  QVBoxLayout *buttonLayout = new QVBoxLayout(buttonBox);

  topLayout->addMultiCellWidget(buttonBox,1,4,5,5);

  addAttendeeButton = new QPushButton(i18n("&Add"),buttonBox);
  buttonLayout->addWidget(addAttendeeButton);
  addAttendeeButton->setEnabled(false);
  connect(addAttendeeButton,SIGNAL(clicked()),SLOT(addNewAttendee()));

  modifyAttendeeButton = new QPushButton(i18n("&Modify"),buttonBox);
  buttonLayout->addWidget(modifyAttendeeButton);
  connect(modifyAttendeeButton,SIGNAL(clicked()),SLOT(updateAttendee()));

  removeAttendeeButton = new QPushButton(i18n("&Remove"),buttonBox);
  buttonLayout->addWidget(removeAttendeeButton);
  connect(removeAttendeeButton, SIGNAL(clicked()),SLOT(removeAttendee()));

  addressBookButton = new QPushButton(i18n("Address &Book..."),buttonBox);
  buttonLayout->addWidget(addressBookButton);
  connect(addressBookButton,SIGNAL(clicked()),SLOT(openAddressBook()));
}
    
void KOEditorDetails::initAttach()
{
/*
  attachGroupBox = new QGroupBox( this, "User_2" );
  attachGroupBox->setGeometry( 10, 190, 580, 100 );
  attachGroupBox->setMinimumSize( 10, 10 );
  attachGroupBox->setMaximumSize( 32767, 32767 );
  attachGroupBox->setEnabled(false);

  attachFileButton = new QPushButton( this, "PushButton_3" );
  attachFileButton->setGeometry( 20, 200, 100, 20 );
  attachFileButton->setMinimumSize( 10, 10 );
  attachFileButton->setMaximumSize( 32767, 32767 );
  attachFileButton->setText( i18n("Attach...") );
  attachFileButton->setEnabled(false);

  removeFileButton = new QPushButton( this, "PushButton_4" );
  removeFileButton->setGeometry( 20, 260, 100, 20 );
  removeFileButton->setMinimumSize( 10, 10 );
  removeFileButton->setMaximumSize( 32767, 32767 );
  removeFileButton->setText( i18n("Remove") );
  removeFileButton->setEnabled(false);

  attachListBox = new KTabListBox( this, "ListBox_2" );
  attachListBox->setGeometry( 140, 200, 440, 80 );
  attachListBox->setMinimumSize( 10, 10 );
  attachListBox->setMaximumSize( 32767, 32767 );
  attachListBox->setEnabled(false);

  saveFileAsButton = new QPushButton( this, "PushButton_5" );
  saveFileAsButton->setGeometry( 20, 230, 100, 20 );
  saveFileAsButton->setMinimumSize( 10, 10 );
  saveFileAsButton->setMaximumSize( 32767, 32767 );
  saveFileAsButton->setText( i18n("Save As...") );
  saveFileAsButton->setEnabled(false);
*/
}

void KOEditorDetails::initMisc()
{
  /*  locationLabel = new QLabel(groupBox);
  locationLabel->setText( i18n("Location:") );
  locationLabel->setFixedSize(locationLabel->sizeHint());
  layout->addWidget(locationLabel);*/


  /*  subLayout = new QHBoxLayout();
  layout->addLayout(subLayout);

    resourceButton = new QPushButton(groupBox);
  resourceButton->setText( i18n("Resources...") );
  resourceButton->setFixedSize(resourceButton->sizeHint());
  subLayout->addWidget(resourceButton);

  resourcesEdit = new QLineEdit(groupBox);
  resourcesEdit->setText( "" );
  resourcesEdit->setFixedHeight(resourcesEdit->sizeHint().height());
  resourcesEdit->setMinimumWidth(resourcesEdit->sizeHint().width());
  subLayout->addWidget(resourcesEdit);

  subLayout = new QHBoxLayout();
  layout->addLayout(subLayout);
  transparencyLabel = new QLabel( groupBox);
  transparencyLabel->setText( i18n("Transparency:") );
  transparencyLabel->setFixedSize(transparencyLabel->sizeHint());
  subLayout->addWidget(transparencyLabel);

  transparencyAmountLabel = new QLabel(groupBox);
  transparencyAmountLabel->setText( "0" );
  transparencyAmountLabel->setMinimumSize(transparencyAmountLabel->sizeHint());
  subLayout->addWidget(transparencyAmountLabel);*/
}

KOEditorDetails::~KOEditorDetails()
{
}

void KOEditorDetails::setEnabled(bool)
{
// This doesn't correspond to the available widgets at the moment
/*
  attendeeEdit->setEnabled(enabled);
  addAttendeeButton->setEnabled(enabled);
  removeAttendeeButton->setEnabled(enabled);
  attachFileButton->setEnabled(enabled);
  saveFileAsButton->setEnabled(enabled);
  addressBookButton->setEnabled(enabled);
*/
/*
  attendeeRoleCombo->setEnabled(enabled);
  //  attendeeRSVPButton->setEnabled(enabled);
  statusCombo->setEnabled(enabled);
  resourceButton->setEnabled(enabled);
  resourcesEdit->setEnabled(enabled);
*/
}

void KOEditorDetails::removeAttendee()
{
  AttendeeListItem *aItem = (AttendeeListItem *)attendeeListBox->selectedItem();
  if (!aItem) return;

  delete aItem;

  clearAttendeeInput();

  checkAttendeeSelection();
}

void KOEditorDetails::attendeeListHilite(QListViewItem *item)
{
  if (!item) return;

  Attendee *a = ((AttendeeListItem *)item)->attendee(); 

  attendeeEdit->setText(a->getName());
  emailEdit->setText(a->getEmail());
  attendeeRoleCombo->setCurrentItem(a->getRole());
  statusCombo->setCurrentItem(a->getStatus());
  attendeeRSVPButton->setChecked(a->RSVP());
}

void KOEditorDetails::attendeeListAction(QListViewItem *item)
{
  if (!item) return;

  qDebug("KOEditorDetails::attendeeListAction(): to be implemented");

  return;

  /*  switch (col) {
  case 0:
    // do something with the attendee here.
    break;
  case 4:
    if (strcmp(attendeeListBox->text(row, col), "Y") == 0)
      attendeeListBox->changeItemPart("N", row, col);
    else
      attendeeListBox->changeItemPart("Y", row, col);
    break;
    }*/
}

void KOEditorDetails::openAddressBook()
{
  KabAPI addrDialog(this);

  if (addrDialog.init() != AddressBook::NoError) {
    KMessageBox::error(this,i18n("Unable to open address book."));
    return;
  }
  KabKey key;
  AddressBook::Entry entry;
  if (addrDialog.exec()) {
    if (addrDialog.getEntry(entry, key) == AddressBook::NoError) {
      // get name -- combo of first and last names
      QString nameStr;
      addrDialog.addressbook()->literalName(entry, nameStr, true, false);
      attendeeEdit->setText(nameStr);

      // take first email address
      if (!entry.emails.isEmpty() && entry.emails.first().length()>0) {
      	emailEdit->setText(entry.emails.first());
      } else {
        emailEdit->setText("");
      }
    } else {
      KMessageBox::sorry(this,i18n("Error getting entry from address book."));
    }
  }
}


void KOEditorDetails::updateAttendee()
{
  AttendeeListItem *aItem = (AttendeeListItem *)attendeeListBox->selectedItem();
  if (!aItem) return;

  delete aItem;
  addNewAttendee();
  checkAttendeeSelection();
}

void KOEditorDetails::addNewAttendee()
{
  // don;t do anything on a blank name
  if (QString(attendeeEdit->text()).stripWhiteSpace().isEmpty())
    return;

  Attendee *a = new Attendee(attendeeEdit->text());

  // this is cool.  If they didn't enter an email address,
  // try to look it up in the address book and fill it in for them.
  if (QString(emailEdit->text()).stripWhiteSpace().isEmpty()) {
    KabAPI addrBook;
    QString name;
    list<AddressBook::Entry> entries;
    name = attendeeEdit->text();
    if (addrBook.init() == AddressBook::NoError) {
      if (addrBook.getEntryByName(name, entries, 1) == AddressBook::NoError) {
	qDebug("positive match");
	// take first email address
	if (!entries.front().emails.isEmpty() && 
	    entries.front().emails.first().length()>0)
	  emailEdit->setText(entries.front().emails.first());
      }
    }
  }

  a->setEmail(emailEdit->text());
  a->setRole(attendeeRoleCombo->currentItem());
  a->setStatus(statusCombo->currentItem());
  a->setRSVP(attendeeRSVPButton->isChecked());

  insertAttendee(a);

  // zero everything out for a new one
  clearAttendeeInput();

  checkAttendeeSelection();
}

void KOEditorDetails::clearAttendeeInput()
{
  attendeeEdit->setText("");
  emailEdit->setText("");
  attendeeRoleCombo->setCurrentItem(0);
  statusCombo->setCurrentItem(0);
  attendeeRSVPButton->setChecked(true);
}


void KOEditorDetails::insertAttendee(Attendee *a)
{
  (void)new AttendeeListItem(a,attendeeListBox);
}

void KOEditorDetails::setDefaults()
{
  attendeeRSVPButton->setChecked(true);
}

void KOEditorDetails::readEvent(KOEvent *event)
{
  QList<Attendee> tmpAList = event->getAttendeeList();
  Attendee *a;
  for (a = tmpAList.first(); a; a = tmpAList.next())
    insertAttendee(new Attendee(*a));

  //  Details->attachListBox->insertItem(i18n("Not implemented yet."));
}

void KOEditorDetails::writeEvent(KOEvent *event)
{
//  qDebug("KOEditorDetails::writeEvent()");
  event->clearAttendees();
  QListViewItem *item;
  AttendeeListItem *a;
  for (item = attendeeListBox->firstChild(); item;
       item = item->nextSibling()) {
    a = (AttendeeListItem *)item;
//    qDebug("KOEditorDetails::writeEvent add");
//    qDebug("  %s",a->attendee()->getName().latin1());
    event->addAttendee(new Attendee(*(a->attendee())));
  }
}

bool KOEditorDetails::validateInput()
{
  return true;
}

void KOEditorDetails::checkLineEdits()
{
  if (attendeeEdit->text().isEmpty()) {
    addAttendeeButton->setEnabled(false);
  } else {
    addAttendeeButton->setEnabled(true);
  }
}

void KOEditorDetails::checkAttendeeSelection()
{
//  qDebug("KOEditorDetails::checkAttendeeSelection()");

  if (attendeeListBox->selectedItem()) {
    removeAttendeeButton->setEnabled(true);
    modifyAttendeeButton->setEnabled(true);
  } else {
    removeAttendeeButton->setEnabled(false);
    modifyAttendeeButton->setEnabled(false);
  }
}
