INCLUDES= -I$(top_srcdir) -I$(top_srcdir)/korganizer $(all_includes)

bin_PROGRAMS = korgac

korgac_LDFLAGS = $(all_libraries) $(KDE_RPATH)
korgac_LDADD   = $(LIB_KDEUI) \
                 $(top_builddir)/korganizer/libkorganizer_eventviewer.la
korgac_SOURCES = korgacmain.cpp alarmdialog.cpp \
	         alarmdockwindow.cpp koalarmclient.cpp \
                 alarmclientiface.skel

check_PROGRAMS = testalarmdlg

testalarmdlg_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testalarmdlg_LDADD = $(LIB_KDEUI) $(top_builddir)/libkcal/libkcal.la \
                     $(top_builddir)/korganizer/libkorganizer_eventviewer.la
testalarmdlg_SOURCES = testalarmdlg.cpp alarmdialog.cpp

noinst_HEADERS = alarmclientiface.h alarmdialog.h \
                 alarmdockwindow.h koalarmclient.h

METASOURCES = AUTO

SUBDIRS = pixmaps
 
autostart_DATA = korgac.desktop
autostartdir = $(datadir)/autostart

include ../../admin/Doxyfile.am
