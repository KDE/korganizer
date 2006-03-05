kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/korganizer/interfaces ${CMAKE_SOURCE_DIR}/korganizer/printing ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(korg_listprint_PART_SRCS listprint.cpp )

kde4_automoc(${korg_listprint_PART_SRCS})

kde4_add_ui_files(korg_listprint_PART_SRCS calprintlistconfig_base.ui )

kde4_add_plugin(korg_listprint WITH_PREFIX ${korg_listprint_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} korg_listprint )

target_link_libraries(korg_listprint  ${QT_AND_KDECORE_LIBS} kdeui korg_stdprinting )

install_targets(${LIB_INSTALL_DIR}/kde4 korg_listprint )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR}/korganizer FILES listprint.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/korganizer/interfaces -I$(top_srcdir)/korganizer/printing -I$(top_srcdir) $(all_includes) 
#
#kde_module_LTLIBRARIES = libkorg_listprint.la
#
#libkorg_listprint_la_SOURCES = calprintlistconfig_base.ui  listprint.cpp
#libkorg_listprint_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) 
#libkorg_listprint_la_LIBADD = $(LIB_KDECORE) $(LIB_KDEUI) $(top_builddir)/korganizer/printing/libkorg_stdprinting.la
#
#noinst_HEADERS = 
#
#servicedir  = $(kde_servicesdir)/korganizer
#service_DATA = listprint.desktop
#
#METASOURCES = AUTO
