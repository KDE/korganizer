// $Id$

#include <stdio.h>

#include <qtooltip.h>
#include <qframe.h>
#include <qpixmap.h>
#include <qlayout.h>
#include <qwidgetstack.h>

#include <kdatepik.h>
#include <kiconloader.h>
#include <kapp.h>
#include <kdebug.h>
#include <klocale.h>
#include <ktoolbarbutton.h>
#include <kstddirs.h>

#include "categoryselectdialog.h"
#include "koprefs.h"

#include "koincidenceeditor.h"
#include "koincidenceeditor.moc"

KOIncidenceEditor::KOIncidenceEditor(const QString &caption,Calendar *calendar) :
  KDialogBase(Tabbed,caption,Ok|Apply|Cancel|Default|User1,Ok,0,0,
              false,false,i18n("Delete"))
{
  mCalendar = calendar;

  mCategoryDialog = new CategorySelectDialog();

  connect(mCategoryDialog,SIGNAL(editCategories()),SIGNAL(editCategories()));

  // Clicking cancel exits the dialog without saving
  connect(this,SIGNAL(cancelClicked()),SLOT(reject()));
}

KOIncidenceEditor::~KOIncidenceEditor()
{
  delete mCategoryDialog;
}

void KOIncidenceEditor::init()
{
  setupGeneralTab();
  setupDetailsTab();
  setupCustomTabs();
}

void KOIncidenceEditor::setupGeneralTab()
{
  QFrame *topFrame = addPage(i18n("General"));

  QBoxLayout *topLayout = new QVBoxLayout(topFrame);  
  topLayout->setMargin(marginHint());

  topLayout->addWidget(setupGeneralTabWidget(topFrame));
}

void KOIncidenceEditor::setupDetailsTab()
{
  QFrame *topFrame = addPage(i18n("Attendees"));

  QBoxLayout *topLayout = new QVBoxLayout(topFrame);  
  topLayout->setMargin(marginHint());

  mDetails = new KOEditorDetails(spacingHint(),topFrame);
  topLayout->addWidget(mDetails);
}

void KOIncidenceEditor::slotApply()
{
  processInput();
}

void KOIncidenceEditor::slotOk()
{
  if (processInput()) accept();
}

void KOIncidenceEditor::updateCategoryConfig()
{
  mCategoryDialog->updateCategoryConfig();
}
