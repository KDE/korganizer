// 	$Id$	

#include <stdio.h>

#include <qlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbt.h>
#include <qlist.h>
#include <qstrlist.h>
#include <qlistbox.h>

#include <kapp.h>
#include <kbuttonbox.h>
#include <klocale.h>

#include "catdlg.h"
#include "catdlg.moc"

CategoryDialog::CategoryDialog(QWidget* parent,const char* name)
  : QDialog( parent, name, FALSE, 0 )
{
  setCaption(i18n("KOrganizer Categories"));

  QVBoxLayout *layout = new QVBoxLayout(this, 10);
  
  QBoxLayout *subLayout = new QHBoxLayout();
  layout->addLayout(subLayout);

  QBoxLayout *subLayout2 = new QVBoxLayout();
  subLayout->addLayout(subLayout2);

  QLabel *catListLabel = new QLabel(this);
  catListLabel->setText(i18n("Available Categories"));
  catListLabel->setMinimumSize(catListLabel->sizeHint());
  subLayout2->addWidget(catListLabel);

  catListBox = new QListBox(this);
  catListBox->setMinimumSize(QSize(75,200));
  catList.append(i18n("Appointment"));
  catList.append(i18n("Business"));
  catList.append(i18n("Meeting"));
  catList.append(i18n("Phone Call"));
  catList.append(i18n("Education"));
  catList.append(i18n("Holiday"));
  catList.append(i18n("Vacation"));
  catList.append(i18n("Special Occasion"));
  catList.append(i18n("Personal"));
  catList.append(i18n("Travel"));
  catList.append(i18n("Miscellaneous"));
  catListBox->insertStrList(&catList);
  subLayout2->addWidget(catListBox);

  subLayout2 = new QVBoxLayout();
  subLayout->addLayout(subLayout2);

  subLayout2->addStretch();
  midButtonBox = new KButtonBox(this, KButtonBox::VERTICAL);
  addButton = midButtonBox->addButton(i18n("&Add >>"));
  connect(addButton, SIGNAL(clicked()), SLOT(addCat()));
  removeButton = midButtonBox->addButton(i18n("<< &Remove"));
  connect(removeButton, SIGNAL(clicked()), SLOT(removeCat()));
  midButtonBox->layout();
  subLayout2->addWidget(midButtonBox);

  subLayout2 = new QVBoxLayout();
  subLayout->addLayout(subLayout2);

  QLabel *selCatListLabel = new QLabel(this);
  selCatListLabel->setText(i18n("Selected Categories"));
  selCatListLabel->setMinimumSize(selCatListLabel->sizeHint());
  subLayout2->addWidget(selCatListLabel);

  selCatListBox = new QListBox(this);
  selCatListBox->setMinimumSize(QSize(75,200));
  subLayout2->addWidget(selCatListBox);
  
  subLayout = new QHBoxLayout();
  layout->addLayout(subLayout);

  QLabel *catLabel = new QLabel(this);
  catLabel->setText(i18n("New Category:"));
  catLabel->setFixedSize(catLabel->sizeHint());
  subLayout->addWidget(catLabel);

  catEdit = new QLineEdit( this );
  catEdit->setFixedHeight(catEdit->sizeHint().height());
  subLayout->addWidget(catEdit);

  QFrame *hLine = new QFrame(this);
  hLine->setFrameStyle(QFrame::HLine|QFrame::Sunken);
  hLine->setFixedHeight(hLine->frameWidth());
  layout->addWidget(hLine);

  mainButtonBox = new KButtonBox(this);
  mainButtonBox->addStretch();
  okButton = mainButtonBox->addButton(i18n("&OK"));
  okButton->setDefault(TRUE);
  connect(okButton, SIGNAL(clicked()), SLOT(accept()));
  cancelButton = mainButtonBox->addButton(i18n("&Cancel"));
  connect(cancelButton, SIGNAL(clicked()), SLOT(reject()));
  mainButtonBox->layout();
  layout->addWidget(mainButtonBox);

  layout->activate();

  resize(minimumSize());
}


CategoryDialog::~CategoryDialog()
{
}

void CategoryDialog::setSelected(const QStrList &selList)
{
  selCatListBox->clear(); 
  selCatListBox->insertStrList(&selList); 
}

void CategoryDialog::addCat() 
{
  if (strlen(catEdit->text()) > 0) {
    selCatListBox->insertItem(catEdit->text());
    catEdit->setText("");
  } else {
    if (catListBox->currentItem() >= 0) {
      bool okToAdd = TRUE;
      for (unsigned int i = 0; i < selCatListBox->count(); i++) {
	if (!strcmp(selCatListBox->text(i), 
		    catListBox->text(catListBox->currentItem()))) {
	  okToAdd = FALSE;
	  break;
	}
      }
      if (okToAdd)
	selCatListBox->insertItem(catListBox->text(catListBox->currentItem()));
    }
  }
}

void CategoryDialog::removeCat()
{
  if (selCatListBox->currentItem() >= 0)
    selCatListBox->removeItem(selCatListBox->currentItem());
}

void CategoryDialog::accept()
{
  unsigned int i;
  QString catStr;

  for (i = 0; i < selCatListBox->count(); i++) {
    catStr += selCatListBox->text(i);
    if (i < selCatListBox->count()-1)
      catStr += ", ";
  }
  emit okClicked(catStr);

  done(Accepted);
}
