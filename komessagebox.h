/*
    This file is part of KOrganizer.
    Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef KOMESSAGEBOX_H
#define KOMESSAGEBOX_H

#include <kguiitem.h>
#include <kstdguiitem.h>
#include <kmessagebox.h>
#include <qmessagebox.h>
#include <qstring.h>


class KOMessageBox
{
  public:
    static int fourBtnMsgBox( QWidget *parent, QMessageBox::Icon type, 
            const QString &text, const QString &caption = QString::null, 
            const KGuiItem &button1 = KStdGuiItem::yes(), const KGuiItem &button2 = KStdGuiItem::no(), 
            const KGuiItem &button3 = KStdGuiItem::cont(), 
            int options = 0 );
};

#endif
