if include_knewstuff
knewstuff_SUBDIR=knewstuff
endif

SUBDIRS = $(knewstuff_SUBDIR) printing . korgac pixmaps sounds plugins interfaces

INCLUDES = \
  -I$(srcdir)/interfaces \
  -I$(top_srcdir)/libkdepim \
  -I$(top_srcdir)/libkpimidentities \
  -I$(top_srcdir)/mimelib \
  -I$(top_srcdir)/certmanager/lib \
  -I$(top_srcdir)/kresources/remote \
  -I$(top_srcdir)/korganizer/printing \
  -I$(top_srcdir) \
	-I$(top_builddir) \
  $(all_includes)

bin_PROGRAMS = korganizer

korganizer_LDFLAGS = $(all_libraries) $(KDE_RPATH)
korganizer_LDADD   = libkorganizer.la
korganizer_SOURCES = main.cpp korganizer.cpp koapp.cpp
korganizer_COMPILE_FIRST = koprefs_base.h


kde_module_LTLIBRARIES = kcm_korganizer.la libkorganizerpart.la

kcm_korganizer_la_SOURCES = koprefsdialog.cpp
kcm_korganizer_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
kcm_korganizer_la_LIBADD = libkorganizer.la $(top_builddir)/libkdepim/libkdepim.la \
  $(LIB_KDECORE)
kcm_korganizer_la_COMPILE_FIRST = koprefs_base.h 	kogroupwareprefspage.h

libkorganizerpart_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
libkorganizerpart_la_LIBADD  = libkorganizer.la
libkorganizerpart_la_SOURCES = korganizer_part.cpp
libkorganizerpart_la_COMPILE_FIRST = publishdialog_base.h koprefs_base.h


check_PROGRAMS = korgplugins timezone

korgplugins_LDFLAGS = $(all_libraries) $(KDE_RPATH)
korgplugins_LDADD   = libkorganizer.la $(LIB_KSYCOCA)
korgplugins_SOURCES = korgplugins.cpp
korgplugins_COMPILE_FIRST = koprefs_base.h

timezone_LDFLAGS = $(all_libraries) $(KDE_RPATH)
timezone_LDADD   = libkorganizer.la $(LIB_KSYCOCA)
timezone_SOURCES = timezone.cpp
timezone_COMPILE_FIRST = koprefs_base.h

lib_LTLIBRARIES = libkorganizer.la libkorganizer_eventviewer.la \
                  libkorganizer_calendar.la

libkorganizer_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -version-info 1:0
libkorganizer_la_LIBADD  = $(LIB_KPARTS) $(LIB_KFILE) \
	-lkdeprint -lkabc -lkutils \
	$(top_builddir)/libkcal/libkcal.la \
	$(top_builddir)/libkpimidentities/libkpimidentities.la \
	$(top_builddir)/libkdepim/libkdepim.la \
	libkorganizer_eventviewer.la \
	libkorganizer_calendar.la \
	printing/libkocorehelper.la \
	printing/libkorg_stdprinting.la \
	$(top_builddir)/kdgantt/libkdgantt.la \
	$(KNEWSTUFF_LIB) \
	$(top_builddir)/kresources/remote/libkcal_resourceremote.la \
        $(top_builddir)/libemailfunctions/libemailfunctions.la
libkorganizer_la_COMPILE_FIRST = \
	outgoingdialog_base.ui \
	incomingdialog_base.ui \
	kogroupwareprefspage.ui \
	$(top_srcdir)/libkdepim/categoryselectdialog.h
libkorganizer_la_SOURCES = komessagebox.cpp outgoingdialog_base.ui \
	koagendaview.cpp koagenda.cpp koagendaitem.cpp \
	datenavigator.cpp kdatenavigator.cpp datenavigatorcontainer.cpp \
	datechecker.cpp \
	komonthview.cpp \
	searchdialog.cpp calendarview.cpp koviewmanager.cpp \
	kodialogmanager.cpp \
	archivedialog.cpp \
	komailclient.cpp kotodoview.cpp kotodoviewitem.cpp \
	kolistview.cpp \
	koincidenceeditor.cpp koeventeditor.cpp kotodoeditor.cpp kojournaleditor.cpp\
	koprefs.cpp kowindowlist.cpp \
	koeditorgeneral.cpp \
	koeditorgeneralevent.cpp koeditorgeneraltodo.cpp koeditorgeneraljournal.cpp \
	koeditordetails.cpp koeditoralarms_base.ui koeditoralarms.cpp \
	koeditorrecurrence.cpp koeditorattachments.cpp \
	kogroupwareprefspage.ui koeventpopupmenu.cpp koeditorfreebusy.cpp \
	outgoingdialog.cpp \
	incomingdialog_base.ui incomingdialog.cpp \
	publishdialog_base.ui publishdialog.cpp \
	koeventview.cpp \
	korganizeriface.skel kcalendariface.skel \
	filtereditdialog.cpp filteredit_base.ui \
	kowhatsnextview.cpp kocounterdialog.cpp \
	kojournalview.cpp journalentry.cpp \
	kocore.cpp mailscheduler.cpp \
	kodaymatrix.cpp docprefs.cpp statusdialog.cpp\
	lineview.cpp timeline.cpp \
	koglobals.cpp konewstuff.cpp \
	alarmclient.cpp \
	actionmanager.cpp resourceview.cpp \
	navigatorbar.cpp kogroupware.cpp \
	kogroupwareincomingdialog_base.ui kogroupwareincomingdialog.cpp \
	history.cpp \
	koprefs_base.kcfgc \
	koincidencetooltip.cpp aboutdata.cpp \
	importdialog.cpp \
	korganizerifaceimpl.cpp \
	freebusymanager.cpp freebusyurldialog.cpp \
	eventarchiver.cpp koidentitymanager.cpp \
	exportwebdialog.cpp kocorehelper.cpp incidencechanger.cpp


libkorganizer_eventviewer_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) \
                                       -no-undefined -version-info 1:0
libkorganizer_eventviewer_la_LIBADD  = $(LIB_KPARTS) \
                                       $(top_builddir)/libkcal/libkcal.la
libkorganizer_eventviewer_la_SOURCES = koeventviewer.cpp urihandler.cpp \
                                       kmailIface.stub koeventviewerdialog.cpp

libkorganizer_calendar_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) \
                                    -no-undefined -version-info 1:0
libkorganizer_calendar_la_LIBADD  = $(LIB_KPARTS) \
                                    $(top_builddir)/libkdepim/libkdepim.la \
                                    $(top_builddir)/libkcal/libkcal.la \
                                    $(top_builddir)/kresources/remote/libkcal_resourceremote.la
libkorganizer_calendar_la_SOURCES = stdcalendar.cpp


kmailIface_DIR = $(top_srcdir)/kmail
kmailIface_DCOPIDLNG = true

METASOURCES = AUTO

noinst_HEADERS = komonthview.h \
	koagendaview.h koagenda.h \
	koagendaitem.h \
	kdatenavigator.h \
	calendarview.h \
	version.h \
	searchdialog.h \
	archivedialog.h \
	komailclient.h kotodoview.h \
	kolistview.h \
	koapp.h korganizer_part.h korganizer.h \
	koprefsdialog.h koeventeditor.h \
	kotodoeditor.h koprefs.h koeventviewer.h \
	kowindowlist.h kocounterdialog.h \
	koeditorgeneralevent.h koeditorgeneraltodo.h koeditordetails.h \
	koeditorrecurrence.h koeventviewerdialog.h koeventpopupmenu.h \
	outgoingdialog.h incomingdialog.h publishdialog.h koeditorfreebusy.h \
	koeventview.h statusdialog.h customlistviewitem.h \
	alarmclient.h \
	navigatorbar.h kogroupware.h kogroupwareincomingdialog.h \
	koincidencetooltip.h korganizerifaceimpl.h \
	exportwebdialog.h

tip_DATA = tips
tipdir = $(kde_datadir)/korganizer

kde_bin_SCRIPTS = ical2vcal korganizerIn

rcdir = $(kde_datadir)/korganizer
rc_DATA = korganizerui.rc

partdir   = $(kde_datadir)/korganizer
part_DATA = korganizer_part.rc

kde_servicetypes_DATA = dcopcalendar.desktop

KDE_ICON = AUTO

META_INCLUDES = $(srcdir)/interfaces/korganizer

messages: rc.cpp
	$(PREPARETIPS) > tips.cc
	$(XGETTEXT) *.cpp *.h korgac/*.cpp knewstuff/*.cpp plugins/*/*.cpp plugins/*/*/*.cpp printing/*.cpp tips.cc -o $(podir)/korganizer.pot
	rm -f tips.cc

xdg_apps_DATA = korganizer.desktop

kde_kcfg_DATA = korganizer.kcfg

kde_services_DATA = korganizer_configmain.desktop \
                    korganizer_configtime.desktop \
                    korganizer_configviews.desktop \
                    korganizer_configcolors.desktop \
                    korganizer_configfonts.desktop \
                    korganizer_configplugins.desktop \
                    korganizer_configgroupscheduling.desktop \
                    korganizer_configgroupautomation.desktop \
                    korganizer_configfreebusy.desktop \
                    webcal.protocol

update_DATA = korganizer.upd
updatedir = $(kde_datadir)/kconf_update

include ../admin/Doxyfile.am
