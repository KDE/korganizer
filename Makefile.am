# set the include path for X, qt and KDE, and versit library for vCalendar
INCLUDES= $(all_includes) \
	   -I$(srcdir)/versit 

bin_PROGRAMS = 	korganizer alarmd
lib_LTLIBRARIES = libkorganizer.la

libkorganizer_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
libkorganizer_la_LIBADD  = -lkparts \
                           versit/libversit.la $(LIB_KFILE) -lkab
libkorganizer_la_LDFLAGS = $(all_libraries) -version-info 1:0:0 -module

korganizer_LDFLAGS = $(all_libraries) $(KDE_RPATH)
korganizer_SOURCES = main.cpp
korganizer_LDADD   = libkorganizer.la

alarmd_LDFLAGS = $(all_libraries) $(KDE_RPATH)
alarmd_LDADD = versit/libversit.la $(LIB_KDEUI)

########## FILES

# Which sources should be compiled for korganizer.
libkorganizer_la_SOURCES =  calobject.cpp catdlg.cpp \
	koagendaview.cpp koagenda.cpp koagendaitem.cpp \
	kdateedit.cpp kdatenav.cpp \
	kdpdatebutton.cpp komonthview.cpp koevent.cpp kpbutton.cpp \
	ksellabel.cpp \
	ktimeedit.cpp main.cpp  \
	searchdialog.cpp calendarview.cpp \
	calprinter.cpp \
	koarchivedlg.cpp \
	scanholiday.c parseholiday.c \
	vcaldrag.cpp komailclient.cpp kotodoview.cpp \
        kobaseview.cpp kolistview.cpp \
	koapp.cpp exportwebdialog.cpp korganizer_part.cpp korganizer.cpp \
        kooptionsdialog.cpp koeventeditor.cpp \
        kotodoeditor.cpp koprefs.cpp koeventviewer.cpp kowindowlist.cpp \
        koeditorgeneralevent.cpp koeditorgeneraltodo.cpp koeditordetails.cpp \
        koeditorrecurrence.cpp koeventviewerdialog.cpp

alarmd_SOURCES = alarmdaemon.cpp alarmmain.cpp calobject.cpp koevent.cpp \
	scanholiday.c parseholiday.c vcaldrag.cpp koprefs.cpp alarmapp.cpp \
        alarmdialog.cpp koeventviewer.cpp

METASOURCES = AUTO

noinst_HEADERS = komonthview.h calobject.h \
	kpbutton.h catdlg.h koagendaview.h koagenda.h \
	koagendaitem.h \
	kdateedit.h ksellabel.h \
	kdatenav.h ksmalldaydisp.h \
	calendarview.h kdpdatebutton.h ktimeedit.h \
	version.h koevent.h \
	searchdialog.h calprinter.h \
	koarchivedlg.h parseholiday.h \
	vcaldrag.h komailclient.h kotodoview.h \
        kobaseview.h kolistview.h \
	alarmapp.h koapp.h exportwebdialog.h korganizer_part.h korganizer.h \
        kooptionsdialog.h koeventeditor.h \
        kotodoeditor.h koprefs.h alarmdaemon.h koeventviewer.h alarmdialog.h \
        kowindowlist.h \
        koeditorgeneralevent.h koeditorgeneraltodo.h koeditordetails.h \
        koeditorrecurrence.h koeventviewerdialog.h


SUBDIRS = versit \
	  pixmaps \
	  holidays \
	  sounds


datadir = $(kde_bindir)
data_SCRIPTS = ical2vcal

kde_conf_DATA = korganizerrc

kdelnkdir = $(kde_appsdir)/Applications
kdelnk_DATA = korganizer.desktop

rcdir = $(kde_datadir)/korganizer
rc_DATA = korganizerui.rc

partdir   = $(kde_datadir)/korganizer
part_DATA = korganizer_part.rc


messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/korganizer.pot

$(srcdir)/parseholiday.c: parseholiday.y
	cd $(srcdir) && bison -d -o parseholiday.c parseholiday.y

$(srcdir)/scanholiday.c: scanholiday.lex parseholiday.h
	cd $(srcdir) && flex -B -oscanholiday.c scanholiday.lex

