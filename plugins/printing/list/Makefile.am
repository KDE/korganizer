INCLUDES = -I$(top_srcdir)/korganizer/interfaces -I$(top_srcdir)/korganizer/printing -I$(top_srcdir) $(all_includes) 

kde_module_LTLIBRARIES = libkorg_listprint.la

libkorg_listprint_la_SOURCES = calprintlistconfig_base.ui  listprint.cpp
libkorg_listprint_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) 
libkorg_listprint_la_LIBADD = $(LIB_KDECORE) $(LIB_KDEUI) $(top_builddir)/korganizer/printing/libkorg_stdprinting.la

noinst_HEADERS = 

servicedir  = $(kde_servicesdir)/korganizer
service_DATA = listprint.desktop

METASOURCES = AUTO
