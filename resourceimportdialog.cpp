/*
    This file is part of KOrganizer.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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

#include "resourceimportdialog.h"

#include "kocore.h"
#include "koprefs.h"

#include <libkcal/calendarresources.h>
#include <libkcal/resourcelocal.h>
#include <libkcal/resourceremote.h>

#include <klocale.h>

#include <qlabel.h>
#include <qlayout.h>

ResourceImportDialog::ResourceImportDialog( const QString &url,
                                            QWidget *parent )
  : KDialogBase( Plain, i18n("Import Calendar"), Ok | Cancel, Ok, parent,
                 0, true, true ),
    mUrl( url )
{
  QFrame *topFrame = plainPage();
  QVBoxLayout *topLayout = new QVBoxLayout( topFrame, 0, spacingHint() );

  QString txt = i18n("Do you want to add the calendar at '%1' to KOrganizer")
                .arg( mUrl );

  topLayout->addWidget( new QLabel( txt, topFrame ) );
}

ResourceImportDialog::~ResourceImportDialog()
{
}

void ResourceImportDialog::slotOk()
{
  kdDebug() << "Adding resource for url '" << mUrl << "'" << endl;

  KCal::CalendarResources *cr = KOCore::self()->calendarResources();
  
  KCal::CalendarResourceManager *manager = cr->resourceManager();

  KCal::ResourceCalendar *resource = 0;

  KURL url( mUrl );
  kdDebug() << "URL: " << url.url() << endl;
  if ( url.isLocalFile() ) {
    kdDebug() << "Local Resource" << endl;
    resource = new KCal::ResourceLocal( mUrl );
    resource->setTimeZoneId( KOPrefs::instance()->mTimeZoneId );
  } else {
    kdDebug() << "Remote Resource" << endl;
//    resource = new KCal::ResourceRemote( url );
  }
  
  if ( resource ) {
    resource->setResourceName( mUrl );
    manager->add( resource );
  }
  
  accept();
}

#include "resourceimportdialog.moc"
