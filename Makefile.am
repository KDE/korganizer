# set the include path for X, qt and KDE, and versit library for vCalendar
INCLUDES = -I$(top_srcdir)/kgantt/kgantt -I$(top_srcdir)/libkcal $(all_includes)

bin_PROGRAMS = 	korganizer alarmd
lib_LTLIBRARIES = libkorganizer.la

libkorganizer_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
libkorganizer_la_LIBADD  = $(LIB_KPARTS) $(LIB_KFILE) \
                           -lkdeprint -lkab \
                           $(top_builddir)/kgantt/kgantt/libkgantt.la \
                           $(top_builddir)/libkcal/libkcal.la
                           
libkorganizer_la_LDFLAGS = $(all_libraries) -version-info 1:0:0 -module

korganizer_LDFLAGS = $(all_libraries) $(KDE_RPATH)
korganizer_SOURCES = main.cpp
korganizer_LDADD   = libkorganizer.la

alarmd_LDFLAGS = $(all_libraries) $(KDE_RPATH)
alarmd_LDADD = $(LIB_KDEUI) $(top_builddir)/libkcal/libkcal.la


########## FILES

# Which sources should be compiled for korganizer.
libkorganizer_la_SOURCES =  outgoingdialog_base.ui \
	koagendaview.cpp koagenda.cpp koagendaitem.cpp \
	kdateedit.cpp kdatenav.cpp \
	kdpdatebutton.cpp komonthview.cpp \
	ksellabel.cpp \
	ktimeedit.cpp main.cpp  \
	searchdialog.cpp calendarview.cpp \
	calprinter.cpp \
	koarchivedlg.cpp \
	komailclient.cpp kotodoview.cpp \
        kobaseview.cpp kolistview.cpp \
        exportwebdialog.cpp htmlexport.cpp \
        koapp.cpp korganizer_part.cpp korganizer.cpp \
        kprefsdialog.cpp koprefsdialog.cpp \
        koincidenceeditor.cpp koeventeditor.cpp kotodoeditor.cpp \
        koprefs.cpp koeventviewer.cpp kowindowlist.cpp \
        koeditorgeneralevent.cpp koeditorgeneraltodo.cpp koeditordetails.cpp \
        koeditorrecurrence.cpp koeventviewerdialog.cpp \
        koeventpopupmenu.cpp kprefs.cpp \
        outgoingdialog.cpp \
        incomingdialog_base.ui incomingdialog.cpp \
        koeventview.cpp categoryselectdialog_base.ui categoryselectdialog.cpp \
        categoryeditdialog_base.ui categoryeditdialog.cpp \
        kofilterview_base.ui kofilterview.cpp \
        koprojectview.cpp \
        korganizeriface.skel \
        filtereditdialog.cpp filteredit_base.ui \
        kowhatsnextview.cpp \
        kojournalview.cpp journalentry.cpp

alarmd_SOURCES = alarmdaemon.cpp alarmmain.cpp \
	koprefs.cpp alarmapp.cpp \
        alarmdialog.cpp koeventviewer.cpp kprefs.cpp

METASOURCES = AUTO

noinst_HEADERS = komonthview.h \
	koagendaview.h koagenda.h \
	koagendaitem.h \
	kdateedit.h ksellabel.h \
	kdatenav.h \
	calendarview.h kdpdatebutton.h ktimeedit.h \
	version.h \
	searchdialog.h calprinter.h \
	koarchivedlg.h \
	komailclient.h kotodoview.h \
        kobaseview.h kolistview.h \
	alarmapp.h koapp.h exportwebdialog.h korganizer_part.h korganizer.h \
        kprefsdialog.h koprefsdialog.h koeventeditor.h \
        kotodoeditor.h kprefs.h koprefs.h alarmdaemon.h koeventviewer.h \
        alarmdialog.h \
        kowindowlist.h \
        koeditorgeneralevent.h koeditorgeneraltodo.h koeditordetails.h \
        koeditorrecurrence.h koeventviewerdialog.h koeventpopupmenu.h \
        outgoingdialog.h incomingdialog.h \
        koeventview.h koprojectview.h


SUBDIRS = pixmaps sounds

autostart_DATA = alarmd.desktop
autostartdir = $(prefix)/share/autostart

tip_DATA = tips
tipdir = $(kde_datadir)/korganizer

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
	preparetips > tips.cc
	$(XGETTEXT) *.cpp tips.cc -o $(podir)/korganizer.pot
	rm -f tips.cc
