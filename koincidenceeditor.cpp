/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// $Id$

#include <qtooltip.h>
#include <qframe.h>
#include <qpixmap.h>
#include <qlayout.h>
#include <qwidgetstack.h>
#include <qdatetime.h>

#include <kdebug.h>
#include <klocale.h>

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
