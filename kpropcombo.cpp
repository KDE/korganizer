// $Id$

#include "kpropcombo.h"
#include "kpropcombo.moc"
#include <kapp.h>

KPropCombo::KPropCombo( QWidget *parent, const char *text, int perc, const char *key, const char *group, KConfig *config, const char *name )
	: QLabel( text, parent,name )
{
	setKConfig( config );
	setGroup( group );
	setKey( key );

	setFontPropagation( QWidget::SameFont );
	comboBox = new QComboBox( TRUE, this );
	percentage = perc;

	sizeUpdate();
}

KPropCombo::~KPropCombo()
{
	delete comboBox;
}

void KPropCombo::sizeUpdate()
{
	QFontMetrics fm( font() );
	int h=fm.height();
	
	setFixedHeight( (h + 8 > 30 ? h + 8 : 30) );
	comboBox->setFixedHeight( height() - 4 );
}

void KPropCombo::resizeEvent( QResizeEvent *rev )
{
	int w = width()*percentage/100;
	comboBox->setGeometry( width() - w - 2, 2, w - 4, 100 );
}

void KPropCombo::fontChange( const QFont & )
{
	sizeUpdate();
}

QComboBox *KPropCombo::getCombo()
{
	return comboBox;
}

const char *KPropCombo::getContents()
{
	return comboBox->currentText();
}

void KPropCombo::setContents( const char *s )
{
	int t;
	for( t=0; t< comboBox->count(); t++ )
	{
		if( QString(comboBox->text( t ) ) ==  s )
		{
			comboBox->setCurrentItem( t );
			break;
		}
	}
}

void KPropCombo::setConfig()
{
	if( ConfigObject )
	{
		ConfigObject->setGroup( Group );
		//		debug("kpropcombo: group=%s key=%s",ConfigObject->group(), Key.data() );
		if( Key.data() != 0 )
			ConfigObject->writeEntry( Key.data(), getContents() );
		else debug("kpropcombo: Null key not allowed");
	}
}

void KPropCombo::getConfig()
{
	ConfigObject->setGroup( Group );
	QString s = ConfigObject->readEntry( Key.data() );
	if( s.data() !=0 )
		setContents( s.data() );
	//	debug("kpropcombo: reading config %s = %s",Key.data(), s.data() );

}

void KPropCombo::setKey( const char *key )
{
	Key=key;
}

void KPropCombo::setGroup( const char *group )
{
	Group= group;
}

void KPropCombo::setKConfig( KConfig *config )
{
	if( config == 0 )
		ConfigObject = kapp->config();
	else 
		ConfigObject=config;
}

KConfig *KPropCombo::getKConfig()
{
	return ConfigObject;
}

const char *KPropCombo::getKey()
{
	return Key.data();
}

const char *KPropCombo::getGroup()
{
	return Group.data();
}
