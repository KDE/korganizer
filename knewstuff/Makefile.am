INCLUDES = -I$(srcdir)/.. $(all_includes)

METASOURCES = AUTO

lib_LTLIBRARIES = libknewstuff.la

libknewstuff_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -version-info 1:0
libknewstuff_la_LIBADD  = $(LIB_KIO)
libknewstuff_la_SOURCES = engine.cpp entry.cpp downloaddialog.cpp \
  uploaddialog.cpp providerdialog.cpp provider.cpp knewstuff.cpp \
  knewstuffgeneric.cpp

bin_PROGRAMS = ghns khotnewstuff

ghns_LDFLAGS = $(all_libraries)
ghns_LDADD   = libknewstuff.la
ghns_SOURCES = ghns.cpp

khotnewstuff_LDFLAGS = $(all_libraries)
khotnewstuff_LDADD = libknewstuff.la
khotnewstuff_SOURCES = khotnewstuff.cpp

EXTRA_PROGRAMS = testnewstuff

testnewstuff_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testnewstuff_LDADD   = libknewstuff.la
testnewstuff_SOURCES = testnewstuff.cpp

knewstuffdir = $(kde_datadir)/knewstuff
knewstuff_DATA = types

kde_conf_DATA = khotnewstuffrc

knsdir = $(includedir)/knewstuff
kns_HEADERS = downloaddialog.h engine.h entry.h ghns.h knewstuffgeneric.h knewstuff.h providerdialog.h provider.h testnewstuff.h uploaddialog.h

KDE_ICON=AUTO

