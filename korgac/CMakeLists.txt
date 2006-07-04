
add_subdirectory( pixmaps ) 

include_directories( ${CMAKE_SOURCE_DIR}/korganizer  )


########### next target ###############

set(korgac_SRCS 
   korgacmain.cpp 
   alarmdialog.cpp 
   alarmdockwindow.cpp 
   koalarmclient.cpp )

kde4_automoc(${korgac_SRCS})

qt4_add_dbus_adaptor(korgac_SRCS org.kde.korganizer.KOrgac.xml koalarmclient.h KOAlarmClient )


kde4_add_executable(korgac ${korgac_SRCS})

target_link_libraries(korgac  ${KDE4_KDEUI_LIBS} korganizer_eventviewer phononcore)

install(TARGETS korgac  DESTINATION ${BIN_INSTALL_DIR} )


########### next target ###############

set(testalarmdlg_SRCS testalarmdlg.cpp alarmdialog.cpp )

kde4_automoc(${testalarmdlg_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(testalarmdlg ${testalarmdlg_SRCS})

target_link_libraries(testalarmdlg  ${KDE4_KDEUI_LIBS} kcal korganizer_eventviewer phononcore)

endif(KDE4_BUILD_TESTS)

########### install files ###############

install( FILES korgac.desktop  DESTINATION /share/autostart)




#original Makefile.am contents follow:

#INCLUDES= -I$(top_srcdir) -I$(top_srcdir)/korganizer $(all_includes)
#
#bin_PROGRAMS = korgac
#
#korgac_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#korgac_LDADD   = $(LIB_KDEUI) \
#                 $(top_builddir)/korganizer/libkorganizer_eventviewer.la
#korgac_SOURCES = korgacmain.cpp alarmdialog.cpp \
#	         alarmdockwindow.cpp koalarmclient.cpp \
#                 alarmclientiface.skel
#
#check_PROGRAMS = testalarmdlg
#
#testalarmdlg_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#testalarmdlg_LDADD = $(LIB_KDEUI) $(top_builddir)/libkcal/libkcal.la \
#                     $(top_builddir)/korganizer/libkorganizer_eventviewer.la
#testalarmdlg_SOURCES = testalarmdlg.cpp alarmdialog.cpp
#
#noinst_HEADERS = alarmclientiface.h alarmdialog.h \
#                 alarmdockwindow.h koalarmclient.h
#
#METASOURCES = AUTO
#
#SUBDIRS = pixmaps
# 
#autostart_DATA = korgac.desktop
#autostartdir = $(datadir)/autostart
#
