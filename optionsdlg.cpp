// $Id$

#include <qlayout.h>
#include <qlabel.h>
#include <qgrpbox.h>
#include <qbttngrp.h>
#include <qlined.h>
#include <qbttngrp.h>
#include <qfont.h>
#include <qslider.h>
#include <qfile.h>
#include <qtextstream.h>

#include <kapp.h>
#include <klocale.h>
#include <kglobal.h>
#include <kfontdialog.h>

#include "kproptext.h"
#include "kpropcombo.h"
#include "kpropspin.h"
#include "kpropcheck.h"
#include "kpropradio.h"
#include "kpropfont.h"
#include "kpropcolor.h"
#include "kpropgroup.h"

#include "optionsdlg.h"
#include "optionsdlg.moc"

koConfig koconf;


OptionsDialog::OptionsDialog( const char *title, 
			      QWidget *parent, char *name, bool modal )
  : KPropDlg( KPropDlg::TREE, KPropDlg::OK | 
	      KPropDlg::CANCEL | KPropDlg::APPLY,
	      title, 0, name, modal )
{
  config = kapp->config();
  
  personalFrame = addPage( -1, i18n("Personal"));
  setupMainTab();
  
  timeFrame = addPage( -1, i18n("Time & Date"));
  setupTimeTab();
  
  fontsFrame = addPage( -1, i18n("Fonts"));
  setupFontsTab();
  
  colorsFrame = addPage( -1, i18n("Colors"));
  setupColorsTab();
  
  viewsFrame = addPage( -1, i18n("Views"));
  setupViewsTab();
  
  //	displayFrame = addPage( -1, i18n("Display"));
  //	setupDisplayTab();
  
  printerFrame = addPage( -1, i18n("Printing"));
  setupPrinterTab();
  
  // Layout
//  setMinimumSize(600, 460 );
//  adjustSize();
  
  // read settings
  emit getConfig();
  
  // Setup primary page for display
  showPage(personalFrame);
  
  setCaption(i18n("KOrganizer Configuration"));
}

///////////////////////////////////////////////////////////////////////////////

OptionsDialog::~OptionsDialog()
{
}

///////////////////////////////////////////////////////////////////////////////

void OptionsDialog::setupMainTab()
{
  KPropText *textbox;
  
  const char *holidayList[] = { "(none)",
				"australia", "austria", "bavarian",
				"belgium", "canada", "catalan", "czechia", 
				"denmark", "dutch", "finnish", "french",
				"frswiss", "german", "hungary", "iceland",
				"italy", "japan",
				"norway", "portugal", "poland",
                                "quebec", "romania", "spain",
				"swedish", "thailand", "uk", "us", 0L };
  
  QVBoxLayout *layout = new QVBoxLayout( personalFrame, 4);
  
  textbox = new KPropText( personalFrame, i18n("Your name:"), 70, "user_name", "Personal Settings" );
  connectConfig( textbox );
  layout->addWidget( textbox );
  
  textbox = new KPropText( personalFrame, i18n("Email address:"), 70, "user_email", "Personal Settings" );
  connectConfig( textbox );
  layout->addWidget( textbox );
  
  textbox = new KPropText( personalFrame, i18n("Additional:"), 70, "Additional", "Personal Settings" );
  connectConfig( textbox );
  layout->addWidget( textbox );
  
  KPropCheck *checkbox = new KPropCheck(personalFrame, i18n("Auto-save Calendar"), "Auto Save", "General");
  connectConfig(checkbox);
  layout->addWidget(checkbox);
  
  checkbox = new KPropCheck(personalFrame, i18n("Confirm Appointment/To-Do Deletes"), "Confirm Deletes", "General");
  connectConfig(checkbox);
  layout->addWidget(checkbox);
  
  KPropCombo *combo = new KPropCombo(personalFrame, i18n("Holidays:"),
				     50, "Holidays", 
				     "Personal Settings");
  combo->getCombo()->insertStrList(holidayList);
  connectConfig(combo);
  layout->addWidget(combo);
	
//	layout->addStretch();
}

///////////////////////////////////////////////////////////////////////////////

void OptionsDialog::setupTimeTab()
{
	const char *tzList[] = { "-1200", "-1130", "-1100", "-1030", "-1000",
				 "-0930", "-0900", "-0830", "-0800", "-0730",
				 "-0700", "-0630", "-0600", "-0530", "-0500",
				 "-0430", "-0400", "-0330", "-0300", "-0230",
				 "-0200", "-0130", "-0100", "-0030", "+0000",
				 "+0030", "+0100", "+0130", "+0200", "+0230",
				 "+0300", "+0330", "+0400", "+0430", "+0500",
				 "+0530", "+0600", "+0630", "+0700", "+0730",
				 "+0800", "+0830", "+0900", "+0930", "+1000",
				 "+1030", "+1100", "+1130", "+1200", "+1230",
				 "+1300", "+1330", "+1400", 0L };

	const char *alarmList[] = { i18n("1 minute"), i18n("5 minutes"), 
				    i18n("10 minutes"), i18n("15 minutes"), 
				    i18n("30 minutes"), 0L };

	QVBoxLayout *layout = new QVBoxLayout( timeFrame, 4 );
	//	QHBoxLayout *hlayout = new QHBoxLayout( 4 );

	

	KPropRadio *radio = new KPropRadio( timeFrame, i18n("Time Format:"), "Time Format", "Time & Date" );
	radio->addRadio( i18n("24 hour"), TRUE);
	radio->addRadio( i18n("AM / PM"), FALSE);
	connectConfig( radio );
	layout->addWidget( radio );

	radio = new KPropRadio( timeFrame, i18n("Date Format:"), "Date Format", "Time & Date" );
	QRadioButton *r = radio->addRadio( i18n("Day.Month.Year (31.01.2000)"), FALSE);
	r = radio->addRadio( i18n("Month/Day/Year (01/31/2000)"), FALSE);
	r = radio->addRadio( i18n("Month-Day-Year (01-31-2000)"), FALSE);
	connectConfig( radio );
	layout->addWidget( radio );

	KPropCombo *combo = new KPropCombo( timeFrame, i18n("Time Zone:"), 50, "Time Zone", "Time & Date" );

	combo->getCombo()->insertStrList(tzList);
	connectConfig( combo );
	layout->addWidget( combo );

	KPropSpin *spinProp = new KPropSpin(timeFrame, 
					    i18n("Default Appointment Time:"),
					    25, "Default Start Time",
					    "Time & Date");
	QSpinBox *spinInt = spinProp->getSpinBox();
	spinInt->setSuffix(":00");
	spinInt->setRange(0, 23);

	connectConfig(spinProp);
	layout->addWidget(spinProp);

	combo = new KPropCombo(timeFrame, i18n("Default Alarm Time:"),
			       50, "Default Alarm Time", "Time & Date");
	combo->getCombo()->insertStrList(alarmList);
	connectConfig(combo);
	layout->addWidget(combo);


	KPropCheck *checkBox = new KPropCheck(timeFrame,
					      i18n("Week Starts on Monday"),
					      "Week Starts Monday",
					      "Time & Date");
	connectConfig(checkBox);
	layout->addWidget(checkBox);

//	layout->addStretch();
}

///////////////////////////////////////////////////////////////////////////////

void OptionsDialog::setupViewsTab()
{
  KPropGroup *group;
  QSlider *slider;
  QVBoxLayout *layout = new QVBoxLayout( viewsFrame, 4 );
  
  KPropSpin *spinProp = new KPropSpin(viewsFrame, i18n("Day Begins At:"), 
				     25, "Day Begins", "Views");
  QSpinBox *spinInt = spinProp->getSpinBox();
  spinInt->setSuffix(":00");
  spinInt->setRange(0, 23);

  connectConfig(spinProp);
  layout->addWidget(spinProp);

  layout->addWidget( group = new KPropGroup( viewsFrame, 
					     i18n("Hour size in schedule view")) );
  group->addWidget( slider = new QSlider( 0,100,5,10, QSlider::Horizontal, group ) );
  slider->setFixedHeight( 16 );

  KPropCheck *checkBox = new KPropCheck(viewsFrame, i18n("Show events that recur daily in Date Navigator"), "Show Daily Recurrences", "Views");
  connectConfig(checkBox);
  layout->addWidget(checkBox);
  
//  layout->addStretch(1);
}
///////////////////////////////////////////////////////////////////////////////
void OptionsDialog::setupFontsTab()
{
	QVBoxLayout *layout = new QVBoxLayout( fontsFrame, 4 );

	KPropFont *fontdlg = new KPropFont( fontsFrame, i18n("Appointment"), 
					    i18n("List view font"), 50, 
					    font(), "List Font", "Fonts" );
	connectConfig( fontdlg );
	layout->addWidget( fontdlg );

	fontdlg = new KPropFont( fontsFrame, i18n("Appointment"), 
				 i18n("Schedule view font"), 50, 
				 font(), "Schedule Font", "Fonts" );
	connectConfig( fontdlg );
	layout->addWidget( fontdlg );

	fontdlg = new KPropFont( fontsFrame, i18n("Appointment"), 
				 i18n("Month view font"), 50, 
				 font(), "Month Font", "Fonts" );
	connectConfig( fontdlg );
	layout->addWidget( fontdlg );

	fontdlg = new KPropFont( fontsFrame, i18n("12345"), i18n("Time bar font"),
				 50, font(), "TimeBar Font", "Fonts" );
	connectConfig( fontdlg );
	layout->addWidget( fontdlg );

	fontdlg = new KPropFont( fontsFrame, i18n("Things to do"),
				 i18n("To-Do list font"), 50,
				 font(), "Todo Font", "Fonts" );
	connectConfig( fontdlg );
	layout->addWidget( fontdlg );

//	layout->addStretch();	

}
///////////////////////////////////////////////////////////////////////////////
void OptionsDialog::setupColorsTab()
{
//	KPropColor *color;
	KPropGroup *group;
	
	QVBoxLayout *layout = new QVBoxLayout( colorsFrame, 4 );
/*
	QPushButton *button = new QPushButton( "Set System Default Colors", colorsFrame );
	button->setFixedSize( button->sizeHint() );
	connect( button, SIGNAL( clicked() ), SLOT( setColorDefaults() ) );
	layout->addWidget( button,0,AlignLeft );
*/
	QCheckBox *checkbox = new KPropCheck( colorsFrame, i18n("Use system default colors"), "DefaultColors", "Colors");  
	connectConfig( checkbox );
	connect( checkbox, SIGNAL( toggled(bool) ), SLOT( toggleSystemColors(bool) ) );
	layout->addWidget( checkbox );
	
	layout->addWidget( group = new KPropGroup( colorsFrame, i18n("Appointment & Checklist items")) );
	group->addWidget( color1 = new KPropColor( group, i18n("Background"), 50, 
			koconf.windowColor, "AptBackground", "Colors" ) );
	connectConfig( color1 );
	group->addWidget( color2 = new KPropColor( group, i18n("Text"), 50, 
			koconf.textColor, "AptText", "Colors" ) );
	connectConfig( color2 );
	group->addWidget( color3 = new KPropColor( group, i18n("Handle Selected"), 50, 
						   koconf.activeTitleColor, "AptSelected", "Colors" ) );
	connectConfig( color3 );
	group->addWidget( color4 = new KPropColor( group, i18n("Handle Unselected"), 50, 
			koconf.inactiveTitleColor, "AptUnselected", "Colors" ) );
	connectConfig( color4 );
	group->addWidget( color5 = new KPropColor( group, i18n("Handle Active Apt."), 50, 
			koconf.activeTextColor, "AptActive", "Colors" ) );
	connectConfig( color5 );

	layout->addWidget( group = new KPropGroup( colorsFrame, "Other" ) );
	group->addWidget( color6 = new KPropColor( group, i18n("Sheet background"), 50, 
			koconf.backgroundColor.dark(120), "SheetBackground", "Colors" ) );
	connectConfig( color6 );
	group->addWidget( color7 = new KPropColor( group, i18n("Calendar Background"), 50, 
			koconf.windowColor, "CalBackground", "Colors" ) );
	connectConfig( color7 );
	group->addWidget( color8 = new KPropColor( group, i18n("Calendar Date Text"), 50, 
			koconf.textColor, "CalText", "Colors" ) );
	connectConfig( color8 );
	group->addWidget( color9 = new KPropColor( group, i18n("Calendar Date Selected"), 50, 
			koconf.selectColor, "CalSelected", "Colors" ) );
	connectConfig( color9 );


/*	
	KPropColor *colorProp = new KPropColor( colorsFrame[A, 
						i18n("Appointment"), 50, 
						"List Color", "Colors" );
	connectConfig(colorProp);
	layout->addWidget(colorProp);

	colorProp = new KPropColor(colorsFrame, 
				   i18n("Today"), 50,
				   "Today Color", "Colors");
	connectConfig(colorProp);
	layout->addWidget(colorProp);

	colorProp = new KPropColor(colorsFrame, 
				   i18n("Holidays"), 50, 
				   "Holiday Color", "Colors");
	connectConfig(colorProp);
	layout->addWidget(colorProp);
*/
// 	layout->addStretch();

}
///////////////////////////////////////////////////////////////////////////////
void OptionsDialog::setupDisplayTab()
{
	// I'm working on this section. Expect major revision shortly. (Fester)
}
///////////////////////////////////////////////////////////////////////////////

void OptionsDialog::setupPrinterTab()
{
	
  QVBoxLayout *layout = new QVBoxLayout( printerFrame, 4 );
  
  KPropCombo *combo = new KPropCombo(printerFrame, i18n("Printer Name"),
				     50, "Printer Name",  "Printer");
  
  QString prName;
  QFile printcap("/etc/printcap");

  if ( printcap.open(IO_ReadOnly) ) {
    QTextStream t (&printcap);
    while ( !t.eof() ){
      QString whole_line = t.readLine();
      whole_line = whole_line.stripWhiteSpace();
      if (whole_line.left(1) != "#" &&
	  whole_line.left(1) != ":" &&
	  whole_line != "") { // ignore certain lines
	QString pname(whole_line.mid(whole_line.find("|")+1,
				     whole_line.length()));
	if (pname.find("|") != -1) { // If there is no 2nd bar.XCXC
	  // Try to account for poorly formatted lines
	  prName = pname.left(pname.find("|"));
	} else {
	  prName = pname.left(pname.find(":"));
	}
	combo->getCombo()->insertItem(prName);
      }
    }
    printcap.close();
  }
  connectConfig(combo);
  layout->addWidget(combo);

  KPropRadio *radio = new KPropRadio( printerFrame, i18n("Paper Size"),
				      "Paper Size", "Printer" );
  radio->addRadio( i18n("A4"), FALSE);
  radio->addRadio( i18n("B5"), FALSE);
  radio->addRadio( i18n("Letter"), TRUE);
  radio->addRadio( i18n("Legal"), FALSE);
  radio->addRadio( i18n("Executive"), FALSE);
  connectConfig( radio );
  layout->addWidget( radio );
  
  radio = new KPropRadio(printerFrame, i18n("Paper Orientation"),
			 "Paper Orientation", "Printer");
  QRadioButton *r = radio->addRadio(i18n("Portrait"), FALSE);
  r->setEnabled(FALSE);
  r = radio->addRadio(i18n("Landscape"), TRUE);
  r->setEnabled(FALSE);
  radio->setEnabled(FALSE); // this is OFF for now.
  connectConfig(radio);
  layout->addWidget(radio);
  
  KPropText *prevText = new KPropText(printerFrame, 
				      i18n("Preview Program"),
				      50, "Preview", "Printer");
  connectConfig(prevText);
  if (!strcmp(prevText->text(), ""))
    prevText->setText("gv");
  layout->addWidget(prevText);
  
//  layout->addStretch();
}

///////////////////////////////////////////////////////////////////////////////

void OptionsDialog::setColorDefaults()
{
	KConfig* config = kapp->config();
	config->setGroup( "Colors" );

	color1->setLabelColor( koconf.windowColor);
	color2->setLabelColor( koconf.textColor);
	color3->setLabelColor( koconf.selectColor);
	color4->setLabelColor( koconf.inactiveTextColor);
	color5->setLabelColor( koconf.activeTextColor);
	color6->setLabelColor( koconf.backgroundColor.dark(120));
	color7->setLabelColor( koconf.windowColor);
	color8->setLabelColor( koconf.textColor);
	color9->setLabelColor( koconf.selectColor);
}

void OptionsDialog::applyColorDefaults()
{
	emit setConfig();
}

void OptionsDialog::toggleSystemColors( bool syscol )
{
	if( syscol )
		setColorDefaults();
//	else
//		emit config();
}
