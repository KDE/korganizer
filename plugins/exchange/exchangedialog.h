/*
    This file is part of KOrganizer.
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>

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
#ifndef EXCHANGEDIALOG_H
#define EXCHANGEDIALOG_H

#include <kdialogbase.h>
#include <kdatewidget.h>

class QComboBox;

class ExchangeDialog : public KDialogBase
{
    Q_OBJECT
  public:
    ExchangeDialog( const QDate &start, const QDate &end, QWidget *parent=0);
    virtual ~ExchangeDialog();

  protected slots:
    void slotOk();

  private:
  public:
    KDateWidget *m_start;
    KDateWidget *m_end;
};

#endif
