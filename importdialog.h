/*
    This file is part of KOrganizer.

    Copyright (c) 2003,2004 Cornelius Schumacher <schumacher@kde.org>

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

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef IMPORTDIALOG_H
#define IMPORTDIALOG_H

#include <kdialogbase.h>

#include <kurl.h>

namespace KCal {
class ResourceRemote;
class ResourceCalendar;
}

class QRadioButton;

using namespace KCal;

class ImportDialog : public KDialogBase
{
    Q_OBJECT
  public:
    ImportDialog( const KURL &url, QWidget *parent );
    ~ImportDialog();

  public slots:
    void slotOk();

  signals:
    void dialogFinished( ImportDialog * );

  protected slots:
    void mergeResource( ResourceCalendar * );

  private:
    KURL mUrl;

    QRadioButton *mAddButton;
    QRadioButton *mMergeButton;
    QRadioButton *mOpenButton;

    ResourceRemote *mMergeResource;
};

#endif
