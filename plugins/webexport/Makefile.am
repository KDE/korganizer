# $Id$

METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/korganizer/interfaces -I$(top_srcdir)/korganizer -I$(top_srcdir)/libkcal $(all_includes)

kde_module_LTLIBRARIES = libkorg_webexport.la

libkorg_webexport_la_SOURCES = webexport.cpp
libkorg_webexport_la_LDFLAGS = -module -L$(top_builddir)/korganizer $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) 
libkorg_webexport_la_LIBADD = -lkorganizer $(LIB_KPARTS)

noinst_HEADERS = webexport.h

servicedir  = $(kde_servicesdir)/korganizer
service_DATA = webexport.desktop

rcdir = $(kde_datadir)/korganizer/plugins
rc_DATA = webexportui.rc
