INCLUDES = -I$(top_srcdir)/korganizer/interfaces -I$(top_srcdir)/korganizer $(all_includes) 

kde_module_LTLIBRARIES = libkorg_datenums.la

libkorg_datenums_la_SOURCES = configdialog.cpp datenums.cpp
libkorg_datenums_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) 
libkorg_datenums_la_LIBADD = $(top_builddir)/korganizer/libkorganizer.la $(LIB_KDEUI)

noinst_HEADERS = datenums.h

servicedir  = $(kde_servicesdir)/korganizer
service_DATA = datenums.desktop

METASOURCES = AUTO
