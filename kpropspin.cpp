// $Id$

#include "kpropspin.h"
#include "kpropspin.moc"
#include <kapp.h>

KPropSpin::KPropSpin( QWidget *parent, const char *text, int perc, const char *key, const char *group, KConfig *config, const char *name )
	: QLabel( text, parent,name )
{
	setKConfig( config );
	setGroup( group );
	setKey( key );

	setFontPropagation( QWidget::SameFont );
	spinBox = new QSpinBox( this );
	percentage = perc;

	sizeUpdate();
}

KPropSpin::~KPropSpin()
{
	delete spinBox;
}

void KPropSpin::sizeUpdate()
{
	QFontMetrics fm( font() );
	int h=fm.height();
	
	setFixedHeight( (h + 8 > 30 ? h + 8 : 30) );
	spinBox->setFixedHeight( height() - 4 );
}

void KPropSpin::resizeEvent( QResizeEvent *rev )
{
	int w = width()*percentage/100;
	spinBox->setGeometry( width() - w - 2, 2, w - 4, 100 );
}

void KPropSpin::fontChange( const QFont & )
{
	sizeUpdate();
}

QSpinBox *KPropSpin::getSpinBox()
{
	return spinBox;
}

const char *KPropSpin::getContents()
{
  return spinBox->text();
}

void KPropSpin::setContents( int value )
{
	spinBox->setValue( value );
}

void KPropSpin::setContents( const char *s )
{
  QString tmpStr(s);
  int index;
  if ((index = tmpStr.find(spinBox->suffix(), 0)) != -1)
    tmpStr.truncate(index);
  spinBox->setValue( tmpStr.toInt() );
}

void KPropSpin::setConfig()
{
  if( ConfigObject )
    {
      ConfigObject->setGroup( Group );
      //		debug("kpropspin: group=%s key=%s",ConfigObject->group(), Key.data() );
      if( Key.data() != 0 )
	ConfigObject->writeEntry( Key.data(), getContents() );
      else debug("kpropspin: Null key not allowed");
    }
}

void KPropSpin::getConfig()
{
  ConfigObject->setGroup( Group );
  QString valStr = ConfigObject->readEntry( Key.data(), "0" );
  setContents( valStr );
  //	debug("kpropspin: reading config %s = %s",Key.data(), s.data() );
}

void KPropSpin::setKey( const char *key )
{
	Key=key;
}

void KPropSpin::setGroup( const char *group )
{
	Group= group;
}

void KPropSpin::setKConfig( KConfig *config )
{
	if( config == 0 )
		ConfigObject = kapp->config();
	else 
		ConfigObject=config;
}

KConfig *KPropSpin::getKConfig()
{
	return ConfigObject;
}

const char *KPropSpin::getKey()
{
	return Key.data();
}

const char *KPropSpin::getGroup()
{
	return Group.data();
}
