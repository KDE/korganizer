METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/korganizer/interfaces \
           -I$(top_srcdir)/libkdepim \
           -I$(top_builddir)/libkdepim \
           -I$(top_srcdir)/korganizer \
					 -I$(top_srcdir) \
					 -I$(top_builddir) \
					 $(all_includes) 


lib_LTLIBRARIES = libkocorehelper.la libkorg_stdprinting.la


libkocorehelper_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) \
                             -no-undefined -version-info 1:0
libkocorehelper_la_LIBADD  = $(LIB_KDEUI)
libkocorehelper_la_SOURCES = cellitem.cpp

libkorg_stdprinting_la_SOURCES = calprinter.cpp calprinthelper.cpp calprintpluginbase.cpp \
        calprintdefaultplugins.cpp \
        calprintdayconfig_base.ui calprintmonthconfig_base.ui \
        calprinttodoconfig_base.ui calprintweekconfig_base.ui
libkorg_stdprinting_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) \
                                 -no-undefined -version-info 1:0
libkorg_stdprinting_la_LIBADD = $(LIB_KDEUI) libkocorehelper.la \
                                $(top_builddir)/libkdepim/libkdepim.la \
                                $(top_builddir)/libkcal/libkcal.la \
                                -lkdeprint 


calprintdir = $(includedir)/korganizer
calprint_HEADERS = calprinthelper.h calprintpluginbase.h calprinter.h
noinst_HEADERS = cellitem.h

