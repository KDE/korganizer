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
#include <kbuttonbox.h>
#include <kabapi.h>

#include "koevent.h"
#include "koprefs.h"

#include "koeditordetails.h"
#include "koeditordetails.moc"


AttendeeListItem::AttendeeListItem(Attendee *a, QListView *parent) :
  QListViewItem(parent)
{
  mAttendee = new Attendee(*a);
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
    setPixmap(4,UserIcon("mailappt"));
  else
    setPixmap(4,UserIcon("nomailappt"));
}


KOEditorDetails::KOEditorDetails (int spacing,QWidget* parent,const char* name)
  : QWidget( parent, name)
{
  mSpacing = spacing;

  topLayout = new QVBoxLayout(this);
  topLayout->setSpacing(mSpacing);

  initAttendee();
  //initAttach();
  initMisc();
}

void KOEditorDetails::initAttendee()
{
  attendeeGroupBox = new QGroupBox(1,Horizontal,i18n("Attendee Information"),
                                   this);
  topLayout->addWidget(attendeeGroupBox);

  attendeeListBox = new QListView( attendeeGroupBox, "attendeeListBox" );
  attendeeListBox->addColumn(i18n("Name"),180);
  attendeeListBox->addColumn(i18n("Email"),180);
  attendeeListBox->addColumn(i18n("Role"),60);
  attendeeListBox->addColumn(i18n("Status"),100);
  attendeeListBox->addColumn(i18n("RSVP"),35);

  connect(attendeeListBox, SIGNAL(clicked(QListViewItem *)),
	  this, SLOT(attendeeListHilite(QListViewItem *)));
  connect(attendeeListBox, SIGNAL(doubleClicked(QListViewItem *)),
	  this, SLOT(attendeeListAction(QListViewItem *)));

  QHBox *nameBox = new QHBox(attendeeGroupBox);

  attendeeLabel = new QLabel(nameBox);
  attendeeLabel->setText(i18n("Attendee Name:"));

  attendeeEdit = new QLineEdit(nameBox);
  attendeeEdit->setText( "" );

  QLabel *emailLabel = new QLabel(nameBox);
  emailLabel->setText(i18n("Email Address:"));
  
  emailEdit = new QLineEdit(nameBox);
  emailEdit->setText("");


  QHBox *roleBox = new QHBox(attendeeGroupBox);
  
  attendeeRoleLabel = new QLabel(roleBox);
  attendeeRoleLabel->setText(i18n("Role:"));
//  attendeeRoleLabel->setAlignment(AlignVCenter|AlignRight);

  attendeeRoleCombo = new QComboBox(false,roleBox);
  attendeeRoleCombo->insertItem( i18n("Attendee") );
  attendeeRoleCombo->insertItem( i18n("Organizer") );
  attendeeRoleCombo->insertItem( i18n("Owner") );
  attendeeRoleCombo->insertItem( i18n("Delegate") );

  statusLabel = new QLabel(roleBox);
  statusLabel->setText( i18n("Status:") );
//  statusLabel->setAlignment(AlignVCenter|AlignRight);

  statusCombo = new QComboBox(false,roleBox);
  statusCombo->insertItem( i18n("Needs Action") );
  statusCombo->insertItem( i18n("Accepted") );
  statusCombo->insertItem( i18n("Sent") );
  statusCombo->insertItem( i18n("Tentative") );
  statusCombo->insertItem( i18n("Confirmed") );
  statusCombo->insertItem( i18n("Declined") );
  statusCombo->insertItem( i18n("Completed") );
  statusCombo->insertItem( i18n("Delegated") );

//  subLayout->addStretch();

  attendeeRSVPButton = new QCheckBox(roleBox);
  attendeeRSVPButton->setText(i18n("Request Response"));

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
//  buttonBox->layout();

//  layout->addWidget(buttonBox);
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
  QGroupBox *groupBox = new QGroupBox(1,Horizontal,this);
  topLayout->addWidget(groupBox);

  QHBox *catBox = new QHBox(groupBox);

  categoriesButton = new QPushButton(catBox);
  categoriesButton->setText(i18n("Categories..."));
  connect(categoriesButton,SIGNAL(clicked()),SIGNAL(openCategoryDialog()));

  categoriesLabel = new QLabel(catBox);
  categoriesLabel->setFrameStyle(QFrame::Panel|QFrame::Sunken);
  categoriesLabel->setText( "" );

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

void KOEditorDetails::setEnabled(bool enabled)
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
  resourceButton->setEnabled(enabled);
  resourcesEdit->setEnabled(enabled);
*/
}

void KOEditorDetails::removeAttendee()
{
  AttendeeListItem *aItem = (AttendeeListItem *)attendeeListBox->currentItem();
  if (!aItem) return;

  delete aItem;
}

void KOEditorDetails::attendeeListHilite(QListViewItem *item)
{
  Attendee *a = ((AttendeeListItem *)item)->attendee(); 

  attendeeEdit->setText(a->getName());
  emailEdit->setText(a->getEmail());
  attendeeRoleCombo->setCurrentItem(a->getRole());
  statusCombo->setCurrentItem(a->getStatus());
  attendeeRSVPButton->setChecked(a->RSVP());
}

void KOEditorDetails::attendeeListAction(QListViewItem *item)
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

void KOEditorDetails::openAddressBook()
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


void KOEditorDetails::updateAttendee()
{
  AttendeeListItem *aItem = (AttendeeListItem *)attendeeListBox->currentItem();
  if (!aItem) return;

  delete aItem;
  addNewAttendee();
}

void KOEditorDetails::addNewAttendee()
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
  a->setRSVP(attendeeRSVPButton->isChecked() ? true : false);

  insertAttendee(a);

  // zero everything out for a new one
  attendeeEdit->setText("");
  emailEdit->setText("");
  attendeeRoleCombo->setCurrentItem(0);
  statusCombo->setCurrentItem(0);
  attendeeRSVPButton->setChecked(true);
}

void KOEditorDetails::insertAttendee(Attendee *a)
{
  mAttendeeList.append(new AttendeeListItem(a,attendeeListBox));
}

void KOEditorDetails::setCategories(QString str)
{
  categoriesLabel->setText(str);
}

void KOEditorDetails::setDefaults()
{
  attendeeRSVPButton->setChecked(true);
}

void KOEditorDetails::readEvent(KOEvent *event)
{
  // attendee information
  // first remove whatever might be here
  mAttendeeList.clear();
  QList<Attendee> tmpAList = event->getAttendeeList();
  Attendee *a;
  for (a = tmpAList.first(); a; a = tmpAList.next())
    insertAttendee(new Attendee (*a));

  //  Details->attachListBox->insertItem(i18n("Not implemented yet."));
  
  // set the status combobox
  statusCombo->setCurrentItem(event->getStatus());

  setCategories(event->getCategoriesStr());
}

void KOEditorDetails::writeEvent(KOEvent *event)
{
  event->clearAttendees();
  unsigned int i;
  for (i = 0; i < mAttendeeList.count(); i++)
    event->addAttendee(new Attendee(*(mAttendeeList.at(i)->attendee())));

  // we should remove this.
  event->setStatus(statusCombo->currentItem());
}
