// $Id$

#include "kpropfont.h"
#include "kpropfont.moc"
#include <kapp.h>

KPropFont::KPropFont( QWidget *parent, const char *testtext, const char *buttontext, int perc, QFont mydefault, const char *key, const char *group, KConfig *config, const char *name )
	: QLabel( parent,name )
{
	setKConfig( config );
	setGroup( group );
	setKey( key );
	setFrameStyle( NoFrame );
	setFontPropagation( SameFont );
	defaultFont = mydefault;
		
	label = new QLabel( testtext, this );
	label->setFrameStyle( Panel|Sunken );
	button = new QPushButton( buttontext, this );
	connect( button, SIGNAL( clicked() ), 
			this, SLOT( getFontDialog() ) );
	percentage = perc;

	sizeUpdate();
}

KPropFont::~KPropFont()
{}

void KPropFont::sizeUpdate()
{
	//QFontMetrics fm( font() );
	//int h=fm.height();
	//debug("sizeupdate");

	int h;
	
	h = QMAX( fontMetrics().height() + 2, label->fontMetrics().height() + 2 );

	setFixedHeight( (h > 30 ? h : 30) );
	button->setFixedHeight( height() - 2 );
	label->setFixedHeight( height() );
}

void KPropFont::resizeEvent( QResizeEvent *rev )
{
	int w = width()*percentage/100;
	button->setGeometry( width() - w - 2, 0, w , 100 );
	label->setGeometry( 0,0, w-4, 100 );
}

void KPropFont::fontChange( const QFont & )
{
	sizeUpdate();
}

void KPropFont::setLabelFont( QFont font )
{
	label->setFont( font );
	sizeUpdate();
}

QFont KPropFont::getLabelFont()
{
	return label->font();
}

void KPropFont::setConfig()
{
  //	debug("kpropfont::setConfig()");
	if( ConfigObject )
	{
		ConfigObject->setGroup( Group );
		//		debug("kpropfont: group=%s key=%s",ConfigObject->group(), Key.data() );
		if( Key.data() != 0 )
			ConfigObject->writeEntry( Key.data(), getLabelFont() );
		else debug("kpropfont: Null key not allowed");
	}
}

void KPropFont::getConfig()
{
  //	debug("kpropfont::config()");
	if( ConfigObject )
	{
	  //		debug("Group:");
		ConfigObject->setGroup( Group.data() );
		//		debug("kpropfont: group set.");
		QFont f = ConfigObject->readFontEntry( Key.data(), &defaultFont );
		//debug("kpropfont: reading config %s = %s",Key.data(), s.data() );
		setLabelFont( f );
	}
}

void KPropFont::setKey( const char *key )
{
	if( key ==0 )
		Key = "DefaultKey";
	else
		Key=key;
}

void KPropFont::setGroup( const char *group )
{
	if( group == 0 )
		Group = "DefaultGroup";
	else
		Group= group;
}

void KPropFont::setKConfig( KConfig *config )
{
	if( config == 0 )
		ConfigObject = kapp->config();
	else
		ConfigObject=config;
}

KConfig *KPropFont::getKConfig()
{
	return ConfigObject;
}

const char *KPropFont::getKey()
{
	return Key.data();
}

const char *KPropFont::getGroup()
{
	return Group.data();
}

void KPropFont::getFontDialog()
{
	QFont font( getLabelFont() );
	KFontDialog::getFont( font );
	setLabelFont( font );
}


