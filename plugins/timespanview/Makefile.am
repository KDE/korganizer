# $Id$

METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/korganizer/interfaces \
  -I$(top_srcdir) -I$(top_srcdir)/korganizer $(all_includes)

kde_module_LTLIBRARIES = libkorg_timespanview.la

libkorg_timespanview_la_SOURCES = timespanview.cpp timespanwidget.cpp kotimespanview.cpp
libkorg_timespanview_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) 
libkorg_timespanview_la_LIBADD = $(top_builddir)/korganizer/libkorganizer.la $(LIB_KPARTS)

noinst_HEADERS = timespanview.h timespanwidget.h kotimespanview.h

servicedir  = $(kde_servicesdir)/korganizer
service_DATA = timespanview.desktop

rcdir = $(kde_datadir)/korganizer/plugins
rc_DATA = timespanviewui.rc
