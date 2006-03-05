kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/korganizer/interfaces ${CMAKE_SOURCE_DIR}/kgantt/kgantt ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(korg_projectview_PART_SRCS projectview.cpp koprojectview.cpp )

kde4_automoc(${korg_projectview_PART_SRCS})

kde4_add_plugin(korg_projectview WITH_PREFIX ${korg_projectview_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} korg_projectview )

target_link_libraries(korg_projectview  ${QT_AND_KDECORE_LIBS} kgantt korganizer kparts )

install_targets(${LIB_INSTALL_DIR}/kde4 korg_projectview )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR}/korganizer FILES projectview.desktop )
install_files( ${DATA_INSTALL_DIR}/korganizer/plugins FILES projectviewui.rc )

kde4_footer()



#original Makefile.am contents follow:

## $Id: Makefile.am 376956 2005-01-09 18:57:14Z kainhofe $
#
#METASOURCES = AUTO
#
#INCLUDES = -I$(top_srcdir)/korganizer/interfaces \
#  -I$(top_srcdir) -I$(top_srcdir)/kgantt/kgantt $(all_includes)
#
#kde_module_LTLIBRARIES = libkorg_projectview.la
#
#libkorg_projectview_la_SOURCES = projectview.cpp koprojectview.cpp
#libkorg_projectview_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) 
#libkorg_projectview_la_LIBADD = $(top_builddir)/kgantt/kgantt/libkgantt.la $(top_builddir)/korganizer/libkorganizer.la $(LIB_KPARTS)
#
#noinst_HEADERS = projectview.h koprojectview.h
#
#servicedir  = $(kde_servicesdir)/korganizer
#service_DATA = projectview.desktop
#
#rcdir = $(kde_datadir)/korganizer/plugins
#rc_DATA = projectviewui.rc
