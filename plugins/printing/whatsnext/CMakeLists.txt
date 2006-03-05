kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/korganizer/interfaces ${CMAKE_SOURCE_DIR}/korganizer/printing ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(korg_whatsnextprint_PART_SRCS whatsnextprint.cpp )

kde4_automoc(${korg_whatsnextprint_PART_SRCS})

kde4_add_ui_files(korg_whatsnextprint_PART_SRCS calprintwhatsnextconfig_base.ui )

kde4_add_plugin(korg_whatsnextprint WITH_PREFIX ${korg_whatsnextprint_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} korg_whatsnextprint )

target_link_libraries(korg_whatsnextprint  ${QT_AND_KDECORE_LIBS} kdeui korg_stdprinting )

install_targets(${LIB_INSTALL_DIR}/kde4 korg_whatsnextprint )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR}/korganizer FILES whatsnextprint.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/korganizer/interfaces -I$(top_srcdir)/korganizer/printing -I$(top_srcdir) $(all_includes) 
#
#kde_module_LTLIBRARIES = libkorg_whatsnextprint.la
#
#libkorg_whatsnextprint_la_SOURCES = calprintwhatsnextconfig_base.ui  whatsnextprint.cpp
#libkorg_whatsnextprint_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) 
#libkorg_whatsnextprint_la_LIBADD = $(LIB_KDECORE) $(LIB_KDEUI) $(top_builddir)/korganizer/printing/libkorg_stdprinting.la
#
#noinst_HEADERS = 
#
#servicedir  = $(kde_servicesdir)/korganizer
#service_DATA = whatsnextprint.desktop
#
#METASOURCES = AUTO
