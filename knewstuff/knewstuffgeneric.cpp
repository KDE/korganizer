/*
    This file is part of KDE.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <qfile.h>
#include <qtextstream.h>

#include <kdebug.h>
#include <klocale.h>
#include <kprocess.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>

#include "entry.h"

#include "knewstuffgeneric.h"

using namespace std;

KNewStuffGeneric::KNewStuffGeneric( const QString &type, QWidget *parent )
  : KNewStuff( type, parent )
{
  mConfig = KGlobal::config();
  mConfig->setGroup("KNewStuff");
}

KNewStuffGeneric::~KNewStuffGeneric()
{
  delete mConfig;
}

bool KNewStuffGeneric::install( const QString &fileName )
{
  kdDebug(5850) << "KNewStuffGeneric::install(): " << fileName << endl;
  QStringList list, list2;

  QString cmd = mConfig->readEntry( "InstallationCommand" );
  if ( !cmd.isEmpty() ) {
    kdDebug(5850) << "InstallationCommand: " << cmd << endl;
    list = QStringList::split( " ", cmd );
    for ( QStringList::iterator it = list.begin(); it != list.end(); ++it ) {
        list2 << (*it).replace("%f", fileName);
    }
    KProcess proc;
    proc << list2;
    proc.start( KProcess::Block );
  }

  return true;
}

bool KNewStuffGeneric::createUploadFile( const QString & /*fileName*/ )
{
  return false;
}

QString KNewStuffGeneric::downloadDestination( KNS::Entry *entry )
{
  QString target = entry->fullName();
  QString res = mConfig->readEntry( "StandardResource" );
  if ( res.isEmpty() )
  {
    target = mConfig->readEntry("TargetDir");
    if ( !target.isEmpty())
    {
      res = "data";
      target.append("/" + entry->fullName());
    }
  }
  if ( res.isEmpty() ) return KNewStuff::downloadDestination( entry );

  QString file = locateLocal( res.utf8() , target );
  if ( KStandardDirs::exists( file ) ) {
    int result = KMessageBox::questionYesNo( parentWidget(),
        i18n("The file '%1' already exists. Do you want to override it?")
        .arg( file ),
        QString::null, i18n("Overwrite") );
    if ( result == KMessageBox::No ) return QString::null;
  }

  return file;
}
