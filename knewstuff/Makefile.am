INCLUDES = $(all_includes)

METASOURCES = AUTO

lib_LTLIBRARIES = libknewstuff.la

libknewstuff_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -version-info 1:0
libknewstuff_la_LIBADD  = $(LIB_KIO)
libknewstuff_la_SOURCES = engine.cpp entry.cpp downloaddialog.cpp \
  uploaddialog.cpp providerdialog.cpp provider.cpp knewstuff.cpp

check_PROGRAMS = testnewstuff

testnewstuff_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testnewstuff_LDADD   = libknewstuff.la
testnewstuff_SOURCES = testnewstuff.cpp
