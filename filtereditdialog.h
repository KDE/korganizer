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

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef _FILTEREDITDIALOG_H
#define _FILTEREDITDIALOG_H
// $Id$	

#include <qptrlist.h>

#include <kdialogbase.h>

#include <libkcal/calfilter.h>

class QComboBox;
class FilterEdit_base;

using namespace KCal;

/**
  * This is the class to add/edit a calendar filter.
  *
  * @short Creates a dialog box to create/edit a calendar filter
  * @author Cornelius Schumacher
  * @version $Revision$
  */
class FilterEditDialog : public KDialogBase
{
    Q_OBJECT
  public:
    FilterEditDialog(QPtrList<CalFilter> *,QWidget *parent=0, const char *name=0);
    virtual ~FilterEditDialog();

  public slots:
    void updateFilterList();

  signals:
    void filterChanged();

  protected slots:
    void slotDefault();
    void slotApply();
    void slotOk();

    void slotAdd();
    void filterSelected();
    void editCategorySelection();
    void updateCategorySelection(const QStringList &categories);

  protected:
    void readFilter(CalFilter *);
    void writeFilter(CalFilter *);

  private:
    QPtrList<CalFilter> *mFilters;

    QComboBox *mSelectionCombo;
    FilterEdit_base *mEditor;
    
    QStringList mCategories;
};

#endif
