/*
    This file is part of libkdepim.

    Copyright (c) 2005 Reinhold Kainhofer <reinhold@kainhofer.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef EMBEDDEDURLPAGE_H
#define EMBEDDEDURLPAGE_H

#include <KUrl>
#include <QWidget>

namespace KParts { class ReadOnlyPart; }

class EmbeddedURLPage : public QWidget
{
    Q_OBJECT
  public:
    EmbeddedURLPage( const QString &url, const QString &mimetype,
                     QWidget *parent);

  public Q_SLOTS:
    void loadContents();
  Q_SIGNALS:
    void openURL( const KUrl &url );
  private:
    void initGUI( const QString &url, const QString &mimetype );

    QString mUri;
    QString mMimeType;
    KParts::ReadOnlyPart* mPart;
};

#endif
