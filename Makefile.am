SUBDIRS = . pixmaps sounds plugins interfaces

INCLUDES = -I$(srcdir)/interfaces -I$(top_srcdir) $(all_includes)

bin_PROGRAMS = korganizer

korganizer_LDFLAGS = $(all_libraries) $(KDE_RPATH)
korganizer_LDADD   = libkorganizer.la
korganizer_SOURCES = main.cpp

check_PROGRAMS = korgplugins

korgplugins_LDFLAGS = $(all_libraries) $(KDE_RPATH)
korgplugins_LDADD   = libkorganizer.la $(LIB_KSYCOCA)
korgplugins_SOURCES = korgplugins.cpp

lib_LTLIBRARIES = libkorganizer.la

libkorganizer_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
libkorganizer_la_LIBADD  = $(LIB_KPARTS) $(LIB_KFILE) \
                           -lkdeprint -lkabc \
                           $(top_builddir)/libkcal/libkcal.la \
                           $(top_builddir)/kalarmd/libkalarmd.la
libkorganizer_la_SOURCES =  outgoingdialog_base.ui \
	koagendaview.cpp koagenda.cpp koagendaitem.cpp \
	kdateedit.cpp kdatenav.cpp \
	komonthview.cpp \
	ksellabel.cpp \
	ktimeedit.cpp main.cpp  \
	searchdialog.cpp calendarview.cpp koviewmanager.cpp \
        kodialogmanager.cpp \
	calprinter.cpp \
	archivedialog.cpp \
	komailclient.cpp kotodoview.cpp kotodoviewitem.cpp \
        kolistview.cpp \
        exportwebdialog.cpp htmlexport.cpp \
        koapp.cpp korganizer_part.cpp korganizer.cpp \
        kprefsdialog.cpp koprefsdialog.cpp \
        koincidenceeditor.cpp koeventeditor.cpp kotodoeditor.cpp \
        koprefs.cpp koeventviewer.cpp kowindowlist.cpp \
        koeditorgeneral.cpp \
        koeditorgeneralevent.cpp koeditorgeneraltodo.cpp koeditordetails.cpp \
        koeditorrecurrence.cpp koeventviewerdialog.cpp \
        koeventpopupmenu.cpp kprefs.cpp \
        outgoingdialog.cpp \
	incomingdialog_base.ui incomingdialog.cpp \
	publishdialog_base.ui publishdialog.cpp \
        koeventview.cpp categoryselectdialog_base.ui categoryselectdialog.cpp \
        categoryeditdialog_base.ui categoryeditdialog.cpp \
        kofilterview_base.ui kofilterview.cpp \
        korganizeriface.skel \
        filtereditdialog.cpp filteredit_base.ui \
        kowhatsnextview.cpp kocounterdialog.cpp \
        kojournalview.cpp journalentry.cpp \
        kocore.cpp plugindialog.cpp mailscheduler.cpp \
	kodaymatrix.cpp docprefs.cpp statusdialog.cpp\
        kotimespanview.cpp timespanview.cpp lineview.cpp timeline.cpp

METASOURCES = AUTO

noinst_HEADERS = komonthview.h \
	koagendaview.h koagenda.h \
	koagendaitem.h \
	kdateedit.h ksellabel.h \
	kdatenav.h \
	calendarview.h ktimeedit.h \
	version.h \
	searchdialog.h calprinter.h \
	archivedialog.h \
	komailclient.h kotodoview.h \
        kolistview.h \
	koapp.h exportwebdialog.h korganizer_part.h korganizer.h \
        kprefsdialog.h koprefsdialog.h koeventeditor.h \
        kotodoeditor.h kprefs.h koprefs.h koeventviewer.h \
        kowindowlist.h kocounterdialog.h \
        koeditorgeneralevent.h koeditorgeneraltodo.h koeditordetails.h \
        koeditorrecurrence.h koeventviewerdialog.h koeventpopupmenu.h \
        outgoingdialog.h incomingdialog.h publishdialog.h \
        koeventview.h statusdialog.h

tip_DATA = tips
tipdir = $(kde_datadir)/korganizer

datadir = $(kde_bindir)
data_SCRIPTS = ical2vcal korganizerIn

kde_conf_DATA = korganizerrc

kdelnkdir = $(kde_appsdir)/Applications
kdelnk_DATA = korganizer.desktop

rcdir = $(kde_datadir)/korganizer
rc_DATA = korganizerui.rc korganizergsui.rc

partdir   = $(kde_datadir)/korganizer
part_DATA = korganizer_part.rc

KDE_ICON = AUTO

META_INCLUDES = $(srcdir)/interfaces/korganizer

messages: rc.cpp
	$(PREPARETIPS) > tips.cc
	$(XGETTEXT) *.cpp plugins/*/*.cpp tips.cc -o $(podir)/korganizer.pot
	rm -f tips.cc
