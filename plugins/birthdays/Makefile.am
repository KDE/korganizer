METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/korganizer/interfaces -I$(top_srcdir)/korganizer \
  -I$(top_srcdir) -I $(top_srcdir)/libkdepim -I$(top_builddir)/korganizer \
  $(all_includes)

kde_module_LTLIBRARIES = libkorg_birthdays.la

libkorg_birthdays_la_SOURCES = birthdays.cpp birthdaysdialog.cpp 
libkorg_birthdays_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries)
libkorg_birthdays_la_LIBADD = $(top_builddir)/korganizer/libkorganizer.la $(LIB_KPARTS)
libkorg_birthdays_la_COMPILE_FIRST = $(top_srcdir)/korganizer/koprefs.h

noinst_HEADERS = birthdays.h birthdaysdialog.h 

servicedir  = $(kde_servicesdir)/korganizer
service_DATA = birthdays.desktop

rcdir = $(kde_datadir)/korganizer/plugins
rc_DATA = birthdaysui.rc
