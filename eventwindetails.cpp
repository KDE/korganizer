// 	$Id$	

#include <qmessagebox.h>
#include <qslider.h>
#include <qlined.h>
#include <qlistbox.h>
#include <qpixmap.h>
#include <qlistview.h>

#include <kapp.h>
#include <klocale.h>
#include <kglobal.h>
#include <kbuttonbox.h>
#include <kiconloader.h>
#include <kabapi.h>

#include "eventwindetails.h"
#include "eventwindetails.moc"


AttendeeListItem::AttendeeListItem(Attendee *a, QListView *parent) :
  QListViewItem(parent)
{
  mAttendee = new Attendee (*a);
  updateItem();
}


AttendeeListItem::~AttendeeListItem()
{
  delete mAttendee;
}


void AttendeeListItem::updateItem()
{
  setText(0,mAttendee->getName());
  setText(1,(!mAttendee->getEmail().isEmpty()) ? mAttendee->getEmail().data() :
                                                 " ");
  setText(2,mAttendee->getRoleStr());
  setText(3,mAttendee->getStatusStr());
//  setText(4,(mAttendee->RSVP() && !mAttendee->getEmail().isEmpty()) ?
//            "Y" : "N");
  if (mAttendee->RSVP() && !mAttendee->getEmail().isEmpty())
    setPixmap(4,BarIcon("mailappt"));
  else
    setPixmap(4,BarIcon("nomailappt"));
}


EventWinDetails::EventWinDetails (QWidget* parent, const char* name, bool todo)
  : QFrame( parent, name, 0 )
{
  isTodo = todo;

  topLayout = new QVBoxLayout(this, 5);

  initAttendee();
  //initAttach();
  initMisc();
}

void EventWinDetails::initAttendee()
{
  attendeeGroupBox = new QGroupBox(this);
  attendeeGroupBox->setTitle(i18n("Attendee Information"));
  topLayout->addWidget(attendeeGroupBox);

  QVBoxLayout *layout = new QVBoxLayout(attendeeGroupBox, 10);

  layout->addSpacing(10); // top caption needs some space

  attendeeListBox = new QListView( attendeeGroupBox, "attendeeListBox" );
  attendeeListBox->addColumn(i18n("Name"),180);
  attendeeListBox->addColumn(i18n("Email"),180);
  attendeeListBox->addColumn(i18n("Role"),60);
  attendeeListBox->addColumn(i18n("Status"),100);
  attendeeListBox->addColumn(i18n("RSVP"),35);
  attendeeListBox->setMinimumSize(QSize(400, 100));
  layout->addWidget(attendeeListBox);

  connect(attendeeListBox, SIGNAL(clicked(QListViewItem *)),
	  this, SLOT(attendeeListHilite(QListViewItem *)));
  connect(attendeeListBox, SIGNAL(doubleClicked(QListViewItem *)),
	  this, SLOT(attendeeListAction(QListViewItem *)));

  QHBoxLayout *subLayout = new QHBoxLayout();
  layout->addLayout(subLayout);

  attendeeLabel = new QLabel(attendeeGroupBox);
  attendeeLabel->setText(i18n("Attendee Name:"));
  attendeeLabel->setFixedWidth(attendeeLabel->sizeHint().width());
  attendeeLabel->setMinimumHeight(attendeeLabel->sizeHint().height());
  subLayout->addWidget(attendeeLabel);

  attendeeEdit = new QLineEdit(attendeeGroupBox);
  attendeeEdit->setMinimumWidth(attendeeEdit->sizeHint().width());
  attendeeEdit->setFixedHeight(attendeeEdit->sizeHint().height());
  attendeeEdit->setText( "" );
  subLayout->addWidget(attendeeEdit);

  QLabel *emailLabel = new QLabel(attendeeGroupBox);
  emailLabel->setText(i18n("Email Address:"));
  emailLabel->setFixedWidth(emailLabel->sizeHint().width());
  emailLabel->setMinimumHeight(emailLabel->sizeHint().height());
  subLayout->addWidget(emailLabel);
  
  emailEdit = new QLineEdit(attendeeGroupBox);
  emailEdit->setMinimumWidth(emailEdit->sizeHint().width());
  emailEdit->setFixedHeight(emailEdit->sizeHint().height());
  emailEdit->setText("");
  subLayout->addWidget(emailEdit);

  subLayout = new QHBoxLayout();
  layout->addLayout(subLayout);
  
  attendeeRoleLabel = new QLabel(attendeeGroupBox);
  attendeeRoleLabel->setText(i18n("Role:"));
  attendeeRoleLabel->setMinimumSize(attendeeRoleLabel->sizeHint());
  attendeeRoleLabel->setAlignment(AlignVCenter|AlignRight);
  subLayout->addWidget(attendeeRoleLabel);

  attendeeRoleCombo = new QComboBox(FALSE, attendeeGroupBox);
  attendeeRoleCombo->insertItem( i18n("Attendee") );
  attendeeRoleCombo->insertItem( i18n("Organizer") );
  attendeeRoleCombo->insertItem( i18n("Owner") );
  attendeeRoleCombo->insertItem( i18n("Delegate") );
  attendeeRoleCombo->setFixedSize(attendeeRoleCombo->sizeHint());
  subLayout->addWidget(attendeeRoleCombo);

  statusLabel = new QLabel(attendeeGroupBox);
  statusLabel->setText( i18n("Status:") );
  statusLabel->setMinimumSize(statusLabel->sizeHint());
  statusLabel->setAlignment(AlignVCenter|AlignRight);
  subLayout->addWidget(statusLabel);

  statusCombo = new QComboBox( FALSE, attendeeGroupBox);
  statusCombo->insertItem( i18n("Needs Action") );
  statusCombo->insertItem( i18n("Accepted") );
  statusCombo->insertItem( i18n("Sent") );
  statusCombo->insertItem( i18n("Tentative") );
  statusCombo->insertItem( i18n("Confirmed") );
  statusCombo->insertItem( i18n("Declined") );
  statusCombo->insertItem( i18n("Completed") );
  statusCombo->insertItem( i18n("Delegated") );
  statusCombo->setFixedSize(statusCombo->sizeHint());
  subLayout->addWidget(statusCombo);

  subLayout->addStretch();

  attendeeRSVPButton = new QCheckBox(attendeeGroupBox);
  attendeeRSVPButton->setText(i18n("Request Response"));
  attendeeRSVPButton->setFixedSize(attendeeRSVPButton->sizeHint());
  subLayout->addWidget(attendeeRSVPButton);

  KButtonBox *buttonBox = new KButtonBox(attendeeGroupBox);

  addAttendeeButton = buttonBox->addButton(i18n("&Add"));
  connect(addAttendeeButton, SIGNAL(clicked()),
	  this, SLOT(addNewAttendee()));

  addAttendeeButton = buttonBox->addButton(i18n("&Modify"));
  connect(addAttendeeButton, SIGNAL(clicked()),
	  this, SLOT(updateAttendee()));

  addressBookButton = buttonBox->addButton(i18n("Address &Book..."));
  connect(addressBookButton, SIGNAL(clicked()),
          this, SLOT(openAddressBook()));

  removeAttendeeButton = buttonBox->addButton(i18n("&Remove"));
  connect(removeAttendeeButton, SIGNAL(clicked()),
	  this, SLOT(removeAttendee()));
  buttonBox->layout();

  layout->addWidget(buttonBox);
}
    
void EventWinDetails::initAttach()
{
/*
  attachGroupBox = new QGroupBox( this, "User_2" );
  attachGroupBox->setGeometry( 10, 190, 580, 100 );
  attachGroupBox->setMinimumSize( 10, 10 );
  attachGroupBox->setMaximumSize( 32767, 32767 );
  attachGroupBox->setEnabled(FALSE);

  attachFileButton = new QPushButton( this, "PushButton_3" );
  attachFileButton->setGeometry( 20, 200, 100, 20 );
  attachFileButton->setMinimumSize( 10, 10 );
  attachFileButton->setMaximumSize( 32767, 32767 );
  attachFileButton->setText( i18n("Attach...") );
  attachFileButton->setEnabled(FALSE);

  removeFileButton = new QPushButton( this, "PushButton_4" );
  removeFileButton->setGeometry( 20, 260, 100, 20 );
  removeFileButton->setMinimumSize( 10, 10 );
  removeFileButton->setMaximumSize( 32767, 32767 );
  removeFileButton->setText( i18n("Remove") );
  removeFileButton->setEnabled(FALSE);

  attachListBox = new KTabListBox( this, "ListBox_2" );
  attachListBox->setGeometry( 140, 200, 440, 80 );
  attachListBox->setMinimumSize( 10, 10 );
  attachListBox->setMaximumSize( 32767, 32767 );
  attachListBox->setEnabled(FALSE);

  saveFileAsButton = new QPushButton( this, "PushButton_5" );
  saveFileAsButton->setGeometry( 20, 230, 100, 20 );
  saveFileAsButton->setMinimumSize( 10, 10 );
  saveFileAsButton->setMaximumSize( 32767, 32767 );
  saveFileAsButton->setText( i18n("Save As...") );
  saveFileAsButton->setEnabled(FALSE);
*/
}

void EventWinDetails::initMisc()
{

  QGroupBox *groupBox = new QGroupBox(this);
  topLayout->addWidget(groupBox);

  QVBoxLayout *layout = new QVBoxLayout(groupBox, 10);
  
  QHBoxLayout *subLayout = new QHBoxLayout();
  layout->addLayout(subLayout);

  categoriesButton = new QPushButton(groupBox);
  categoriesButton->setText(i18n("Categories..."));
  categoriesButton->setFixedSize(categoriesButton->sizeHint());
  subLayout->addWidget(categoriesButton);

  categoriesLabel = new QLabel(groupBox);
  categoriesLabel->setFrameStyle(QFrame::Panel|QFrame::Sunken);
  categoriesLabel->setText( "" );
  categoriesLabel->setMinimumSize(categoriesLabel->sizeHint());
  subLayout->addWidget(categoriesLabel);

  /*  locationLabel = new QLabel(groupBox);
  locationLabel->setText( i18n("Location:") );
  locationLabel->setFixedSize(locationLabel->sizeHint());
  layout->addWidget(locationLabel);*/

  subLayout = new QHBoxLayout();
  layout->addLayout(subLayout);

  priorityLabel = new QLabel(groupBox);
  priorityLabel->setText( i18n("Priority:") );
  priorityLabel->setMinimumWidth(priorityLabel->sizeHint().width());
  priorityLabel->setFixedHeight(priorityLabel->sizeHint().height());
  subLayout->addWidget(priorityLabel);

  priorityCombo = new QComboBox( FALSE, groupBox);
  priorityCombo->insertItem( i18n("Low (1)") );
  priorityCombo->insertItem( i18n("Normal (2)") );
  priorityCombo->insertItem( i18n("High (3)") );
  priorityCombo->insertItem( i18n("Maximum (4)") );
  priorityCombo->setFixedSize(priorityCombo->sizeHint());
  connect(priorityCombo, SIGNAL(activated(int)),
    this, SLOT(setModified()));
  subLayout->addWidget(priorityCombo);

  subLayout->addStretch();
  
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


EventWinDetails::~EventWinDetails()
{
}


void EventWinDetails::setEnabled(bool enabled)
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
  categoriesButton->setEnabled(enabled);
  categoriesLabel->setEnabled(enabled);
/*
  attendeeRoleCombo->setEnabled(enabled);
  //  attendeeRSVPButton->setEnabled(enabled);
  statusCombo->setEnabled(enabled);
  priorityCombo->setEnabled(enabled);
  resourceButton->setEnabled(enabled);
  resourcesEdit->setEnabled(enabled);
*/
}

void EventWinDetails::removeAttendee()
{
  AttendeeListItem *aItem = (AttendeeListItem *)attendeeListBox->currentItem();
  if (!aItem) return;

  delete aItem;
}

void EventWinDetails::attendeeListHilite(QListViewItem *item)
{
  Attendee *a = ((AttendeeListItem *)item)->attendee(); 

  attendeeEdit->setText(a->getName());
  emailEdit->setText(a->getEmail());
  attendeeRoleCombo->setCurrentItem(a->getRole());
  statusCombo->setCurrentItem(a->getStatus());
  attendeeRSVPButton->setChecked(a->RSVP());
}

void EventWinDetails::attendeeListAction(QListViewItem *item)
{
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

void EventWinDetails::openAddressBook()
{
  KabAPI addrDialog(this);

  if (addrDialog.init() != AddressBook::NoError) {
    QMessageBox::critical(this, i18n("KOrganizer Error"),
			  i18n("Unable to open address book."));
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
      if (!entry.emails.isEmpty() && entry.emails.first().length()>0)
      	emailEdit->setText(entry.emails.first());
      
    } else {
      QMessageBox::warning(this, i18n("KOrganizer Error"),
			   i18n("Error getting entry from address book."));
    }
  }
}


void EventWinDetails::updateAttendee()
{
  AttendeeListItem *aItem = (AttendeeListItem *)attendeeListBox->currentItem();
  if (!aItem) return;

  delete aItem;
  addNewAttendee();
}

void EventWinDetails::addNewAttendee()
{
  // don;t do anything on a blank name
  if (QString(attendeeEdit->text()).stripWhiteSpace().isEmpty())
    return;

  Attendee *a;
  
  a = new Attendee(attendeeEdit->text());

  // this is cool.  If they didn't enter an email address,
  // try to look it up in the address book and fill it in for them.
  if (QString(emailEdit->text()).stripWhiteSpace().isEmpty()) {
    KabAPI addrBook;
    QString name;
    list<AddressBook::Entry> entries;
    name = attendeeEdit->text();
    if (addrBook.init() == AddressBook::NoError) {
      if (addrBook.getEntryByName(name, entries, 1) == AddressBook::NoError) {
	debug("positive match");
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
  a->setRSVP(attendeeRSVPButton->isChecked() ? TRUE : FALSE);

  insertAttendee(a);
  setModified();

  // zero everything out for a new one
  attendeeEdit->setText("");
  emailEdit->setText("");
  attendeeRoleCombo->setCurrentItem(0);
  statusCombo->setCurrentItem(0);
  attendeeRSVPButton->setChecked(TRUE);
}

void EventWinDetails::insertAttendee(Attendee *a)
{
  mAttendeeList.append(new AttendeeListItem(a,attendeeListBox));
}

void EventWinDetails::setModified()
{
  emit modifiedEvent();
}
