INCLUDES= -I$(top_srcdir) -I$(top_srcdir)/kalarmd -I$(top_builddir)/kalarmd \
  $(all_includes)

bin_PROGRAMS = korgac simplealarmdaemon

korgac_LDFLAGS = $(all_libraries) $(KDE_RPATH)
korgac_LDADD = $(LIB_KDEUI) $(top_builddir)/libkcal/libkcal.la
korgac_SOURCES = korgacmain.cpp alarmdialog.cpp \
	alarmdockwindow.cpp koeventviewer.cpp koalarmclient.cpp \
        alarmguiiface.skel

simplealarmdaemon_LDFLAGS = $(all_libraries) $(KDE_RPATH)
simplealarmdaemon_LDADD = $(LIB_KDEUI) $(top_builddir)/libkcal/libkcal.la
simplealarmdaemon_SOURCES = simplealarmdaemon.cpp alarmdialog.cpp koeventviewer.cpp  \
  simplealarmdaemonmain.cpp

check_PROGRAMS = testalarmdlg

testalarmdlg_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testalarmdlg_LDADD = $(LIB_KDEUI) $(top_builddir)/libkcal/libkcal.la \
	$(top_builddir)/kalarmd/libkalarmd.la
testalarmdlg_SOURCES = testalarmdlg.cpp alarmdialog.cpp koeventviewer.cpp

noinst_HEADERS = alarmguiiface.h alarmdialog.h \
	alarmdockwindow.h koeventviewer.h koalarmclient.h \
	simplealarmdaemon.h

alarmguiiface.h: $(top_srcdir)/kalarmd/alarmguiiface.h
	cp $(top_srcdir)/kalarmd/alarmguiiface.h .

METASOURCES = AUTO

SUBDIRS = pixmaps
 
autostart_DATA = korgac.desktop
autostartdir = $(prefix)/share/autostart

KDE_ICON = AUTO

include ../../admin/Doxyfile.am
