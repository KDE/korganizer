/*
    This file is part of KOrganizer.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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

#include "progresswidget.h"

#include <kprogress.h>

#include <qwidgetstack.h>
#include <qlayout.h>
#include <qtimer.h>
#include <qlabel.h>

ProgressWidget::ProgressWidget( QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  QBoxLayout *topLayout = new QHBoxLayout( this );

  mStack = new QWidgetStack( this );
  topLayout->addWidget( mStack );
  mStack->setMaximumHeight( fontMetrics().height() );
  
  mIdle = new QLabel( this );
  mStack->addWidget( mIdle );

  mProgress = new KProgress( this );
  mStack->addWidget( mProgress );

  mIdleTimer = new QTimer( this );
  connect( mIdleTimer, SIGNAL( timeout() ), SLOT( setIdle() ) );

  setIdle();
}

void ProgressWidget::setProgress( int percent )
{
  mStack->raiseWidget( mProgress );

  mProgress->setProgress( percent );

  if ( percent == 100 ) {
    mIdleTimer->start( 1000, true );
  } else {
    mIdleTimer->stop();
  }
}

void ProgressWidget::setIdle()
{
  mStack->raiseWidget( mIdle );
}

#include "progresswidget.moc"
