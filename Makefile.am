# set the include path for X, qt and KDE, and versit library for vCalendar
INCLUDES= $(all_includes) \
	   -I$(srcdir)/versit 

# this gives better compile-time warnings.
CXXFLAGS = -g -Wall -pedantic

bin_PROGRAMS = 	korganizer alarmd
lib_LTLIBRARIES = libkorganizer.la

libkorganizer_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
libkorganizer_la_LIBADD  = -lkparts \
                           versit/libversit.a $(LIB_KFILE) -lkab
libkorganizer_la_LDFLAGS = $(all_libraries) -version-info 1:0:0 -module

korganizer_LDFLAGS = $(KDE_RPATH)
korganizer_SOURCES = main.cpp
korganizer_LDADD   = libkorganizer.la

alarmd_LDADD =  versit/libversit.a $(LIB_KDEUI)

########## FILES

# Which sources should be compiled for korganizer.
libkorganizer_la_SOURCES =  calobject.cpp catdlg.cpp editeventwin.cpp \
	eventwidget.cpp eventwingeneral.cpp eventwindetails.cpp \
	eventwinrecur.cpp koagendaview.cpp koagenda.cpp koagendaitem.cpp \
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
	dateedit.cpp datevalidator.cpp todoeventwin.cpp eventwin.cpp \
        wingeneral.cpp todowingeneral.cpp kobaseview.cpp kolistview.cpp \
	koapp.cpp exportwebdialog.cpp korganizer_part.cpp korganizer.cpp

alarmd_SOURCES = alarmdaemon.cpp alarmmain.cpp calobject.cpp koevent.cpp \
	scanholiday.c parseholiday.c vcaldrag.cpp

METASOURCES = AUTO

noinst_HEADERS = komonthview.h misc.h calobject.h \
	kpbutton.h optionsdlg.h catdlg.h koagendaview.h koagenda.h \
	koagendaitem.h kptbutton.h \
	quickdialog.h editeventwin.h kdated.h ksellabel.h \
	eventwidget.h kdatenav.h ksmalldaydisp.h \
	calendarview.h eventwindetails.h kdpdatebutton.h ktimed.h \
	version.h eventwingeneral.h koevent.h \
	eventwinrecur.h kdpquickevt.h \
	searchdialog.h calprinter.h kptbutton.h \
	koarchivedlg.h kpropdlg.h kpropcheck.h \
	kpropcombo.h kpropradio.h kproptext.h kpropfont.h \
	kpropcolor.h kpropspin.h parseholiday.h aboutdlg.h \
	vcaldrag.h kpropgroup.h komailclient.h kotodoview.h \
	dateedit.h datevalidator.h todoeventwin.h eventwin.h \
        wingeneral.h todowingeneral.h kobaseview.h kolistview.h \
	alarmapp.h koapp.h exportwebdialog.h korganizer_part.h korganizer.h


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

messages: rc.cpp
	$(XGETTEXT) rc.cpp $(korganizer_SOURCES) -o $(podir)/korganizer.pot

$(srcdir)/parseholiday.c: parseholiday.y
	cd $(srcdir) && bison -d -o parseholiday.c parseholiday.y

$(srcdir)/scanholiday.c: scanholiday.lex parseholiday.h
	cd $(srcdir) && flex -B -oscanholiday.c scanholiday.lex

