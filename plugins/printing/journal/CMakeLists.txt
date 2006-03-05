kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/korganizer/interfaces ${CMAKE_SOURCE_DIR}/korganizer/printing ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(korg_journalprint_PART_SRCS journalprint.cpp )

kde4_automoc(${korg_journalprint_PART_SRCS})

kde4_add_ui_files(korg_journalprint_PART_SRCS calprintjournalconfig_base.ui )

kde4_add_plugin(korg_journalprint WITH_PREFIX ${korg_journalprint_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} korg_journalprint )

target_link_libraries(korg_journalprint  ${QT_AND_KDECORE_LIBS} kdeui korg_stdprinting )

install_targets(${LIB_INSTALL_DIR}/kde4 korg_journalprint )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR}/korganizer FILES journalprint.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/korganizer/interfaces -I$(top_srcdir)/korganizer/printing -I$(top_srcdir) $(all_includes) 
#
#kde_module_LTLIBRARIES = libkorg_journalprint.la
#
#libkorg_journalprint_la_SOURCES =calprintjournalconfig_base.ui  journalprint.cpp
#libkorg_journalprint_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) 
#libkorg_journalprint_la_LIBADD = $(LIB_KDECORE) $(LIB_KDEUI) $(top_builddir)/korganizer/printing/libkorg_stdprinting.la
#
#noinst_HEADERS = 
#
#servicedir  = $(kde_servicesdir)/korganizer
#service_DATA = journalprint.desktop
#
#METASOURCES = AUTO
