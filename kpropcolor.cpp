// $Id$

#include "kpropcolor.h"
#include "kpropcolor.moc"
#include <kapp.h>

KPropColor::KPropColor( QWidget *parent, const char *buttontext, int perc, QColor mydefault, const char *key, const char *group, KConfig *config, const char *name )
	: QFrame( parent,name )
{
	setKConfig( config );
	setGroup( group );
	setKey( key );
	setFrameStyle( QFrame::NoFrame );
	setFontPropagation( NoChildren );
	defaultColor = mydefault;

	button = new QPushButton( buttontext, this );
	connect( button, SIGNAL( clicked() ), 
			this, SLOT( getColorDialog() ) );
	percentage = perc;

	sizeUpdate();
}

KPropColor::~KPropColor()
{}

void KPropColor::sizeUpdate()
{
  int h = fontMetrics().height() + 2;
	setFixedHeight( (h > 30 ? h : 30) );
	button->setFixedHeight( height() - 2 );
}

void KPropColor::resizeEvent( QResizeEvent *rev )
{
	int w = width()*percentage/100;
	button->setGeometry( width() - w - 2, 0, w , 100 );
}

void KPropColor::paintEvent( QPaintEvent *pev )
{
	QPainter painter;
	QFrame::paintEvent( pev );

	painter.begin( this );

	painter.drawRect(0,0, width() * percentage / 120, height() );
//			      10, 10);

	painter.setBrush( LabelColor );
	painter.setPen( NoPen );
	
	painter.drawRect( 1,1, width() * percentage / 120 - 2, height() - 2 );
	painter.end();
}

void KPropColor::fontChange( const QFont & )
{
	sizeUpdate();
	getConfig();
}

void KPropColor::setLabelColor( QColor color )
{
	LabelColor = color;
	update();
}

QColor KPropColor::getLabelColor()
{
	return LabelColor;
}

void KPropColor::getColorDialog()
{
	QColor newcol = LabelColor;
	KColorDialog::getColor( newcol );
		LabelColor = newcol;
	update();
}


void KPropColor::setConfig()
{
  //	debug("kpropcolor::setConfig()");
	if( ConfigObject )
	{
		ConfigObject->setGroup( Group );
		//		debug("kpropcolor: group=%s key=%s",ConfigObject->group(), Key.data() );
		if( Key.data() != 0 )
			ConfigObject->writeEntry( Key.data(), getLabelColor() );
		else
			debug("kpropcolor: Null key not allowed");
	}
}

void KPropColor::getConfig()
{
  //	debug("kpropcolor::config()");
	if( ConfigObject )
	{
	  //debug printf("Group:");
		ConfigObject->setGroup( Group.data() );
		//		debug("kpropcolor: group set.");
		QColor c = ConfigObject->readColorEntry( Key.data(), &defaultColor );
		//debug("kpropcolor: reading config %s = %s",Key.data(), s.data() );
		setLabelColor( c );
	}
}

void KPropColor::setKey( const char *key )
{
	if( key ==0 )
		Key = "DefaultKey";
	else
		Key=key;
}

void KPropColor::setGroup( const char *group )
{
	if( group == 0 )
		Group = "DefaultGroup";
	else
		Group= group;
}

void KPropColor::setKConfig( KConfig *config )
{
	if( config == 0 )
		ConfigObject = kapp->config();
	else
		ConfigObject=config;
}

KConfig *KPropColor::getKConfig()
{
	return ConfigObject;
}

const char *KPropColor::getKey()
{
	return Key.data();
}

const char *KPropColor::getGroup()
{
	return Group.data();
}

