// $Id$

#include <qlayout.h>
#include <qlabel.h>
#include <qgroupbox.h>
#include <qbuttongroup.h>
#include <qlineedit.h>
#include <qfont.h>
#include <qslider.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qcombobox.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qspinbox.h>
#include <qdatetime.h>

#include <kapp.h>
#include <klocale.h>
#include <kglobal.h>
#include <kfontdialog.h>
#include <kstddirs.h>
#include <kmessagebox.h>
#include <kcolordlg.h>
#include <kiconloader.h>
#include <kiconeffect.h>

#include "koprefs.h"

#include "kooptionsdialog.h"
#include "kooptionsdialog.moc"

// Identity icon. This is from KMail.
const char *user_xpm[] = {
"    32    32      159            2",
".. c none",
".# c #020204",
".a c #060204",
".b c #060604",
".c c #0a0a04",
".d c #16160c",
".e c #161614",
".f c #26261c",
".g c #2a2a14",
".h c #363614",
".i c #3a3a24",
".j c #3e3e14",
".k c #3e3e24",
".l c #464214",
".m c #46421c",
".n c #464224",
".o c #464624",
".p c #464634",
".q c #4a4a1c",
".r c #4e4e3c",
".s c #525224",
".t c #565624",
".u c #5a5624",
".v c #5e5a2c",
".w c #5e5e24",
".x c #625e24",
".y c #626224",
".z c #66662c",
".A c #6e6a34",
".B c #6e6e34",
".C c #727234",
".D c #7e7a24",
".E c #827e3c",
".F c #8a862c",
".G c #8a863c",
".H c #8e8a3c",
".I c #8e8a5c",
".J c #928e34",
".K c #969234",
".L c #9a922c",
".M c #9a963c",
".N c #9e9e8c",
".O c #a6a23c",
".P c #aaa234",
".Q c #aaa634",
".R c #aaa63c",
".S c #aaa64c",
".T c #aaaa8c",
".U c #aea64c",
".V c #aeaa34",
".W c #aeaa54",
".X c #b2aa2c",
".Y c #b2aa4c",
".Z c #b2aa54",
".0 c #b2ae34",
".1 c #b2ae3c",
".2 c #b2ae4c",
".3 c #b2ae54",
".4 c #b6ae4c",
".5 c #b6ae54",
".6 c #b6b234",
".7 c #b6b24c",
".8 c #b6b254",
".9 c #bab244",
"#. c #bab24c",
"## c #bab254",
"#a c #bab63c",
"#b c #bab654",
"#c c #beb64c",
"#d c #beb654",
"#e c #beba54",
"#f c #c2ba4c",
"#g c #c2ba54",
"#h c #c2be4c",
"#i c #c2be54",
"#j c #c6be4c",
"#k c #c6be54",
"#l c #c6c254",
"#m c #cac24c",
"#n c #cac254",
"#o c #cac64c",
"#p c #cac654",
"#q c #cec64c",
"#r c #cec654",
"#s c #ceca4c",
"#t c #ceca54",
"#u c #d2ca34",
"#v c #d2ca4c",
"#w c #d2ca54",
"#x c #d2ce4c",
"#y c #d2ce54",
"#z c #d6ce4c",
"#A c #d6ce54",
"#B c #d6d24c",
"#C c #d6d254",
"#D c #dad24c",
"#E c #dad254",
"#F c #dad654",
"#G c #ded654",
"#H c #deda5c",
"#I c #e2d654",
"#J c #e2d65c",
"#K c #e2da4c",
"#L c #e2da54",
"#M c #e2da5c",
"#N c #e2de54",
"#O c #e6de54",
"#P c #e6de5c",
"#Q c #e6e254",
"#R c #e6e25c",
"#S c #e6e264",
"#T c #eae254",
"#U c #eae25c",
"#V c #eae654",
"#W c #eae65c",
"#X c #eae664",
"#Y c #eae66c",
"#Z c #eee654",
"#0 c #eee65c",
"#1 c #eee664",
"#2 c #eee66c",
"#3 c #eeea54",
"#4 c #eeea64",
"#5 c #eeea6c",
"#6 c #f2ea54",
"#7 c #f2ea5c",
"#8 c #f2ea64",
"#9 c #f2ea74",
"a. c #f2ea7c",
"a# c #f2ee5c",
"aa c #f6ee54",
"ab c #f6ee5c",
"ac c #f6ee6c",
"ad c #f6ee7c",
"ae c #faf25c",
"af c #faf264",
"ag c #faf274",
"ah c #faf284",
"ai c #faf28c",
"aj c #faf67c",
"ak c #fafaec",
"al c #fef67c",
"am c #fef694",
"an c #fefa8c",
"ao c #fefa94",
"ap c #fefa9c",
"aq c #fefaa4",
"ar c #fefaac",
"as c #fefab4",
"at c #fefeac",
"au c #fefeb4",
"av c #fefebc",
"aw c #fefec4",
"ax c #fefecc",
"ay c #fefed4",
"az c #fefedc",
"aA c #fefee4",
"aB c #fefeec",
"aC c #fefef4",
"...........................#.#.#.#.#.#.#........................",
".....................d.#.##0#3#W#W#R#U#H.#.#.e..................",
".................#.##T#6a##0#Z#Z#T#Q#O#L#G#F#C.#.#..............",
"...............##Z#7aaa##0#6#0#0#V#Q#Q#M#J#E#A#r#z.b............",
".............##Wabafagajagac#4#U#Q#Q#O#N#L#G#B#y#q#E.#..........",
"...........#abaaajaoatatapah#9#X#Q#O#N#L#G#D#E#B#t#q#j.#........",
".........##8aeajasaxazaxarai#9#X#Q#N#L#I#I#F#D#z#w#o#o#..#......",
".......##TaeajasaAaCaB.N.I#Q#5#U#N#O#O.E.G#D#A#B#x#w#n#f#b.b....",
".......##7afapazaCak.T.r.p.F#P#R#L#G.H.m.o.J#w#y#s#s#o#f#f.#....",
".....##Ta#alawaAaCaC.C.n.i.s#R#P#L#L.w.k.i.t#D#v#w#r#n#k#f#..#..",
".....##6abanawayazax.C.i.f.n#R#O#G#I.w.h.f.o#C#x#v#s#m#k#c.8.#..",
".....#ababaoauavawaw.B.f.#.h#G#N#J#G.v.f.e.j#v#s#t#r#n#f#e.4.#..",
"...##T#6#7ahaqaqamai.z.#.#.l#G#G#G#G.u.#.#.q#w#s#q#m#l#f#f.4.4.#",
"...##0#Z#7#9adada.#2#X.l.m#G#I#G#E#E#z.h.i#z#x#s#p#r#j#i#b.5.Z.#",
"...##3#3#0#1#2#Y#S#P#N#G#G#G#G#D#F#A#z#z#z#x#t#s#m#n#j#f#b.7.Y.#",
"...##T#Z#V#0#R#U#U#R#M#H#G#G#D#E#z#B#A#A#y#t#v#r#p#m#g#f#b.2.W.#",
"...##U#T#Q#W#Q#R#z#K#K#G#F#G#D#E#A#z#x#v#s#t#o#m#c#h#e#c##.Y.W.#",
"...##Q#Q#Q#O#O#K.F#u#I#G#D#D#E#z#z#y#A#t#w#o#r#f.D#a#f#b.4.Y.W.#",
"...c#O#O#N#N#N#K.v#j#G#G#B#B#C#A#v#v#x#q#r#o#o#b.u.1#c#b.4.Z.2.#",
"...##N#O#K#L#L#G.#.V#D#z#E#A#z#x#y#t#t#p#s#n#j.L.#.X#b.4.Z.Y.4.#",
".....##G#L#I#G#D.X.#.V#x#z#A#w#v#x#v#q#r#q#j.Q.#.1#d#b.7.Z.Z.#..",
".....##C#E#D#G#D#o.#.y.0#y#x#x#t#r#r#p#m#j.P.x.##c#b.4.Z.Y.U.#..",
".....##B#C#z#C#z#v.P.#.x#q#w#x#s#s#q#p#m.4.u.#.R#..8.2.W.Z.O.#..",
".......##q#A#z#A#z#v.6.#.#.0#f#h#f#e.9.Y.#.#.K#..8.4.Y.W.S.#....",
".......##s#t#s#t#x#t#q#a.E.#.#.#.#.#.#.#.J.X#..4.Z.Z.Y.S.O.#....",
".........##f#r#q#p#q#o#q#m.O.E.A.A.A.F.M#c##.4.5.2.W.W.U.b......",
"...........##t#j#n#m#m#j#j#j#h#h#g#d#c#b.7.3.2.Y.W.Y.W.#........",
".............##i#c#e#h#i#f#g#c#c#b#b##.7.4.2.Z.Y.W.3.#..........",
"...............#.7.3.4#b#b#b#.#..7.2.Z.3.Z.Y.Z.U.O.b............",
".................#.#.2.W.4.4.5.Z.Z.Z.Y.W.W.Z.W.#.a..............",
".....................#.#.#.Y.W.Y.Y.Y.Y.O.g.#.#..................",
"...........................#.#.#.#.#.a.#........................"
};


KOOptionsDialog::KOOptionsDialog(QWidget *parent, char *name, bool modal) :
  KDialogBase(IconList,i18n("Preferences"),Ok|Apply|Cancel|Default,Ok,parent,
              name,modal,true)
{
  mCategoryDict.setAutoDelete(true);

  setupMainTab();
  setupTimeTab();
  setupFontsTab();
  setupColorsTab();
  setupViewsTab();
  setupPrinterTab();

  readConfig();
  
  QObject::connect(this,SIGNAL(defaultClicked()),SLOT(setDefaults()));
  QObject::connect(this,SIGNAL(cancelClicked()),SLOT(reject()));
}


KOOptionsDialog::~KOOptionsDialog()
{
}


void KOOptionsDialog::setupMainTab()
{
  KIconEffect *iconeffect=KGlobal::iconLoader()->iconEffect();
  QFrame *topFrame = addPage(i18n("Personal"),0,
	iconeffect->apply(QPixmap(user_xpm), KIcon::Desktop, KIcon::DefaultState));

  QGridLayout *topLayout = new QGridLayout(topFrame,6,2);
  topLayout->setSpacing(spacingHint());
  topLayout->setMargin(marginHint());
  
  
  topLayout->addWidget(new QLabel(i18n("Your name:"),topFrame),0,0);
  mNameEdit = new QLineEdit(topFrame);
  topLayout->addWidget(mNameEdit,0,1);

  topLayout->addWidget(new QLabel(i18n("Email address:"),topFrame),1,0);
  mEmailEdit = new QLineEdit(topFrame);
  topLayout->addWidget(mEmailEdit,1,1);
  
  topLayout->addWidget(new QLabel(i18n("Additional:"),topFrame),2,0);
  mAdditionalEdit = new QLineEdit(topFrame);
  topLayout->addWidget(mAdditionalEdit,2,1);
  

  QGroupBox *autoSaveGroup = new QGroupBox(1,Horizontal,i18n("Auto-Save"),
                                           topFrame);
  topLayout->addMultiCellWidget(autoSaveGroup,3,3,0,1);

  mAutoSaveCheck = new QCheckBox(i18n("Enable automatic saving of calendar"),
                                 autoSaveGroup);

  QHBox *intervalBox = new QHBox(autoSaveGroup);
  intervalBox->setSpacing(spacingHint());

  (void)new QLabel(i18n("Save interval in minutes:"),intervalBox);
  mAutoSaveIntervalSpin = new QSpinBox(0,500,1,intervalBox);


  mHolidayList << QString::null;
  QStringList countryList = KGlobal::dirs()->findAllResources("data",
      "korganizer/holiday_*", false, true);
  for ( QStringList::Iterator it = countryList.begin();
        it != countryList.end();
        ++it )
    mHolidayList << (*it).mid((*it).findRev('_') + 1);

  topLayout->addWidget(new QLabel(i18n("Holidays:"),topFrame),4,0);
  mHolidayCombo = new QComboBox(topFrame);
  mHolidayCombo->insertStringList(mHolidayList);

  topLayout->addWidget(mHolidayCombo,4,1);


  mBccCheck = new QCheckBox(i18n("Send copy to owner when mailing events"),
                            topFrame);
  topLayout->addMultiCellWidget(mBccCheck,5,5,0,1);


  mConfirmCheck = new QCheckBox(i18n("Confirm Deletes"),topFrame);
  topLayout->addMultiCellWidget(mConfirmCheck,6,6,0,1);


  topLayout->setRowStretch(7,1);
}


void KOOptionsDialog::setupTimeTab()
{
  QFrame *topFrame = addPage(i18n("Time & Date"),0,
                             DesktopIcon("clock",KIcon::SizeMedium));
  
  QGridLayout *topLayout = new QGridLayout(topFrame,5,2);
  topLayout->setSpacing(spacingHint());
  topLayout->setMargin(marginHint());
  
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

  topLayout->addWidget(new QLabel(i18n("TimeZone:"),topFrame),0,0);
  mTimeZoneCombo = new QComboBox(topFrame);
  mTimeZoneCombo->insertStrList(tzList);
  topLayout->addWidget(mTimeZoneCombo,0,1);

  topLayout->addWidget(new QLabel(i18n("Default Appointment Time:"),
                       topFrame),1,0);
  mStartTimeSpin = new QSpinBox(0,23,1,topFrame);
  mStartTimeSpin->setSuffix(":00");
  topLayout->addWidget(mStartTimeSpin,1,1);

  topLayout->addWidget(new QLabel(i18n("Default duration of new appointment:"),
                       topFrame),2,0);
  mDefaultDurationSpin = new QSpinBox(0,23,1,topFrame);
  mDefaultDurationSpin->setSuffix(":00");
  topLayout->addWidget(mDefaultDurationSpin,2,1);

  QStringList alarmList;  
  alarmList << i18n("1 minute") << i18n("5 minutes") << i18n("10 minutes")
            << i18n("15 minutes") << i18n("30 minutes");
  topLayout->addWidget(new QLabel(i18n("Default Alarm Time:"),topFrame),
                       3,0);
  mAlarmTimeCombo = new QComboBox(topFrame);
  mAlarmTimeCombo->insertStringList(alarmList);
  topLayout->addWidget(mAlarmTimeCombo,3,1);
  
  topLayout->setRowStretch(4,1);
}


void KOOptionsDialog::setupViewsTab()
{
  QFrame *topFrame = addPage(i18n("Views"),0,
                             DesktopIcon("viewmag",KIcon::SizeMedium));
  
  QGridLayout *topLayout = new QGridLayout(topFrame,5,2);
  topLayout->setSpacing(spacingHint());
  topLayout->setMargin(marginHint());
  
  topLayout->addWidget(new QLabel(i18n("Day begins at:"),
                       topFrame),0,0);
  mDayBeginsSpin = new QSpinBox(0,23,1,topFrame);
  mDayBeginsSpin->setSuffix(":00");
  topLayout->addWidget(mDayBeginsSpin,0,1);

  QGroupBox *hourSizeGroup = new QGroupBox(1,Horizontal,
                                           i18n("Hour size in schedule view"),
                                           topFrame);
  mHourSizeSlider = new QSlider(0,100,5,10,Horizontal,hourSizeGroup);
  topLayout->addMultiCellWidget(hourSizeGroup,1,1,0,1);
// Disable hour size setting because it is not used. Has to be fixed.
  hourSizeGroup->hide();
    
  mDailyRecurCheck =
      new QCheckBox(i18n("Show events that recur daily in Date Navigator"),
                    topFrame);
  topLayout->addWidget(mDailyRecurCheck,2,0);
  
  mWeeklyRecurCheck =
      new QCheckBox(i18n("Show events that recur weekly in Date Navigator"),
                    topFrame);
  topLayout->addWidget(mWeeklyRecurCheck,3,0);
  
  mEnableToolTipsCheck =
      new QCheckBox(i18n("Enable ToolTips displaying summary of events"),
                    topFrame);
  topLayout->addWidget(mEnableToolTipsCheck,4,0);
  
  topLayout->setRowStretch(5,1);
}


void KOOptionsDialog::setupFontsTab()
{
  QFrame *topFrame = addPage(i18n("Fonts"),0,
                             DesktopIcon("fonts",KIcon::SizeMedium));

  QGridLayout *topLayout = new QGridLayout(topFrame,5,2);
  topLayout->setSpacing(spacingHint());
  topLayout->setMargin(marginHint());

  mTimeBarFont = new QLabel(KGlobal::locale()->formatTime(QTime(12,34)),
                            topFrame);
  mTimeBarFont->setFrameStyle(QFrame::Panel|QFrame::Sunken);
  topLayout->addWidget(mTimeBarFont,0,0);

  QPushButton *buttonTimeBar = new QPushButton(i18n("Time Bar"),topFrame);
  topLayout->addWidget(buttonTimeBar,0,1);
  connect(buttonTimeBar,SIGNAL(clicked()),SLOT(selectTimeBarFont()));

  mMonthViewFont = new QLabel(KGlobal::locale()->formatTime(QTime(12,34)) +
                              " " + i18n("Event Text"),topFrame);
  mMonthViewFont->setFrameStyle(QFrame::Panel|QFrame::Sunken);
  topLayout->addWidget(mMonthViewFont,1,0);

  QPushButton *buttonMonthView = new QPushButton(i18n("Month View"),topFrame);
  topLayout->addWidget(buttonMonthView,1,1);
  connect(buttonMonthView,SIGNAL(clicked()),SLOT(selectMonthViewFont()));

  mAgendaViewFont = new QLabel(i18n("Event Text"),topFrame);
  mAgendaViewFont->setFrameStyle(QFrame::Panel|QFrame::Sunken);
  topLayout->addWidget(mAgendaViewFont,2,0);

  QPushButton *buttonAgendaView = new QPushButton(i18n("Agenda View"),topFrame);
  topLayout->addWidget(buttonAgendaView,2,1);
  connect(buttonAgendaView,SIGNAL(clicked()),SLOT(selectAgendaViewFont()));

  topLayout->setRowStretch(3,1);
}

void KOOptionsDialog::selectTimeBarFont()
{
  QFont theFont(mTimeBarFont->font());
  QString theText(mTimeBarFont->text());
  KFontDialog::getFontAndText(theFont,theText);
  mTimeBarFont->setFont(theFont);
}

void KOOptionsDialog::selectMonthViewFont()
{
  QFont theFont(mMonthViewFont->font());
  QString theText(mMonthViewFont->text());
  KFontDialog::getFontAndText(theFont,theText);
  mMonthViewFont->setFont(theFont);
}

void KOOptionsDialog::selectAgendaViewFont()
{
  QFont theFont(mAgendaViewFont->font());
  QString theText(mAgendaViewFont->text());
  KFontDialog::getFontAndText(theFont,theText);
  mAgendaViewFont->setFont(theFont);
}


void KOOptionsDialog::setupColorsTab()
{
  QFrame *topFrame = addPage(i18n("Colors"),0,
                             DesktopIcon("colors",KIcon::SizeMedium));

  QGridLayout *topLayout = new QGridLayout(topFrame,5,2);
  topLayout->setSpacing(spacingHint());
  topLayout->setMargin(marginHint());

  // Holiday Color
  mHolidayColor = new QFrame(topFrame);
  mHolidayColor->setFrameStyle(QFrame::Panel|QFrame::Plain);
  topLayout->addWidget(mHolidayColor,0,0);

  QPushButton *buttonHolidayColor = new QPushButton(i18n("Holiday Color"),
                                                    topFrame);
  topLayout->addWidget(buttonHolidayColor,0,1);
  connect(buttonHolidayColor,SIGNAL(clicked()),SLOT(selectHolidayColor()));

  // Highlight Color
  mHighlightColor = new QFrame(topFrame);
  mHighlightColor->setFrameStyle(QFrame::Panel|QFrame::Plain);
  topLayout->addWidget(mHighlightColor,1,0);

  QPushButton *buttonHighlightColor = new QPushButton(i18n("Highlight Color"),
                                                      topFrame);
  topLayout->addWidget(buttonHighlightColor,1,1);
  connect(buttonHighlightColor,SIGNAL(clicked()),SLOT(selectHighlightColor()));

  // Event color
  mEventColor = new QFrame(topFrame);
  mEventColor->setFrameStyle(QFrame::Panel|QFrame::Plain);
  topLayout->addWidget(mEventColor,2,0);

  QPushButton *buttonEventColor = new QPushButton(i18n("Default Event Color"),
                                                      topFrame);
  topLayout->addWidget(buttonEventColor,2,1);
  connect(buttonEventColor,SIGNAL(clicked()),SLOT(selectEventColor()));

  // agenda view background color
  mAgendaBgColor = new QFrame(topFrame);
  mAgendaBgColor->setFrameStyle(QFrame::Panel|QFrame::Plain);
  topLayout->addWidget(mAgendaBgColor,3,0);

  QPushButton *buttonAgendaBgColor =
      new QPushButton(i18n("Agenda View Background Color"),topFrame);
  topLayout->addWidget(buttonAgendaBgColor,3,1);
  connect(buttonAgendaBgColor,SIGNAL(clicked()),SLOT(selectAgendaBgColor()));

  // categories colors
  QGroupBox *categoryGroup = new QGroupBox(1,Horizontal,i18n("Categories"),
                                           topFrame);
  topLayout->addMultiCellWidget(categoryGroup,4,4,0,1);

  mCategoryCombo = new QComboBox(categoryGroup);
  mCategoryCombo->insertStringList(KOPrefs::instance()->mCustomCategories);
  connect(mCategoryCombo,SIGNAL(activated(int)),SLOT(updateCategoryColor()));

  QHBox *categoryBox = new QHBox(categoryGroup);
  categoryBox->setSpacing(spacingHint());

  mCategoryColor = new QFrame(categoryBox);
  mCategoryColor->setFrameStyle(QFrame::Panel|QFrame::Plain);
  
  QPushButton *categoryButton = new QPushButton(i18n("Select Color"),
                                                categoryBox);
  connect(categoryButton,SIGNAL(clicked()),SLOT(selectCategoryColor()));
  updateCategoryColor();
  
  topLayout->setRowStretch(5,1);
}

void KOOptionsDialog::selectHolidayColor()
{
  QColor myColor;
  int result = KColorDialog::getColor( myColor );
  if ( result == KColorDialog::Accepted ) {
    mHolidayColor->setBackgroundColor(myColor);
  }
}

void KOOptionsDialog::selectHighlightColor()
{
  QColor myColor;
  int result = KColorDialog::getColor( myColor );
  if ( result == KColorDialog::Accepted ) {
    mHighlightColor->setBackgroundColor(myColor);
  }
}

void KOOptionsDialog::selectEventColor()
{
  QColor myColor;
  int result = KColorDialog::getColor( myColor );
  if ( result == KColorDialog::Accepted ) {
    mEventColor->setBackgroundColor(myColor);
  }
}

void KOOptionsDialog::selectAgendaBgColor()
{
  QColor myColor;
  int result = KColorDialog::getColor( myColor );
  if ( result == KColorDialog::Accepted ) {
    mAgendaBgColor->setBackgroundColor(myColor);
  }
}

void KOOptionsDialog::selectCategoryColor()
{
  QColor myColor;
  int result = KColorDialog::getColor( myColor );
  if ( result == KColorDialog::Accepted ) {
    mCategoryColor->setBackgroundColor(myColor);
    mCategoryDict.insert(mCategoryCombo->currentText(),new QColor(myColor));
  }
}

void KOOptionsDialog::updateCategoryColor()
{
  QString cat = mCategoryCombo->currentText();
  QColor *color = mCategoryDict.find(cat);
  if (!color) {
    color = KOPrefs::instance()->categoryColor(cat);
  }
  if (color) {
    mCategoryColor->setBackgroundColor(*color);
  }
}

void KOOptionsDialog::setupPrinterTab()
{
  mPrinterTab = addPage(i18n("Printing"),0,
                             DesktopIcon("fileprint",KIcon::SizeMedium));
  
  QGridLayout *topLayout = new QGridLayout(mPrinterTab,5,2);
  topLayout->setSpacing(spacingHint());
  topLayout->setMargin(marginHint());

  topLayout->addWidget(new QLabel(i18n("Printer Name:"),mPrinterTab),0,0);
  mPrinterCombo = new QComboBox(mPrinterTab);
  topLayout->addWidget(mPrinterCombo,0,1);

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
	mPrinterCombo->insertItem(prName);
      }
    }
    printcap.close();
  }

  mPaperSizeGroup = new QButtonGroup(1,Horizontal,i18n("Paper Size:"),
                                     mPrinterTab);
  (void)new QRadioButton(i18n("A4"),mPaperSizeGroup);
  (void)new QRadioButton(i18n("B5"),mPaperSizeGroup);
  (void)new QRadioButton(i18n("Letter"),mPaperSizeGroup);
  (void)new QRadioButton(i18n("Legal"),mPaperSizeGroup);
  (void)new QRadioButton(i18n("Executive"),mPaperSizeGroup);
  topLayout->addMultiCellWidget(mPaperSizeGroup,1,1,0,1);

  mPaperOrientationGroup = new QButtonGroup(1,Horizontal,
                                            i18n("Paper Orientation:"),
                                            mPrinterTab);
  (void)new QRadioButton(i18n("Portrait"),mPaperOrientationGroup);
  (void)new QRadioButton(i18n("Landscape"),mPaperOrientationGroup);
  topLayout->addMultiCellWidget(mPaperOrientationGroup,2,2,0,1);

  topLayout->addWidget(new QLabel(i18n("Preview Program:"),mPrinterTab),3,0);
  mPrintPreviewEdit = new QLineEdit(mPrinterTab);
  topLayout->addWidget(mPrintPreviewEdit,3,1);

  // Add some pixels spacing to avoid scrollbars in icon field. Not safe, but
  // works for me :-)
  topLayout->addRowSpacing(4,27);

  topLayout->setRowStretch(4,1);
}


void KOOptionsDialog::showPrinterTab()
{
  showPage(pageIndex(mPrinterTab));
}


void KOOptionsDialog::setDefaults()
{
  KOPrefs::instance()->setDefaults();
  readConfig();
}


void KOOptionsDialog::setCombo(QComboBox *combo, const QString & text,
                               const QStringList *tags)
{
  if (tags) {
    int i = tags->findIndex(text);
    combo->setCurrentItem(i);
  } else {
    for(int i=0;i<combo->count();++i) {
      if (combo->text(i) == text) {
        combo->setCurrentItem(i);
        break;
      }
    }
  }
}

void KOOptionsDialog::readConfig()
{
  mNameEdit->setText(KOPrefs::instance()->mName);
  mEmailEdit->setText(KOPrefs::instance()->mEmail);
  mAdditionalEdit->setText(KOPrefs::instance()->mAdditional);
  mBccCheck->setChecked(KOPrefs::instance()->mBcc);

  mAutoSaveCheck->setChecked(KOPrefs::instance()->mAutoSave);
  mAutoSaveIntervalSpin->setValue(KOPrefs::instance()->mAutoSaveInterval);
  mConfirmCheck->setChecked(KOPrefs::instance()->mConfirm);

  setCombo(mHolidayCombo,KOPrefs::instance()->mHoliday, &mHolidayList);
  
  setCombo(mTimeZoneCombo,KOPrefs::instance()->mTimeZone);

  mStartTimeSpin->setValue(KOPrefs::instance()->mStartTime);
  mDefaultDurationSpin->setValue(KOPrefs::instance()->mDefaultDuration);
  mAlarmTimeCombo->setCurrentItem(KOPrefs::instance()->mAlarmTime);

  mDayBeginsSpin->setValue(KOPrefs::instance()->mDayBegins);
  mHourSizeSlider->setValue(KOPrefs::instance()->mHourSize);
  mDailyRecurCheck->setChecked(KOPrefs::instance()->mDailyRecur);
  mWeeklyRecurCheck->setChecked(KOPrefs::instance()->mWeeklyRecur);
  mEnableToolTipsCheck->setChecked(KOPrefs::instance()->mEnableToolTips);

  mTimeBarFont->setFont(KOPrefs::instance()->mTimeBarFont);
  mMonthViewFont->setFont(KOPrefs::instance()->mMonthViewFont);
  mAgendaViewFont->setFont(KOPrefs::instance()->mAgendaViewFont);

  mHolidayColor->setBackgroundColor(KOPrefs::instance()->mHolidayColor);
  mHighlightColor->setBackgroundColor(KOPrefs::instance()->mHighlightColor);
  mEventColor->setBackgroundColor(KOPrefs::instance()->mEventColor);
  mAgendaBgColor->setBackgroundColor(KOPrefs::instance()->mAgendaBgColor);

  setCombo(mPrinterCombo,KOPrefs::instance()->mPrinter);

  mPaperSizeGroup->setButton(KOPrefs::instance()->mPaperSize);
  mPaperOrientationGroup->setButton(KOPrefs::instance()->mPaperOrientation);
  mPrintPreviewEdit->setText(KOPrefs::instance()->mPrintPreview);
}


void KOOptionsDialog::writeConfig()
{
  qDebug("KOOptionsDialog::writeConfig()");

  KOPrefs::instance()->mBcc = mBccCheck->isChecked();
  KOPrefs::instance()->mAutoSave = mAutoSaveCheck->isChecked();
  KOPrefs::instance()->mAutoSaveInterval = mAutoSaveIntervalSpin->value();
  KOPrefs::instance()->mConfirm = mConfirmCheck->isChecked();

  KOPrefs::instance()->mName = mNameEdit->text();
  KOPrefs::instance()->mEmail = mEmailEdit->text();
  KOPrefs::instance()->mAdditional = mAdditionalEdit->text();
  KOPrefs::instance()->mHoliday = *mHolidayList.at(mHolidayCombo->currentItem());
  qDebug("%s", KOPrefs::instance()->mHoliday.latin1());

  KOPrefs::instance()->mTimeZone = mTimeZoneCombo->currentText();
  KOPrefs::instance()->mStartTime = mStartTimeSpin->value();
  KOPrefs::instance()->mDefaultDuration = mDefaultDurationSpin->value();
  KOPrefs::instance()->mAlarmTime = mAlarmTimeCombo->currentItem();

  KOPrefs::instance()->mDayBegins = mDayBeginsSpin->value();
  KOPrefs::instance()->mHourSize = mHourSizeSlider->value();
  KOPrefs::instance()->mDailyRecur = mDailyRecurCheck->isChecked();
  KOPrefs::instance()->mWeeklyRecur = mWeeklyRecurCheck->isChecked();
  KOPrefs::instance()->mEnableToolTips = mEnableToolTipsCheck->isChecked();

  KOPrefs::instance()->mTimeBarFont = mTimeBarFont->font();
  KOPrefs::instance()->mMonthViewFont = mMonthViewFont->font();
  KOPrefs::instance()->mAgendaViewFont = mAgendaViewFont->font();

  KOPrefs::instance()->mHolidayColor = mHolidayColor->backgroundColor();
  KOPrefs::instance()->mHighlightColor = mHighlightColor->backgroundColor();
  KOPrefs::instance()->mEventColor = mEventColor->backgroundColor();
  KOPrefs::instance()->mAgendaBgColor = mAgendaBgColor->backgroundColor();

  QDictIterator<QColor> it(mCategoryDict);
  while (it.current()) {
    KOPrefs::instance()->setCategoryColor(it.currentKey(),*it.current());
    ++it;
  }
  mCategoryDict.clear();

  KOPrefs::instance()->mPrinter = mPrinterCombo->currentText();
  KOPrefs::instance()->mPaperSize = 
      mPaperSizeGroup->id(mPaperSizeGroup->selected());
  KOPrefs::instance()->mPaperOrientation =
      mPaperOrientationGroup->id(mPaperOrientationGroup->selected());
  KOPrefs::instance()->mPrintPreview = mPrintPreviewEdit->text();
  
  KOPrefs::instance()->writeConfig();
}


void KOOptionsDialog::slotApply()
{
  writeConfig();
  emit configChanged();
}

void KOOptionsDialog::slotOk()
{
  slotApply();
  accept();
}

void KOOptionsDialog::slotDefault()
{
  if (KMessageBox::warningContinueCancel(this,
      i18n("You are about to set all preferences to default values. All "
      "custom modifications will be lost."),i18n("Setting Default Preferences"),
      i18n("Continue"))
    == KMessageBox::Continue) setDefaults(); 
}


void KOOptionsDialog::updateCategories()
{
  mCategoryCombo->clear();
  mCategoryCombo->insertStringList(KOPrefs::instance()->mCustomCategories);  
  updateCategoryColor();
}
