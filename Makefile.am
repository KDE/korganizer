# set the include path for X, qt and KDE, and versit library for vCalendar
INCLUDES= $(all_includes) \
	   -I$(srcdir)/versit \
           -I$(top_srcdir)/libical/src/libical \
           -I$(top_srcdir)/libical/src/libicalss \
	   -I$(top_builddir)/libical/src/libical \
	   -I$(top_builddir)/libical/src/libicalss

bin_PROGRAMS = 	korganizer alarmd
lib_LTLIBRARIES = libkorganizer.la

libkorganizer_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
libkorganizer_la_LIBADD  = -lkparts \
                           versit/libversit.la $(LIB_KFILE) -lkab \
                           $(top_builddir)/libical/src/libical/libical.la \
                           $(top_builddir)/libical/src/libicalss/libicalss.la
libkorganizer_la_LDFLAGS = $(all_libraries) -version-info 1:0:0 -module

korganizer_LDFLAGS = $(all_libraries) $(KDE_RPATH)
korganizer_SOURCES = main.cpp
korganizer_LDADD   = libkorganizer.la

alarmd_LDFLAGS = $(all_libraries) $(KDE_RPATH)
alarmd_LDADD = versit/libversit.la $(LIB_KDEUI) \
               $(top_builddir)/libical/src/libical/libical.la \
               $(top_builddir)/libical/src/libicalss/libicalss.la


########## FILES

# Which sources should be compiled for korganizer.
libkorganizer_la_SOURCES =  outgoingdialog_base.ui \
        calobject.cpp calendarlocal.cpp calformat.cpp vcalformat.cpp \
        icalformat.cpp catdlg.cpp \
	koagendaview.cpp koagenda.cpp koagendaitem.cpp \
	kdateedit.cpp kdatenav.cpp \
	kdpdatebutton.cpp komonthview.cpp koevent.cpp \
	ksellabel.cpp \
	ktimeedit.cpp main.cpp  \
	searchdialog.cpp calendarview.cpp \
	calprinter.cpp \
	koarchivedlg.cpp \
	vcaldrag.cpp komailclient.cpp kotodoview.cpp \
        kobaseview.cpp kolistview.cpp \
	koapp.cpp exportwebdialog.cpp korganizer_part.cpp korganizer.cpp \
        kprefsdialog.cpp koprefsdialog.cpp koeventeditor.cpp \
        kotodoeditor.cpp koprefs.cpp koeventviewer.cpp kowindowlist.cpp \
        koeditorgeneralevent.cpp koeditorgeneraltodo.cpp koeditordetails.cpp \
        koeditorrecurrence.cpp koeventviewerdialog.cpp \
        koeventpopupmenu.cpp kprefs.cpp koexceptions.cpp \
        outgoingdialog.cpp \
        incomingdialog_base.ui incomingdialog.cpp \
        scheduler.cpp imipscheduler.cpp dummyscheduler.cpp \
        koeventview.cpp categoryselectdialog_base.ui categoryselectdialog.cpp \
        categoryeditdialog_base.ui categoryeditdialog.cpp calfilter.cpp \
        kofilterview_base.ui kofilterview.cpp \
        scanholiday.c parseholiday.c \
        koprojectview.cpp xQGantt.cpp xQTask.cpp xQGanttListView.cpp \
        xQGanttBarView.cpp xQGanttListViewPort.cpp xQGanttBarViewPort.cpp \
        xQGanttBarViewPort_Events.cpp \
        korganizeriface.skel

alarmd_SOURCES = alarmdaemon.cpp alarmmain.cpp \
        calobject.cpp calendarlocal.cpp \
        calformat.cpp vcalformat.cpp icalformat.cpp \
        koevent.cpp \
	vcaldrag.cpp koprefs.cpp alarmapp.cpp \
        alarmdialog.cpp koeventviewer.cpp kprefs.cpp koexceptions.cpp \
        scheduler.cpp calfilter.cpp \
        scanholiday.c parseholiday.c

METASOURCES = AUTO

noinst_HEADERS = komonthview.h \
        calobject.h calendarlocal.h calformat.h vcalformat.h icalformat.h \
	catdlg.h koagendaview.h koagenda.h \
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
        kprefsdialog.h koprefsdialog.h koeventeditor.h \
        kotodoeditor.h kprefs.h koprefs.h alarmdaemon.h koeventviewer.h \
        alarmdialog.h \
        kowindowlist.h \
        koeditorgeneralevent.h koeditorgeneraltodo.h koeditordetails.h \
        koeditorrecurrence.h koeventviewerdialog.h koeventpopupmenu.h \
        koexceptions.h outgoingdialog.h incomingdialog.h \
        koeventview.h calfilter.h koprojectview.h \
        xQGantt.h xQTask.h


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
rc_DATA = korganizerui.rc korganizergsui.rc

partdir   = $(kde_datadir)/korganizer
part_DATA = korganizer_part.rc

KDE_ICON = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/korganizer.pot

$(srcdir)/parseholiday.c: parseholiday.y
	cd $(srcdir) && bison -d -o parseholiday.c parseholiday.y

$(srcdir)/scanholiday.c: scanholiday.lex parseholiday.h
	cd $(srcdir) && flex -B -oscanholiday.c scanholiday.lex

