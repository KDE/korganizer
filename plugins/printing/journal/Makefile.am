INCLUDES = -I$(top_srcdir)/korganizer/interfaces -I$(top_srcdir)/korganizer/printing -I$(top_builddir)/libkdepim -I$(top_srcdir) $(all_includes) 

kde_module_LTLIBRARIES = libkorg_journalprint.la

libkorg_journalprint_la_SOURCES =calprintjournalconfig_base.ui  journalprint.cpp
libkorg_journalprint_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) 
libkorg_journalprint_la_LIBADD = $(LIB_KDECORE) $(LIB_KDEUI) $(top_builddir)/korganizer/printing/libkorg_stdprinting.la

noinst_HEADERS = 

servicedir  = $(kde_servicesdir)/korganizer
service_DATA = journalprint.desktop

METASOURCES = AUTO
