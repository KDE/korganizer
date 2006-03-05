kde4_header()

add_subdirectory( pixmaps ) 

include_directories( ${CMAKE_SOURCE_DIR}/korganizer ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(korgac_SRCS 
   korgacmain.cpp 
   alarmdialog.cpp 
   alarmdockwindow.cpp 
   koalarmclient.cpp )

kde4_automoc(${korgac_SRCS})

kde4_add_dcop_skels(korgac_SRCS alarmclientiface.h )

kde4_add_executable(korgac ${korgac_SRCS})

target_link_libraries(korgac  ${QT_AND_KDECORE_LIBS} kdeui korganizer_eventviewer )

install_targets(/bin korgac )


########### next target ###############

set(testalarmdlg_SRCS testalarmdlg.cpp alarmdialog.cpp )

kde4_automoc(${testalarmdlg_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(testalarmdlg ${testalarmdlg_SRCS})

target_link_libraries(testalarmdlg  ${QT_AND_KDECORE_LIBS} kdeui kcal korganizer_eventviewer )

endif(KDE4_BUILD_TESTS)

########### install files ###############

install_files( /share/autostart FILES korgac.desktop )

kde4_footer()



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
