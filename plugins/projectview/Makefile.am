# $Id$

METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/korganizer/interfaces \
  -I$(top_srcdir) -I$(top_srcdir)/kgantt/kgantt $(all_includes)

kde_module_LTLIBRARIES = libkorg_projectview.la

libkorg_projectview_la_SOURCES = projectview.cpp koprojectview.cpp
libkorg_projectview_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) 
libkorg_projectview_la_LIBADD = $(top_builddir)/kgantt/kgantt/libkgantt.la $(top_builddir)/korganizer/libkorganizer.la $(LIB_KPARTS)

noinst_HEADERS = projectview.h koprojectview.h

servicedir  = $(kde_servicesdir)/korganizer
service_DATA = projectview.desktop

rcdir = $(kde_datadir)/korganizer/plugins
rc_DATA = projectviewui.rc
