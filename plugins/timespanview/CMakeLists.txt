kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/korganizer/interfaces ${CMAKE_SOURCE_DIR}/korganizer ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(korg_timespanview_PART_SRCS 
   lineview.cpp 
   timeline.cpp 
   timespanview.cpp 
   timespanwidget.cpp 
   kotimespanview.cpp )

kde4_automoc(${korg_timespanview_PART_SRCS})

kde4_add_plugin(korg_timespanview WITH_PREFIX ${korg_timespanview_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} korg_timespanview )

target_link_libraries(korg_timespanview  ${QT_AND_KDECORE_LIBS} korganizer kparts )

install_targets(${LIB_INSTALL_DIR}/kde4 korg_timespanview )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR}/korganizer FILES timespanview.desktop )
install_files( ${DATA_INSTALL_DIR}/korganizer/plugins FILES timespanviewui.rc )

kde4_footer()



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
