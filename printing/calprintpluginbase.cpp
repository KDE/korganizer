/*
    This file is part of KOrganizer.

    Copyright (c) 1998 Preston Brown <pbrown@kde.org>
    Copyright (c) 2003 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include <qpainter.h>
#include <qlayout.h>
#include <qframe.h>
#include <qlabel.h>
#include <qptrlist.h>
#include <qintdict.h>

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kcalendarsystem.h>
#include <kprinter.h>

#include <libkcal/todo.h>
#include <libkcal/event.h>
#include <libkcal/calendar.h>

#include "calprinthelper.h"
#include "calprintpluginbase.h"

#ifndef KORG_NOPRINTER

QWidget *CalPrintPluginBase::createConfigWidget( QWidget *w )
{
  QFrame *wdg = new QFrame( w );
  QVBoxLayout *layout = new QVBoxLayout( wdg );

  QLabel *title = new QLabel( description(), wdg );
  QFont titleFont( title->font() );
  titleFont.setPointSize( 20 );
  titleFont.setBold( true );
  title->setFont( titleFont );

  layout->addWidget( title );
  layout->addWidget( new QLabel( info(), wdg ) );
  layout->addSpacing( 20 );
  layout->addWidget( new QLabel( i18n("This printing style does not "
                                      "have any configuration options."),
                                 wdg ) );
  layout->addStretch();
  return wdg;
}

void CalPrintPluginBase::doPrint()
{
  QPainter p;

  mPrinter->setColorMode( mUseColors?(KPrinter::Color):(KPrinter::GrayScale) );

  p.begin(mPrinter);
  // the painter initially begins at 72 dpi per the Qt docs.
  // we want half-inch margins.
  p.setViewport( mHelper->mMargin, mHelper->mMargin,
                p.viewport().width() - mHelper->mMargin,
                p.viewport().height() - mHelper->mMargin );
  int pageWidth = p.viewport().width();
  int pageHeight = p.viewport().height();

  print(p, pageWidth, pageHeight);

  p.end();
}

void CalPrintPluginBase::doLoadConfig()
{
  if ( mConfig ) {
    KConfigGroupSaver saver( mConfig, description() );
    mConfig->sync();
    QDateTime currDate( QDate::currentDate() );
    mFromDate = mConfig->readDateTimeEntry( "FromDate", &currDate ).date();
    mToDate = mConfig->readDateTimeEntry( "ToDate" ).date();
    mUseColors = mConfig->readBoolEntry( "UseColors", true );
    mHelper->setUseColors( mUseColors );
    loadConfig();
  } else {
    kdDebug(5850) << "No config available in loadConfig!!!!" << endl;
  }
}

void CalPrintPluginBase::doSaveConfig()
{
  if ( mConfig ) {
    KConfigGroupSaver saver( mConfig, description() );
    saveConfig();
    mConfig->writeEntry( "FromDate", QDateTime( mFromDate ) );
    mConfig->writeEntry( "ToDate", QDateTime( mToDate ) );
    mConfig->writeEntry( "UseColors", mUseColors );
    mConfig->sync();
  } else {
    kdDebug(5850) << "No config available in saveConfig!!!!" << endl;
  }
}

#endif
