# $Id$

SUBDIRS=

METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/korganizer/interfaces $(all_includes)

kde_module_LTLIBRARIES = libkorg_exchange.la

libkorg_exchange_la_SOURCES = exchangeprogress.cpp exchange.cpp exchangedialog.cpp exchangeconfig.cpp

libkorg_exchange_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries)
libkorg_exchange_la_LIBADD = $(top_builddir)/korganizer/libkorganizer.la $(LIB_KDEUI) $(LIB_KIO) $(LIB_KPARTS) -lmimelib

noinst_HEADERS = exchange.h exchangedialog.h exchangeprogress.h exchangeconfig.h

servicedir  = $(kde_servicesdir)/korganizer
service_DATA = exchange.desktop

rcdir = $(kde_datadir)/korganizer/plugins
rc_DATA = exchangeui.rc

