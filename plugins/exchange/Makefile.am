# $Id$

SUBDIRS=

METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/korganizer/interfaces \
   -I$(top_srcdir)/libkpimexchange/core -I$(top_builddir)/libkpimexchange/core \
	-I$(top_srcdir) \
    $(all_includes)
 
kde_module_LTLIBRARIES = libkorg_exchange.la

libkorg_exchange_la_SOURCES =  exchange.cpp exchangedialog.cpp exchangeconfig.cpp
libkorg_exchange_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries)
libkorg_exchange_la_LIBADD = $(top_builddir)/korganizer/libkorganizer.la \
			     $(top_builddir)/libkpimexchange/libkpimexchange.la \
			     $(LIB_KDEUI) $(LIB_KPARTS)

noinst_HEADERS = exchange.h exchangedialog.h exchangeconfig.h

servicedir  = $(kde_servicesdir)/korganizer
service_DATA = exchange.desktop

rcdir = $(kde_datadir)/korganizer/plugins
rc_DATA = exchangeui.rc

