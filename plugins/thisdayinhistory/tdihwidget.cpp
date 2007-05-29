/*
    This file is part of KOrganizer.
    Copyright (c) 2007 Loïc Corbasson <loic.corbasson@gmail.com>

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
*/

#include "tdihwidget.h"
#include "tdihwidget.moc"
#include <kurllabel.h>
#include <ktoolinvocation.h>

TDIHWidget::TDIHWidget(QWidget* parent)
  : KUrlLabel(parent)
{
  connect( this, SIGNAL(leftClickedUrl(const QString&)),
           this, SLOT(invokeBrowser(const QString&)) );
}

TDIHWidget::~TDIHWidget()
{
}

void TDIHWidget::invokeBrowser(const QString &url) {
  KToolInvocation::invokeBrowser(url);
}
