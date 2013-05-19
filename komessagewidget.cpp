/*
  This file is part of KOrganizer.

  Copyright (c) 2013 Sérgio Martins <iamsergio@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "komessagewidget.h"
#include <QApplication>
#include <QMouseEvent>

KOMessageWidget::KOMessageWidget(QWidget *parent) : KMessageWidget(parent)
{
    hide();
    setCloseButtonVisible(false);
    setWordWrap(true);
}

KOMessageWidget::~KOMessageWidget()
{
}

void KOMessageWidget::showEvent(QShowEvent *event)
{
    qApp->installEventFilter(this);
    KMessageWidget::showEvent(event);
}

void KOMessageWidget::hideEvent(QHideEvent *event)
{
    // No need to spend cycles on an event-filter when this is going to
    // me hidden most of the time
    qApp->removeEventFilter(this);
    KMessageWidget::hideEvent(event);
}

bool KOMessageWidget::eventFilter(QObject *, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress)
        hide();

    return false; // we don't want it
}
