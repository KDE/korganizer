# set the include path for X, qt and KDE, and versit library for vCalendar
INCLUDES= $(all_includes) \
	   -I$(srcdir)/versit 

bin_PROGRAMS = 	korganizer alarmd
lib_LTLIBRARIES = libkorganizer.la

libkorganizer_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
libkorganizer_la_LIBADD  = -lkparts \
                           versit/libversit.a $(LIB_KFILE) -lkab
libkorganizer_la_LDFLAGS = $(all_libraries) -version-info 1:0:0 -module

korganizer_LDFLAGS = $(all_libraries) $(KDE_RPATH)
korganizer_SOURCES = main.cpp
korganizer_LDADD   = libkorganizer.la

alarmd_LDFLAGS = $(all_libraries) $(KDE_RPATH)
alarmd_LDADD = versit/libversit.a $(LIB_KDEUI)

########## FILES

# Which sources should be compiled for korganizer.
libkorganizer_la_SOURCES =  calobject.cpp catdlg.cpp \
	eventwidget.cpp \
	koagendaview.cpp koagenda.cpp koagendaitem.cpp \
	kdated.cpp kdatenav.cpp \
	kdpdatebutton.cpp komonthview.cpp koevent.cpp kpbutton.cpp \
	kdpquickevt.cpp ksellabel.cpp \
	ktimed.cpp main.cpp  \
	searchdialog.cpp calendarview.cpp quickdialog.cpp \
	calprinter.cpp kptbutton.cpp \
	koarchivedlg.cpp optionsdlg.cpp kpropdlg.cpp kpropcheck.cpp \
	kpropcombo.cpp kpropradio.cpp kpropfont.cpp \
	kpropcolor.cpp kpropspin.cpp kproptext.cpp \
	scanholiday.c parseholiday.c aboutdlg.cpp \
	vcaldrag.cpp kpropgroup.cpp komailclient.cpp kotodoview.cpp \
	dateedit.cpp datevalidator.cpp \
        kobaseview.cpp kolistview.cpp \
	koapp.cpp exportwebdialog.cpp korganizer_part.cpp korganizer.cpp \
        kooptionsdialog.cpp koeventeditor.cpp koeditorwidgets.cpp \
        kotodoeditor.cpp

alarmd_SOURCES = alarmdaemon.cpp alarmmain.cpp calobject.cpp koevent.cpp \
	scanholiday.c parseholiday.c vcaldrag.cpp

METASOURCES = AUTO

noinst_HEADERS = komonthview.h misc.h calobject.h \
	kpbutton.h optionsdlg.h catdlg.h koagendaview.h koagenda.h \
	koagendaitem.h kptbutton.h \
	quickdialog.h kdated.h ksellabel.h \
	eventwidget.h kdatenav.h ksmalldaydisp.h \
	calendarview.h kdpdatebutton.h ktimed.h \
	version.h koevent.h \
	kdpquickevt.h \
	searchdialog.h calprinter.h kptbutton.h \
	koarchivedlg.h kpropdlg.h kpropcheck.h \
	kpropcombo.h kpropradio.h kproptext.h kpropfont.h \
	kpropcolor.h kpropspin.h parseholiday.h aboutdlg.h \
	vcaldrag.h kpropgroup.h komailclient.h kotodoview.h \
	dateedit.h datevalidator.h \
        kobaseview.h kolistview.h \
	alarmapp.h koapp.h exportwebdialog.h korganizer_part.h korganizer.h \
        kooptionsdialog.h koeventeditor.h koeditorwidgets.h \
        kotodoeditor.h


SUBDIRS = versit \
	  pixmaps \
	  holidays \
	  sounds


datadir = $(kde_bindir)
data_SCRIPTS = ical2vcal

kde_conf_DATA = korganizerrc

kdelnkdir = $(kde_appsdir)/Applications
kdelnk_DATA = korganizer.desktop

mimedir = $(kde_mimedir)/text
mime_DATA = x-vcalendar.desktop

rcdir = $(kde_datadir)/korganizer
rc_DATA = korganizer.rc

partdir   = $(kde_datadir)/korganizer
part_DATA = korganizer_part.rc


messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/korganizer.pot

$(srcdir)/parseholiday.c: parseholiday.y
	cd $(srcdir) && bison -d -o parseholiday.c parseholiday.y

$(srcdir)/scanholiday.c: scanholiday.lex parseholiday.h
	cd $(srcdir) && flex -B -oscanholiday.c scanholiday.lex

