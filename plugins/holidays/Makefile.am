# $Id$

SUBDIRS=holidays

METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/korganizer/interfaces $(all_includes)

kde_module_LTLIBRARIES = libkorg_holidays.la

libkorg_holidays_la_SOURCES = holidays.cpp configdialog.cpp scanholiday.c parseholiday.c

libkorg_holidays_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) 
libkorg_holidays_la_LIBADD = $(LIB_KDEUI)

noinst_HEADERS = holidays.h configdialog.h

servicedir  = $(kde_servicesdir)/korganizer
service_DATA = holidays.desktop

parser:
	cd $(srcdir) && $(YACC) -p kcal -d -o parseholiday.c parseholiday.y
	cd $(srcdir) && $(YACC) -p kcal -d -o parseholiday.c parseholiday.y
	cd $(srcdir) && $(LEX) -Pkcal -B -oscanholiday.c scanholiday.lex
