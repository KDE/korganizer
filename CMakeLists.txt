kde4_header()

add_subdirectory( printing ) 
add_subdirectory( korgac ) 
add_subdirectory( pixmaps ) 
add_subdirectory( sounds ) 
add_subdirectory( plugins ) 
add_subdirectory( interfaces ) 

include_directories( ${CMAKE_SOURCE_DIR}/libemailfunctions ${CMAKE_SOURCE_DIR}/libkpimidentities ${CMAKE_SOURCE_DIR}/libkholidays ${CMAKE_SOURCE_DIR}/mimelib ${CMAKE_SOURCE_DIR}/libkleo ${CMAKE_SOURCE_DIR}/korganizer/printing ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(korganizer_SRCS 
   main.cpp 
   korganizer.cpp 
   koapp.cpp 
   komessagebox.cpp 
   koagendaview.cpp 
   koagenda.cpp 
   koagendaitem.cpp 
   datenavigator.cpp 
   kdatenavigator.cpp 
   datenavigatorcontainer.cpp 
   datechecker.cpp 
   komonthview.cpp 
   searchdialog.cpp 
   calendarview.cpp 
   koviewmanager.cpp 
   kodialogmanager.cpp 
   archivedialog.cpp 
   kohelper.cpp 
   komailclient.cpp 
   kotodoview.cpp 
   kotodoviewitem.cpp 
   kotodoviewquicksearch.cpp 
   klistviewnewsearchline.cpp 
   kolistview.cpp 
   koincidenceeditor.cpp 
   koeventeditor.cpp 
   kotodoeditor.cpp 
   kojournaleditor.cpp 
   koprefs.cpp 
   kowindowlist.cpp 
   koeditorgeneral.cpp 
   koeditorgeneralevent.cpp 
   koeditorgeneraltodo.cpp 
   koeditorgeneraljournal.cpp 
   koeditordetails.cpp 
   koeditoralarms.cpp 
   koeditorrecurrence.cpp 
   koeditorattachments.cpp 
   koeventpopupmenu.cpp 
   koeditorfreebusy.cpp 
   publishdialog.cpp 
   koeventview.cpp 
   filtereditdialog.cpp 
   kowhatsnextview.cpp 
   kocounterdialog.cpp 
   kojournalview.cpp 
   journalentry.cpp 
   kocore.cpp 
   mailscheduler.cpp 
   kodaymatrix.cpp 
   docprefs.cpp 
   statusdialog.cpp 
   koglobals.cpp 
   konewstuff.cpp 
   alarmclient.cpp 
   actionmanager.cpp 
   resourceview.cpp 
   navigatorbar.cpp 
   kogroupware.cpp 
   history.cpp 
   aboutdata.cpp 
   importdialog.cpp 
   korganizerifaceimpl.cpp 
   freebusymanager.cpp 
   freebusyurldialog.cpp 
   eventarchiver.cpp 
   koidentitymanager.cpp 
   exportwebdialog.cpp 
   kocorehelper.cpp 
   incidencechanger.cpp 
   templatemanagementdialog.cpp )

kde4_automoc(${korganizer_SRCS})

kde4_add_ui3_files(korganizer_SRCS 
   kogroupwareprefspage.ui 
   publishdialog_base.ui 
   template_management_dialog_base.ui )

kde4_add_ui_files(korganizer_SRCS filteredit_base.ui koeditoralarms_base.ui)

kde4_add_dcop_skels(korganizer_SRCS korganizeriface.h kcalendariface.h )

kde4_add_kcfg_files(korganizer_SRCS koprefs_base.kcfgc )

kde4_add_executable(korganizer ${korganizer_SRCS})

target_link_libraries(korganizer  ${QT_AND_KDECORE_LIBS} korganizer kparts knewstuff kdeprint kabc kutils kcal kpimidentities kdepim korganizer_eventviewer korganizer_calendar kocorehelper korg_stdprinting kdgantt emailfunctions kholidays )

install_targets(/bin korganizer )


########### next target ###############

set(kcm_korganizer_PART_SRCS koprefsdialog.cpp )

kde4_automoc(${kcm_korganizer_PART_SRCS})

kde4_add_plugin(kcm_korganizer ${kcm_korganizer_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_korganizer )

target_link_libraries(kcm_korganizer  ${QT_AND_KDECORE_LIBS} korganizer kdepim )

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_korganizer )


########### next target ###############

set(korganizerpart_PART_SRCS korganizer_part.cpp )

kde4_automoc(${korganizerpart_PART_SRCS})

kde4_add_plugin(korganizerpart WITH_PREFIX ${korganizerpart_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} korganizerpart )

target_link_libraries(korganizerpart  ${QT_AND_KDECORE_LIBS} korganizer )

install_targets(${LIB_INSTALL_DIR}/kde4 korganizerpart )


########### next target ###############

set(korgplugins_SRCS korgplugins.cpp )

kde4_automoc(${korgplugins_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(korgplugins ${korgplugins_SRCS})

target_link_libraries(korgplugins  ${QT_AND_KDECORE_LIBS} korganizer kio )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(timezone_SRCS timezone.cpp )

kde4_automoc(${timezone_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(timezone ${timezone_SRCS})

target_link_libraries(timezone  ${QT_AND_KDECORE_LIBS} korganizer kio )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(korganizer_eventviewer_LIB_SRCS koeventviewer.cpp urihandler.cpp koeventviewerdialog.cpp )

kde4_automoc(${korganizer_eventviewer_LIB_SRCS})

kde4_add_dcop_stubs(korganizer_eventviewer_LIB_SRCS ${CMAKE_SOURCE_DIR}/kmail/kmailIface.h korganizeriface.h ${CMAKE_SOURCE_DIR}/knode/knodeiface.h )

kde4_add_library(korganizer_eventviewer SHARED ${korganizer_eventviewer_LIB_SRCS})

target_link_libraries(korganizer_eventviewer  ${QT_AND_KDECORE_LIBS} kparts kcal )

set_target_properties(korganizer_eventviewer PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} korganizer_eventviewer )


########### next target ###############

set(korganizer_calendar_LIB_SRCS stdcalendar.cpp )

kde4_automoc(${korganizer_calendar_LIB_SRCS})

kde4_add_library(korganizer_calendar SHARED ${korganizer_calendar_LIB_SRCS})

target_link_libraries(korganizer_calendar  ${QT_AND_KDECORE_LIBS} kparts kdepim kcal )

set_target_properties(korganizer_calendar PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} korganizer_calendar )


########### next target ###############

set(korganizer_LIB_SRCS dummy.cpp )

kde4_automoc(${korganizer_LIB_SRCS})

file(WRITE dummy.cpp "//autogenerated file by cmake\n")
kde4_add_library(korganizer SHARED ${korganizer_LIB_SRCS})

target_link_libraries(korganizer  ${QT_AND_KDECORE_LIBS} )

set_target_properties(korganizer PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} korganizer )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES korganizer.desktop )
install_files( ${KCFG_INSTALL_DIR} FILES korganizer.kcfg )
install_files( ${DATA_INSTALL_DIR}/korganizer FILES korganizer_part.rc )
install_files( ${SERVICETYPES_INSTALL_DIR} FILES dcopcalendar.desktop )
install_files( ${DATA_INSTALL_DIR}/korganizer FILES tips )
install_files( ${SERVICES_INSTALL_DIR} FILES korganizer_configmain.desktop                      korganizer_configtime.desktop                      korganizer_configviews.desktop                      korganizer_configcolors.desktop                      korganizer_configfonts.desktop                      korganizer_configplugins.desktop                      korganizer_configgroupscheduling.desktop                      korganizer_configfreebusy.desktop                      korganizer_configdesignerfields.desktop                      webcal.protocol )
install_files( ${DATA_INSTALL_DIR}/korganizer FILES korganizerui.rc )
install_files( ${DATA_INSTALL_DIR}/kconf_update FILES korganizer.upd )

kde4_install_icons( ${ICON_INSTALL_DIR}  hicolor )

kde4_create_doxygen_docs( REFERENCES libkcal kdeui  )

kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = printing . korgac pixmaps sounds plugins interfaces
#
#INCLUDES = \
#  -I$(srcdir)/interfaces \
#  -I$(top_srcdir)/libemailfunctions \
#  -I$(top_srcdir)/libkpimidentities \
#  -I$(top_srcdir)/libkholidays \
#  -I$(top_srcdir)/mimelib \
#  -I$(top_srcdir)/libkleo \
#  -I$(top_srcdir)/korganizer/printing \
#  -I$(top_srcdir) \
#  -I$(top_builddir) \
#  $(all_includes)
#
#bin_PROGRAMS = korganizer
#
#korganizer_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#korganizer_LDADD   = libkorganizer.la
#korganizer_SOURCES = main.cpp korganizer.cpp koapp.cpp
#korganizer_COMPILE_FIRST = koprefs_base.h
#
#
#kde_module_LTLIBRARIES = kcm_korganizer.la libkorganizerpart.la
#
#kcm_korganizer_la_SOURCES = koprefsdialog.cpp
#kcm_korganizer_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
#kcm_korganizer_la_LIBADD = libkorganizer.la $(top_builddir)/libkdepim/libkdepim.la \
#  $(LIB_KDECORE)
#kcm_korganizer_la_COMPILE_FIRST = koprefs_base.h 	kogroupwareprefspage.h
#
#libkorganizerpart_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
#libkorganizerpart_la_LIBADD  = libkorganizer.la
#libkorganizerpart_la_SOURCES = korganizer_part.cpp
#libkorganizerpart_la_COMPILE_FIRST = publishdialog_base.h koprefs_base.h
#
#
#check_PROGRAMS = korgplugins timezone
#
#korgplugins_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#korgplugins_LDADD   = libkorganizer.la $(LIB_KSYCOCA)
#korgplugins_SOURCES = korgplugins.cpp
#korgplugins_COMPILE_FIRST = koprefs_base.h
#
#timezone_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#timezone_LDADD   = libkorganizer.la $(LIB_KSYCOCA)
#timezone_SOURCES = timezone.cpp
#timezone_COMPILE_FIRST = koprefs_base.h
#
#lib_LTLIBRARIES = libkorganizer_eventviewer.la \
#                  libkorganizer_calendar.la libkorganizer.la 
#
#libkorganizer_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -version-info 1:0
#libkorganizer_la_LIBADD  = $(LIB_KPARTS) $(LIB_KFILE) $(LIB_KNEWSTUFF) \
#	-lkdeprint -lkabc -lkutils \
#	$(top_builddir)/libkcal/libkcal.la \
#	$(top_builddir)/libkpimidentities/libkpimidentities.la \
#	$(top_builddir)/libkdepim/libkdepim.la \
#	libkorganizer_eventviewer.la \
#	libkorganizer_calendar.la \
#	printing/libkocorehelper.la \
#	printing/libkorg_stdprinting.la \
#	$(top_builddir)/kdgantt/libkdgantt.la \
#        $(top_builddir)/libemailfunctions/libemailfunctions.la \
#        $(top_builddir)/libkholidays/libkholidays.la
#libkorganizer_la_COMPILE_FIRST = \
#	kogroupwareprefspage.ui \
#	$(top_srcdir)/libkdepim/categoryselectdialog.h
#libkorganizer_la_SOURCES = komessagebox.cpp \
#	koagendaview.cpp koagenda.cpp koagendaitem.cpp \
#	datenavigator.cpp kdatenavigator.cpp datenavigatorcontainer.cpp \
#	datechecker.cpp \
#	komonthview.cpp \
#	searchdialog.cpp calendarview.cpp koviewmanager.cpp \
#	kodialogmanager.cpp \
#	archivedialog.cpp \
#	kohelper.cpp \
#	komailclient.cpp kotodoview.cpp kotodoviewitem.cpp kotodoviewquicksearch.cpp \
#	klistviewnewsearchline.cpp \
#	kolistview.cpp \
#	koincidenceeditor.cpp koeventeditor.cpp kotodoeditor.cpp kojournaleditor.cpp\
#	koprefs.cpp kowindowlist.cpp \
#	koeditorgeneral.cpp \
#	koeditorgeneralevent.cpp koeditorgeneraltodo.cpp koeditorgeneraljournal.cpp \
#	koeditordetails.cpp koeditoralarms_base.ui koeditoralarms.cpp \
#	koeditorrecurrence.cpp koeditorattachments.cpp \
#	kogroupwareprefspage.ui koeventpopupmenu.cpp koeditorfreebusy.cpp \
#	publishdialog_base.ui publishdialog.cpp \
#	koeventview.cpp \
#	korganizeriface.skel kcalendariface.skel \
#	filtereditdialog.cpp filteredit_base.ui \
#	kowhatsnextview.cpp kocounterdialog.cpp \
#	kojournalview.cpp journalentry.cpp \
#	kocore.cpp mailscheduler.cpp \
#	kodaymatrix.cpp docprefs.cpp statusdialog.cpp\
#	koglobals.cpp konewstuff.cpp \
#	alarmclient.cpp \
#	actionmanager.cpp resourceview.cpp \
#	navigatorbar.cpp kogroupware.cpp \
#	history.cpp \
#	koprefs_base.kcfgc \
#	aboutdata.cpp \
#	importdialog.cpp \
#	korganizerifaceimpl.cpp \
#	freebusymanager.cpp freebusyurldialog.cpp \
#	eventarchiver.cpp koidentitymanager.cpp \
#	exportwebdialog.cpp kocorehelper.cpp incidencechanger.cpp \
#  template_management_dialog_base.ui templatemanagementdialog.cpp
#
#
#libkorganizer_eventviewer_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) \
#                                       -no-undefined -version-info 1:0
#libkorganizer_eventviewer_la_LIBADD  = $(LIB_KPARTS) \
#                                       $(top_builddir)/libkcal/libkcal.la
#libkorganizer_eventviewer_la_SOURCES = koeventviewer.cpp urihandler.cpp \
#                                       kmailIface.stub koeventviewerdialog.cpp \
#                                       korganizeriface.stub knodeiface.stub
#
#libkorganizer_calendar_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) \
#                                    -no-undefined -version-info 1:0
#libkorganizer_calendar_la_LIBADD  = $(LIB_KPARTS) \
#                                    $(top_builddir)/libkdepim/libkdepim.la \
#                                    $(top_builddir)/libkcal/libkcal.la 
#libkorganizer_calendar_la_SOURCES = stdcalendar.cpp
#
#
#kmailIface_DIR = $(top_srcdir)/kmail
#kmailIface_DCOPIDLNG = true
#
#knodeiface_DIR = $(top_srcdir)/knode
#knodeiface_DCOPIDLNG = true
#
#METASOURCES = AUTO
#
#noinst_HEADERS = komonthview.h \
#	koagendaview.h koagenda.h \
#	koagendaitem.h \
#	kdatenavigator.h \
#	calendarview.h \
#	version.h \
#	searchdialog.h \
#	archivedialog.h \
#	komailclient.h kotodoview.h kotodoviewquicksearch.h \
#	klistviewnewsearchline.h \
#	kolistview.h \
#	koapp.h korganizer_part.h korganizer.h \
#	koprefsdialog.h koeventeditor.h \
#	kotodoeditor.h koprefs.h koeventviewer.h \
#	kowindowlist.h kocounterdialog.h \
#	koeditorgeneralevent.h koeditorgeneraltodo.h koeditordetails.h \
#	koeditorrecurrence.h koeventviewerdialog.h koeventpopupmenu.h \
#	publishdialog.h koeditorfreebusy.h \
#	koeventview.h statusdialog.h customlistviewitem.h \
#	alarmclient.h kohelper.h \
#	navigatorbar.h kogroupware.h \
#	korganizerifaceimpl.h \
#	exportwebdialog.h templatemanagementdialog.h
#
#tip_DATA = tips
#tipdir = $(kde_datadir)/korganizer
#
#kde_bin_SCRIPTS = ical2vcal
#
#rcdir = $(kde_datadir)/korganizer
#rc_DATA = korganizerui.rc
#
#partdir   = $(kde_datadir)/korganizer
#part_DATA = korganizer_part.rc
#
#kde_servicetypes_DATA = dcopcalendar.desktop
#
#KDE_ICON = AUTO
#
#META_INCLUDES = $(srcdir)/interfaces/korganizer
#
#messages: rc.cpp
#	$(PREPARETIPS) > tips.cpp
#	$(EXTRACTRC) `find . -name "*.rc" -o -name "*.ui" -o -name "*.kcfg"` >> rc.cpp
#	$(XGETTEXT) `find . -name "*.cpp" -o -name "*.h"` -o $(podir)/korganizer.pot
#	rm -f tips.cpp
#
#xdg_apps_DATA = korganizer.desktop
#
#kde_kcfg_DATA = korganizer.kcfg
#
#kde_services_DATA = korganizer_configmain.desktop \
#                    korganizer_configtime.desktop \
#                    korganizer_configviews.desktop \
#                    korganizer_configcolors.desktop \
#                    korganizer_configfonts.desktop \
#                    korganizer_configplugins.desktop \
#                    korganizer_configgroupscheduling.desktop \
#                    korganizer_configfreebusy.desktop \
#                    korganizer_configdesignerfields.desktop \
#                    webcal.protocol
#
#update_DATA = korganizer.upd
#updatedir = $(kde_datadir)/kconf_update
#
#DOXYGEN_REFERENCES=libkcal kdeui
#include $(top_srcdir)/admin/Doxyfile.am
#
#install-data-local: 
#	$(mkinstalldirs) $(DESTDIR)$(kde_servicesdir)
#	$(mkinstalldirs) $(DESTDIR)$(kde_servicesdir)/korganizer
#	$(INSTALL_DATA) $(srcdir)/uninstall.desktop $(DESTDIR)$(kde_servicesdir)/korganizer_configgroupautomation.desktop
#	$(INSTALL_DATA) $(srcdir)/uninstall.desktop $(DESTDIR)$(kde_servicesdir)/korganizer/webexport.desktop
#	$(INSTALL_DATA) $(srcdir)/uninstall.desktop $(DESTDIR)$(kde_servicesdir)/korganizer/holidays.desktop
