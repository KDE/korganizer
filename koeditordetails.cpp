// $Id$

#include <qtooltip.h>
#include <qfiledialog.h>
#include <qlayout.h>
#include <qvbox.h>
#include <qbuttongroup.h>
#include <qvgroupbox.h>
#include <qwidgetstack.h>
#include <qdatetime.h>

#include <kapp.h>
#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kstddirs.h>
#include <kabapi.h>
#include <kmessagebox.h>

#include "incidence.h"
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
  setText(0,mAttendee->name());
  setText(1,mAttendee->email());
  setText(2,mAttendee->roleStr());
  setText(3,mAttendee->statusStr());
  if (mAttendee->RSVP() && !mAttendee->email().isEmpty())
    setPixmap(4,SmallIcon("mailappt"));
  else
    setPixmap(4,SmallIcon("nomailappt"));
}


KOEditorDetails::KOEditorDetails (int spacing,QWidget* parent,const char* name)
  : QWidget( parent, name)
{
  QGridLayout *topLayout = new QGridLayout(this);
  topLayout->setSpacing(spacing);

  mListView = new QListView(this,"mListView");
  mListView->addColumn(i18n("Name"),180);
  mListView->addColumn(i18n("Email"),180);
  mListView->addColumn(i18n("Role"),60);
  mListView->addColumn(i18n("Status"),100);
  mListView->addColumn(i18n("RSVP"),35);
  topLayout->addMultiCellWidget(mListView,0,0,0,5);

  connect(mListView, SIGNAL(clicked(QListViewItem *)),
	  this, SLOT(attendeeListHilite(QListViewItem *)));
  connect(mListView, SIGNAL(doubleClicked(QListViewItem *)),
	  this, SLOT(attendeeListAction(QListViewItem *)));
  connect(mListView,SIGNAL(selectionChanged()),
          SLOT(checkAttendeeSelection()));

  QLabel *attendeeLabel = new QLabel(this);
  attendeeLabel->setText(i18n("Attendee Name:"));
  topLayout->addWidget(attendeeLabel,1,0);

  mNameEdit = new QLineEdit(this);
  mNameEdit->setText("");
  topLayout->addMultiCellWidget(mNameEdit,1,1,1,4);
  connect(mNameEdit,SIGNAL(textChanged(const QString &)),
          SLOT(checkLineEdits()));

  QLabel *emailLabel = new QLabel(this);
  emailLabel->setText(i18n("Email Address:"));
  topLayout->addWidget(emailLabel,2,0);

  mEmailEdit = new QLineEdit(this);
  mEmailEdit->setText("");
  topLayout->addMultiCellWidget(mEmailEdit,2,2,1,4);

  QLabel *attendeeRoleLabel = new QLabel(this);
  attendeeRoleLabel->setText(i18n("Role:"));
  topLayout->addWidget(attendeeRoleLabel,3,0);

  mRoleCombo = new QComboBox(false,this);
  mRoleCombo->insertStringList(Attendee::roleList());
  topLayout->addWidget(mRoleCombo,3,1);

  topLayout->setColStretch(2,1);

  QLabel *statusLabel = new QLabel(this);
  statusLabel->setText( i18n("Status:") );
  topLayout->addWidget(statusLabel,3,3);

  mStatusCombo = new QComboBox(false,this);
  mStatusCombo->insertStringList(Attendee::statusList());
  topLayout->addWidget(mStatusCombo,3,4);

  mRsvpButton = new QCheckBox(this);
  mRsvpButton->setText(i18n("Request Response"));
  topLayout->addMultiCellWidget(mRsvpButton,4,4,0,4);

  QWidget *buttonBox = new QWidget(this);
  QVBoxLayout *buttonLayout = new QVBoxLayout(buttonBox);

  topLayout->addMultiCellWidget(buttonBox,1,4,5,5);

  mAddButton = new QPushButton(i18n("&Add"),buttonBox);
  buttonLayout->addWidget(mAddButton);
  mAddButton->setEnabled(false);
  connect(mAddButton,SIGNAL(clicked()),SLOT(addNewAttendee()));

  mModifyButton = new QPushButton(i18n("&Modify"),buttonBox);
  buttonLayout->addWidget(mModifyButton);
  connect(mModifyButton,SIGNAL(clicked()),SLOT(updateAttendee()));

  mRemoveButton = new QPushButton(i18n("&Remove"),buttonBox);
  buttonLayout->addWidget(mRemoveButton);
  connect(mRemoveButton, SIGNAL(clicked()),SLOT(removeAttendee()));

  mAddressBookButton = new QPushButton(i18n("Address &Book..."),buttonBox);
  buttonLayout->addWidget(mAddressBookButton);
  connect(mAddressBookButton,SIGNAL(clicked()),SLOT(openAddressBook()));

  checkAttendeeSelection();
}

KOEditorDetails::~KOEditorDetails()
{
}

void KOEditorDetails::removeAttendee()
{
  AttendeeListItem *aItem = (AttendeeListItem *)mListView->selectedItem();
  if (!aItem) return;

  delete aItem;

  clearAttendeeInput();

  checkAttendeeSelection();
}

void KOEditorDetails::attendeeListHilite(QListViewItem *item)
{
  if (!item) return;

  Attendee *a = ((AttendeeListItem *)item)->attendee();

  mNameEdit->setText(a->name());
  mEmailEdit->setText(a->email());
  mRoleCombo->setCurrentItem(a->role());
  mStatusCombo->setCurrentItem(a->status());
  mRsvpButton->setChecked(a->RSVP());
}

void KOEditorDetails::attendeeListAction(QListViewItem *item)
{
  if (!item) return;

  kdDebug() << "KOEditorDetails::attendeeListAction(): to be implemented" << endl;

  return;

  /*  switch (col) {
  case 0:
    // do something with the attendee here.
    break;
  case 4:
    if (strcmp(mListView->text(row, col), "Y") == 0)
      mListView->changeItemPart("N", row, col);
    else
      mListView->changeItemPart("Y", row, col);
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
      mNameEdit->setText(nameStr);

      // take first email address
      if (!entry.emails.isEmpty() && entry.emails.first().length()>0) {
      	mEmailEdit->setText(entry.emails.first());
      } else {
        mEmailEdit->setText("");
      }
    } else {
      KMessageBox::sorry(this,i18n("Error getting entry from address book."));
    }
  }
}


void KOEditorDetails::updateAttendee()
{
  AttendeeListItem *aItem = (AttendeeListItem *)mListView->selectedItem();
  if (!aItem) return;

  delete aItem;
  addNewAttendee();
  checkAttendeeSelection();
}

void KOEditorDetails::addNewAttendee()
{
  // don;t do anything on a blank name
  if (QString(mNameEdit->text()).stripWhiteSpace().isEmpty())
    return;

  // this is cool.  If they didn't enter an email address,
  // try to look it up in the address book and fill it in for them.
  if (QString(mEmailEdit->text()).stripWhiteSpace().isEmpty()) {
    KabAPI addrBook;
    QString name;
    std::list<AddressBook::Entry> entries;
    name = mNameEdit->text();
    if (addrBook.init() == AddressBook::NoError) {
      if (addrBook.getEntryByName(name, entries, 1) == AddressBook::NoError) {
	kdDebug() << "positive match" << endl;
	// take first email address
	if (!entries.front().emails.isEmpty() &&
	    entries.front().emails.first().length()>0)
	  mEmailEdit->setText(entries.front().emails.first());
      }
    }
  }

  Attendee *a = new Attendee(mNameEdit->text(),mEmailEdit->text(),
                             mRsvpButton->isChecked(),
                             Attendee::PartStat(mStatusCombo->currentItem()),
                             Attendee::Role(mRoleCombo->currentItem()));

  insertAttendee(a);

  // zero everything out for a new one
  clearAttendeeInput();

  checkAttendeeSelection();
}

void KOEditorDetails::clearAttendeeInput()
{
  mNameEdit->setText("");
  mEmailEdit->setText("");
  mRoleCombo->setCurrentItem(0);
  mStatusCombo->setCurrentItem(0);
  mRsvpButton->setChecked(true);
}


void KOEditorDetails::insertAttendee(Attendee *a)
{
  (void)new AttendeeListItem(a,mListView);
}

void KOEditorDetails::setDefaults()
{
  mRsvpButton->setChecked(true);
}

void KOEditorDetails::readEvent(Incidence *event)
{
  QList<Attendee> tmpAList = event->attendees();
  Attendee *a;
  for (a = tmpAList.first(); a; a = tmpAList.next())
    insertAttendee(new Attendee(*a));

  //  Details->attachListBox->insertItem(i18n("Not implemented yet."));
}

void KOEditorDetails::writeEvent(Incidence *event)
{
//  kdDebug() << "KOEditorDetails::writeEvent()" << endl;
  event->clearAttendees();
  QListViewItem *item;
  AttendeeListItem *a;
  for (item = mListView->firstChild(); item;
       item = item->nextSibling()) {
    a = (AttendeeListItem *)item;
//    kdDebug() << "KOEditorDetails::writeEvent add" << endl;
//    kdDebug() << "  " << a->attendee()->getName() << endl;
    event->addAttendee(new Attendee(*(a->attendee())));
  }
}

bool KOEditorDetails::validateInput()
{
  return true;
}

void KOEditorDetails::checkLineEdits()
{
  if (mNameEdit->text().isEmpty()) {
    mAddButton->setEnabled(false);
  } else {
    mAddButton->setEnabled(true);
  }
}

void KOEditorDetails::checkAttendeeSelection()
{
//  kdDebug() << "KOEditorDetails::checkAttendeeSelection()" << endl;

  if (mListView->selectedItem()) {
    mRemoveButton->setEnabled(true);
    mModifyButton->setEnabled(true);
  } else {
    mRemoveButton->setEnabled(false);
    mModifyButton->setEnabled(false);
  }
}
