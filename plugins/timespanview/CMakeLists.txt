
include_directories( ${CMAKE_SOURCE_DIR}/korganizer/interfaces ${CMAKE_SOURCE_DIR}/korganizer  )


########### next target ###############

set(korg_timespanview_PART_SRCS 
   lineview.cpp 
   timeline.cpp 
   timespanview.cpp 
   timespanwidget.cpp 
   kotimespanview.cpp )


kde4_add_plugin(korg_timespanview WITH_PREFIX ${korg_timespanview_PART_SRCS})



target_link_libraries(korg_timespanview ${KDE4_KPARTS_LIBS} korganizer)

install(TARGETS korg_timespanview  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES timespanview.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/korganizer)
install( FILES timespanviewui.rc  DESTINATION ${DATA_INSTALL_DIR}/korganizer/plugins)




#original Makefile.am contents follow:

## $Id: Makefile.am 440080 2005-07-29 19:03:08Z kainhofe $
#
#METASOURCES = AUTO
#
#INCLUDES = -I$(top_srcdir)/korganizer/interfaces \
#  -I$(top_srcdir) -I$(top_srcdir)/korganizer $(all_includes)
#
#kde_module_LTLIBRARIES = libkorg_timespanview.la
#
#libkorg_timespanview_la_SOURCES = lineview.cpp timeline.cpp timespanview.cpp timespanwidget.cpp kotimespanview.cpp
#libkorg_timespanview_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) 
#libkorg_timespanview_la_LIBADD = $(top_builddir)/korganizer/libkorganizer.la $(LIB_KPARTS)
#
#noinst_HEADERS = timespanview.h timespanwidget.h kotimespanview.h
#
#servicedir  = $(kde_servicesdir)/korganizer
#service_DATA = timespanview.desktop
#
#rcdir = $(kde_datadir)/korganizer/plugins
#rc_DATA = timespanviewui.rc
