# $Id$

METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/korganizer/interfaces \
           -I$(top_srcdir)/libkholidays \
           $(all_includes)

kde_module_LTLIBRARIES = libkorg_holidays.la

libkorg_holidays_la_SOURCES = holidaySettings.kcfgc holidays.cpp configdialog.cpp

libkorg_holidays_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) 
libkorg_holidays_la_LIBADD = $(LIB_KDEUI) \
              $(top_builddir)/libkholidays/libkholidays.la

noinst_HEADERS = holidays.h configdialog.h

servicedir  = $(kde_servicesdir)/korganizer
service_DATA = holidays.desktop
