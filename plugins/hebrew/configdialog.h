/*
    This file is part of KOrganizer.
    Copyright (c) 2003 Jonathan Singer <jsinger@leeta.net>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <kdialogbase.h>
#include <qcheckbox.h>

/**
@author Jonathan Singer
*/
class ConfigDialog:public KDialogBase
{
Q_OBJECT public:

  ConfigDialog(QWidget * parent = 0);
  virtual ~ ConfigDialog();

protected:
  void load();
  void save();

  protected slots: void slotOk();

private:
   QCheckBox * omer_box;
  QCheckBox *parsha_box;
  QCheckBox *israel_box;
  QCheckBox *chol_box;

};

#endif
