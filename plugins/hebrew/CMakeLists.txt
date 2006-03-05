kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/korganizer/interfaces ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(korg_hebrew_PART_SRCS 
   hebrew.cpp 
   configdialog.cpp 
   holiday.cpp 
   parsha.cpp 
   converter.cpp )

kde4_automoc(${korg_hebrew_PART_SRCS})

kde4_add_plugin(korg_hebrew WITH_PREFIX ${korg_hebrew_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} korg_hebrew )

target_link_libraries(korg_hebrew  ${QT_AND_KDECORE_LIBS} kdeui )

install_targets(${LIB_INSTALL_DIR}/kde4 korg_hebrew )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR}/korganizer FILES hebrew.desktop )

kde4_footer()



#original Makefile.am contents follow:

## $Id: Makefile.am 242335 2003-08-12 19:33:21Z jsinger $
#
#METASOURCES = AUTO
#
#INCLUDES = -I$(top_srcdir)/korganizer/interfaces $(all_includes)
#
#kde_module_LTLIBRARIES = libkorg_hebrew.la
#
#libkorg_hebrew_la_SOURCES = hebrew.cpp configdialog.cpp holiday.cpp parsha.cpp converter.cpp
#libkorg_hebrew_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) 
#libkorg_hebrew_la_LIBADD = $(LIB_KDECORE) $(LIB_KDEUI)
#
#noinst_HEADERS = hebrew.h configdialog.h parsha.h converter.h holiday.h
#
#servicedir  = $(kde_servicesdir)/korganizer
#service_DATA = hebrew.desktop
