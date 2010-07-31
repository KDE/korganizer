/*
    This file is part of KOrganizer.

    Copyright (c) 2003,2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

    Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    Author: Sergio Martins, <sergio.martins@kdab.com>

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

#ifndef PREVIEWDIALOG_H
#define PREVIEWDIALOG_H

#include <kdialogbase.h>
#include <kurl.h>

class KOListView;

namespace KCal {
  class CalendarLocal;
}

class PreviewDialog : public KDialogBase
{
    Q_OBJECT
  public:
    PreviewDialog( const KURL &url, QWidget *parent );
    ~PreviewDialog();
    bool loadCalendar();

  public slots:
    void slotAdd();
    void slotMerge();

  signals:
    void dialogFinished( PreviewDialog * );
    void openURL( const KURL &, bool );
    void addResource( const KURL & );

  private:
    // Checks if mOriginalUrl is a temp file, if it is we ask the user a place to 
    // keep the calendar file
    bool isTempFile() const;
  private:
    KURL mOriginalUrl;
    KURL *mLocalUrl;
    KOListView *mListView;
    KCal::CalendarLocal *mCalendar;
};

#endif
