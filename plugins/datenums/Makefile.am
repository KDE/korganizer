# $Id$

INCLUDES = -I$(top_srcdir)/korganizer/interfaces $(all_includes)

kde_module_LTLIBRARIES = libkorg_datenums.la

libkorg_datenums_la_SOURCES = datenums.cpp
libkorg_datenums_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) 
libkorg_datenums_la_LIBADD = $(LIB_KDECORE)

noinst_HEADERS = datenums.h

servicedir  = $(kde_servicesdir)/korganizer
service_DATA = datenums.desktop
