// $Id$

#include <qstringlist.h>
#include <qlistbox.h>
#include <qlineedit.h>

#include "koprefs.h"

#include "categoryeditdialog.h"

/* 
 *  Constructs a CategoryEditDialog which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
CategoryEditDialog::CategoryEditDialog( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : CategoryEditDialog_base( parent, name, modal, fl )
{
  QStringList::Iterator it;

  for (it = KOPrefs::instance()->mCustomCategories.begin();
       it != KOPrefs::instance()->mCustomCategories.end(); ++it ) {
    mCategories->insertItem(*it);
  }
  
  connect(mCategories,SIGNAL(highlighted(const QString &)),
          mEdit,SLOT(setText(const QString &)));
}

/*  
 *  Destroys the object and frees any allocated resources
 */
CategoryEditDialog::~CategoryEditDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

void CategoryEditDialog::add()
{
  if (!mEdit->text().isEmpty()) {
    mCategories->insertItem(mEdit->text());
    mEdit->setText("");
  }
}

void CategoryEditDialog::remove()
{
  if (mCategories->currentItem() >= 0) {
    mCategories->removeItem(mCategories->currentItem());
  }
}

void CategoryEditDialog::modify()
{
  if (!mEdit->text().isEmpty()) {
    if (mCategories->currentItem() >= 0) {
      mCategories->changeItem(mEdit->text(),mCategories->currentItem());
      mEdit->setText("");
    }
  }
}

void CategoryEditDialog::slotOk()
{
  slotApply();
  accept();
}

void CategoryEditDialog::slotApply()
{
  KOPrefs::instance()->mCustomCategories.clear();
  
  for (uint i = 0;i<mCategories->count();++i) {
    KOPrefs::instance()->mCustomCategories.append(mCategories->text(i));
  }
  
  emit categoryConfigChanged();
}
