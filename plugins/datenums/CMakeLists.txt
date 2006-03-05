kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/korganizer/interfaces ${CMAKE_SOURCE_DIR}/korganizer ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(korg_datenums_PART_SRCS configdialog.cpp datenums.cpp )

kde4_automoc(${korg_datenums_PART_SRCS})

kde4_add_plugin(korg_datenums WITH_PREFIX ${korg_datenums_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} korg_datenums )

target_link_libraries(korg_datenums  ${QT_AND_KDECORE_LIBS} kdeui korganizer )

install_targets(${LIB_INSTALL_DIR}/kde4 korg_datenums )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR}/korganizer FILES datenums.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/korganizer/interfaces -I$(top_srcdir)/korganizer $(all_includes) 
#
#kde_module_LTLIBRARIES = libkorg_datenums.la
#
#libkorg_datenums_la_SOURCES = configdialog.cpp datenums.cpp
#libkorg_datenums_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) 
#libkorg_datenums_la_LIBADD = $(LIB_KDECORE) $(LIB_KDEUI) $(top_builddir)/korganizer/libkorganizer.la
#
#noinst_HEADERS = datenums.h
#
#servicedir  = $(kde_servicesdir)/korganizer
#service_DATA = datenums.desktop
#
#METASOURCES = AUTO
