KDE_OPTIONS = nofinal

SUBDIRS = knewstuff korgac . pixmaps sounds plugins interfaces

INCLUDES = \
  -I$(srcdir)/interfaces -I$(top_srcdir)/libkdepim -I$(top_srcdir) $(all_includes)

bin_PROGRAMS = korganizer

korganizer_LDFLAGS = $(all_libraries) $(KDE_RPATH)
korganizer_LDADD   = libkorganizer.la
korganizer_SOURCES = main.cpp korganizer.cpp koapp.cpp
korganizer_COMPILE_FIRST = koprefs.h


kde_module_LTLIBRARIES = kcm_korganizer.la libkorganizerpart.la

kcm_korganizer_la_SOURCES = koprefsdialog.cpp
kcm_korganizer_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
kcm_korganizer_la_LIBADD = libkorganizer.la $(top_builddir)/libkdepim/libkdepim.la \
  $(LIB_KDECORE)

libkorganizerpart_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
libkorganizerpart_la_LIBADD  = libkorganizer.la
libkorganizerpart_la_SOURCES = korganizer_part.cpp
libkorganizerpart_la_COMPILE_FIRST = publishdialog_base.h koprefs.h


check_PROGRAMS = korgplugins timezone

korgplugins_LDFLAGS = $(all_libraries) $(KDE_RPATH)
korgplugins_LDADD   = libkorganizer.la $(LIB_KSYCOCA)
korgplugins_SOURCES = korgplugins.cpp
korgplugins_COMPILE_FIRST = koprefs.h

timezone_LDFLAGS = $(all_libraries) $(KDE_RPATH)
timezone_LDADD   = libkorganizer.la $(LIB_KSYCOCA)
timezone_SOURCES = timezone.cpp
timezone_COMPILE_FIRST = koprefs.h

koprefs_base.h koprefs_base.cpp: $(srcdir)/koprefs.cfg $(top_builddir)/libkdepim/cfgc/cfgc
	$(top_builddir)/libkdepim/cfgc/cfgc $(srcdir)/koprefs.cfg
$(srcdir)/koprefs.h: koprefs_base.h


lib_LTLIBRARIES = libkorganizer.la

libkorganizer_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -version-info 1:0
libkorganizer_la_LIBADD  = $(LIB_KPARTS) $(LIB_KFILE) \
                           -lkdeprint -lkabc -lkutils \
                           $(top_builddir)/libkcal/libkcal.la \
                           $(top_builddir)/libkdepim/libkdepim.la \
                           $(top_builddir)/kalarmd/libkalarmd.la \
                           $(top_builddir)/korganizer/knewstuff/libknewstuff.la
libkorganizer_la_COMPILE_FIRST = kofilterview_base.ui \
	outgoingdialog_base.ui \
	incomingdialog_base.ui \
	../libkdepim/categoryselectdialog.h \
	../kalarmd/alarmdaemoniface_stub.h \
	koprefs_base.h
libkorganizer_la_SOURCES =  outgoingdialog_base.ui \
	koagendaview.cpp koagenda.cpp koagendaitem.cpp \
	datenavigator.cpp kdatenavigator.cpp \
	komonthview.cpp \
	ktimeedit.cpp \
	searchdialog.cpp calendarview.cpp koviewmanager.cpp \
        kodialogmanager.cpp \
	calprintdayconfig_base.ui calprintweekconfig_base.ui \
	calprintmonthconfig_base.ui calprinttodoconfig_base.ui \
	calprinter.cpp calprintbase.cpp calprintplugins.cpp \
	archivedialog.cpp \
	komailclient.cpp kotodoview.cpp kotodoviewitem.cpp \
        kolistview.cpp \
        koprefsdialog.cpp \
        koincidenceeditor.cpp koeventeditor.cpp kotodoeditor.cpp \
        koprefs.cpp koeventviewer.cpp kowindowlist.cpp \
        koeditorgeneral.cpp \
        koeditorgeneralevent.cpp koeditorgeneraltodo.cpp koeditordetails.cpp \
        koeditorrecurrence.cpp koeventviewerdialog.cpp \
        koeventpopupmenu.cpp \
        outgoingdialog.cpp \
	incomingdialog_base.ui incomingdialog.cpp \
	publishdialog_base.ui publishdialog.cpp \
        koeventview.cpp \
        kofilterview_base.ui kofilterview.cpp \
        korganizeriface.skel kcalendariface.skel \
        filtereditdialog.cpp filteredit_base.ui \
        kowhatsnextview.cpp kocounterdialog.cpp \
        kojournalview.cpp journalentry.cpp \
        kocore.cpp plugindialog.cpp mailscheduler.cpp \
	kodaymatrix.cpp docprefs.cpp statusdialog.cpp\
        kotimespanview.cpp timespanview.cpp lineview.cpp timeline.cpp \
        koglobals.cpp konewstuff.cpp savetemplatedialog.cpp \
        kalarmdclient.cpp simplealarmclient.cpp \
	actionmanager.cpp resourceview.cpp \
        navigatorbar.cpp kogroupware.cpp \
	kogroupwareincomingdialog_base.ui kogroupwareincomingdialog.cpp \
        history.cpp \
        koprefs_base.cpp koincidencetooltip.cpp aboutdata.cpp

METASOURCES = AUTO

noinst_HEADERS = komonthview.h \
	koagendaview.h koagenda.h \
	koagendaitem.h \
	kdatenavigator.h \
	calendarview.h ktimeedit.h \
	version.h \
	searchdialog.h calprinter.h calprintbase.h calprintplugins.h \
	archivedialog.h \
	komailclient.h kotodoview.h \
        kolistview.h \
	koapp.h korganizer_part.h korganizer.h \
        koprefsdialog.h koeventeditor.h \
        kotodoeditor.h koprefs.h koeventviewer.h \
        kowindowlist.h kocounterdialog.h \
        koeditorgeneralevent.h koeditorgeneraltodo.h koeditordetails.h \
        koeditorrecurrence.h koeventviewerdialog.h koeventpopupmenu.h \
        outgoingdialog.h incomingdialog.h publishdialog.h \
        koeventview.h statusdialog.h customlistviewitem.h \
        alarmclient.h kalarmdclient.h simplealarmclient.h \
        navigatorbar.h kogroupware.h kogroupwareincomingdialog.h \
	koincidencetooltip.h

DISTCLEANFILES = koprefs_base.cpp

tip_DATA = tips
tipdir = $(kde_datadir)/korganizer

kde_bin_SCRIPTS = ical2vcal korganizerIn

xdg_apps_DATA = korganizer.desktop

rcdir = $(kde_datadir)/korganizer
rc_DATA = korganizerui.rc korganizergsui.rc

partdir   = $(kde_datadir)/korganizer
part_DATA = korganizer_part.rc

kde_services_DATA = webcal.protocol
kde_servicetypes_DATA = dcopcalendar.desktop

KDE_ICON = AUTO

META_INCLUDES = $(srcdir)/interfaces/korganizer

messages: rc.cpp
	$(PREPARETIPS) > tips.cc
	$(XGETTEXT) *.cpp knewstuff/*.cpp plugins/*/*.cpp tips.cc -o $(podir)/korganizer.pot
	rm -f tips.cc

include ../admin/Doxyfile.am

pim_DATA = configmain.desktop configtime.desktop \
  configviews.desktop configcolors.desktop \
  configfonts.desktop configgroupscheduling.desktop \
  configgroupautomation.desktop
pimdir = $(kde_appsdir)/PIM/korganizer
