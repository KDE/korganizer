/***************************************************************

    $Id$

     Requires the Qt and KDE widget libraries, available at no cost at
     http://www.troll.no and http://www.kde.org respectively

     Copyright (C) 1997, 1998 Fester Zigterman ( fzr@dds.nl )

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


***************************************************************/

#include <kapp.h>

#include "kproptext.h"
#include "kproptext.moc"

KPropText::KPropText( QWidget *parent, const char *text, int perc, const char *key, const char *group, KConfig *config, const char *name )
	: QLabel( text, parent,name )
{
	setKConfig( config );
	setGroup( group );
	setKey( key );


//	setText( text );
	setFontPropagation( QWidget::SameFont );
	lineEdit = new QLineEdit( this );

	percentage = perc;

	sizeUpdate();
}

KPropText::~KPropText()
{
	delete lineEdit;
}

QLineEdit *KPropText::getLineEdit()
{
	return lineEdit;
}

void KPropText::sizeUpdate()
{
	QFontMetrics fm( font() );
	int h=fm.height();
	
	setFixedHeight( (h + 8 > 30 ? h + 8 : 30) );
	lineEdit->setFixedHeight( height() - 4 );
}

void KPropText::resizeEvent( QResizeEvent *rev )
{
	int w = width()*percentage/100;
	lineEdit->setGeometry( width() - w - 2, 2, w - 4, 100 );
}

void KPropText::fontChange( const QFont & )
{
	sizeUpdate();
}

void KPropText::setContents( const char *text )
{
	lineEdit->setText( text );
}

const char *KPropText::getContents()
{
	return lineEdit->text();
}

void KPropText::setConfig()
{
  //	debug("kproptext::setConfig()");
	if( ConfigObject )
	{
		ConfigObject->setGroup( Group );
		//		debug("kproptext: group=%s key=%s",ConfigObject->group(), Key.data() );
		if( Key.data() != 0 )
			ConfigObject->writeEntry( Key.data(), getContents() );
		else debug("kproptext: Null key not allowed");
	}
}

void KPropText::getConfig()
{
  //	debug("kproptext::config()");
	if( ConfigObject )
	{
		ConfigObject->setGroup( Group );
		//		debug("kproptext: group set.");
		QString s = ConfigObject->readEntry( Key.data() );
		//		debug("kproptext: reading config %s = %s",Key.data(), s.data() );
		setContents( s.data() );
	}
}


void KPropText::setKey( const char *key )
{
	Key=key;
}

void KPropText::setGroup( const char *group )
{
	Group= group;
}

void KPropText::setKConfig( KConfig *config )
{
	if( config == 0 )
		ConfigObject = kapp->config();
	else 
		ConfigObject=config;
}

KConfig *KPropText::getKConfig()
{
	return ConfigObject;
}

const char *KPropText::getKey()
{
	return Key.data();
}

const char *KPropText::getGroup()
{
	return Group.data();
}
