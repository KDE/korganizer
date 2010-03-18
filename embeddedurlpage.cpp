/*
    This file is part of libkdepim.

    Copyright (c) 2005 Reinhold Kainhofer <reinhold@kainhofer.com>
    Part of loadContents() copied from the kpartsdesignerplugin:
    Copyright (C) 2005, David Faure <faure@kde.org>

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

#include "embeddedurlpage.h"

#include <KLocale>
#include <KMimeType>
#include <kparts/componentfactory.h>
#include <kparts/browserextension.h>
#include <kparts/part.h>

#include <QLabel>
#include <QLayout>
#include <QVBoxLayout>

EmbeddedURLPage::EmbeddedURLPage( const QString &url, const QString &mimetype,
                                  QWidget *parent )
  : QWidget( parent ), mUri(url), mMimeType( mimetype ), mPart( 0 )
{
  initGUI( url, mimetype );
}

void EmbeddedURLPage::initGUI( const QString &url, const QString &/*mimetype*/ )
{
  QVBoxLayout *layout = new QVBoxLayout( this );
  QLabel* label = new QLabel( i18n("Showing URL %1", url ), this );
  layout->addWidget( label );
}

void EmbeddedURLPage::loadContents()
{
  if ( !mPart ) {
    if ( mMimeType.isEmpty() || mUri.isEmpty() )
        return;
    QString mimetype = mMimeType;
    if ( mimetype == "auto" )
        mimetype == KMimeType::findByUrl( mUri )->name();
    // "this" is both the parent widget and the parent object
    mPart = KParts::ComponentFactory::createPartInstanceFromQuery<KParts::ReadOnlyPart>( mimetype, QString(), this, this );
    if ( mPart ) {
        mPart->openUrl( mUri );
        mPart->widget()->show();
    }
    KParts::BrowserExtension* be = KParts::BrowserExtension::childObject( mPart );
    connect( be, SIGNAL( openUrlRequestDelayed( const KUrl &, const KParts::OpenUrlArguments &, const KParts::BrowserArguments & ) ),
//              mPart, SLOT( openURL( const KUrl & ) ) );
             this, SIGNAL( openURL( const KUrl & ) ) );
  }
}

#include "embeddedurlpage.moc"
