// 	$Id$	

#include <qlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlistbox.h>

#include <kbuttonbox.h>
#include <klocale.h>

#include "koprefs.h"

#include "catdlg.h"
#include "catdlg.moc"

CategoryDialog::CategoryDialog(QWidget* parent,const char* name)
  : KDialogBase(parent, name, false, i18n("Categories"),
                Ok|Cancel,Ok,false)
{
  QWidget *topWidget = new QWidget(this);
  setMainWidget(topWidget);

  QVBoxLayout *layout = new QVBoxLayout(topWidget);
  layout->setMargin(marginHint());
  layout->setSpacing(spacingHint());
  
  QBoxLayout *subLayout = new QHBoxLayout();
  layout->addLayout(subLayout);

  QBoxLayout *subLayout2 = new QVBoxLayout();
  subLayout->addLayout(subLayout2);

  QLabel *catListLabel = new QLabel(topWidget);
  catListLabel->setText(i18n("Available Categories"));
  subLayout2->addWidget(catListLabel);

  catListBox = new QListBox(topWidget);
  catListBox->setMinimumSize(QSize(75,200));
  catListBox->insertItem(i18n("Appointment"));
  catListBox->insertItem(i18n("Business"));
  catListBox->insertItem(i18n("Meeting"));
  catListBox->insertItem(i18n("Phone Call"));
  catListBox->insertItem(i18n("Education"));
  catListBox->insertItem(i18n("Holiday"));
  catListBox->insertItem(i18n("Vacation"));
  catListBox->insertItem(i18n("Special Occasion"));
  catListBox->insertItem(i18n("Personal"));
  catListBox->insertItem(i18n("Travel"));
  catListBox->insertItem(i18n("Miscellaneous"));
  catListBox->insertItem(i18n("Birthday"));
  catListBox->insertStringList(KOPrefs::instance()->mCustomCategories);
  subLayout2->addWidget(catListBox);

  subLayout2 = new QVBoxLayout();
  subLayout->addLayout(subLayout2);

  subLayout2->addStretch();
  midButtonBox = new KButtonBox(topWidget, KButtonBox::VERTICAL);
  addButton = midButtonBox->addButton(i18n("&Add >>"));
  connect(addButton, SIGNAL(clicked()), SLOT(addCat()));
  removeButton = midButtonBox->addButton(i18n("&Remove"));
  connect(removeButton, SIGNAL(clicked()), SLOT(removeCat()));
  midButtonBox->layout();
  subLayout2->addWidget(midButtonBox);
  subLayout2->addStretch();

  subLayout2 = new QVBoxLayout();
  subLayout->addLayout(subLayout2);

  QLabel *selCatListLabel = new QLabel(topWidget);
  selCatListLabel->setText(i18n("Selected Categories"));
  selCatListLabel->setMinimumSize(selCatListLabel->sizeHint());
  subLayout2->addWidget(selCatListLabel);

  selCatListBox = new QListBox(topWidget);
  selCatListBox->setMinimumSize(QSize(75,200));
  subLayout2->addWidget(selCatListBox);
  
  subLayout = new QHBoxLayout();
  layout->addLayout(subLayout);

  QLabel *catLabel = new QLabel(topWidget);
  catLabel->setText(i18n("New Category:"));
  subLayout->addWidget(catLabel);

  catEdit = new QLineEdit(topWidget);
  subLayout->addWidget(catEdit);
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
  QString catText = catEdit->text();

  if (!catText.isEmpty()) {
    selCatListBox->insertItem(catText);
    if (KOPrefs::instance()->mCustomCategories.find(catText) ==
        KOPrefs::instance()->mCustomCategories.end()) {
      KOPrefs::instance()->mCustomCategories.append(catText);
      catListBox->insertItem(catText);
    }
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
  if (selCatListBox->currentItem() >= 0) {
    selCatListBox->removeItem(selCatListBox->currentItem());
  }
  if (catListBox->currentItem() >= 0) {
    KOPrefs::instance()->mCustomCategories.remove(catListBox->currentText());
    catListBox->removeItem(catListBox->currentItem());
  }
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
  emit categoriesSelected(catStr);

  done(Accepted);
}
