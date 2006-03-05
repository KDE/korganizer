kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/korganizer/interfaces ${CMAKE_SOURCE_DIR}/korganizer ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kocorehelper_LIB_SRCS cellitem.cpp )

kde4_automoc(${kocorehelper_LIB_SRCS})

kde4_add_library(kocorehelper SHARED ${kocorehelper_LIB_SRCS})

target_link_libraries(kocorehelper  ${QT_AND_KDECORE_LIBS} kdeui )

set_target_properties(kocorehelper PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} kocorehelper )


########### next target ###############

set(korg_stdprinting_LIB_SRCS 
   calprinter.cpp 
   calprinthelper.cpp 
   calprintpluginbase.cpp 
   calprintdefaultplugins.cpp )

kde4_automoc(${korg_stdprinting_LIB_SRCS})

kde4_add_ui_files(korg_stdprinting_LIB_SRCS 
   calprintdayconfig_base.ui 
   calprintmonthconfig_base.ui 
   calprinttodoconfig_base.ui 
   calprintweekconfig_base.ui )

kde4_add_library(korg_stdprinting SHARED ${korg_stdprinting_LIB_SRCS})

target_link_libraries(korg_stdprinting  ${QT_AND_KDECORE_LIBS} kdeui kocorehelper kdepim kcal kdeprint )

set_target_properties(korg_stdprinting PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} korg_stdprinting )


########### install files ###############

install_files( /include/korganizer FILES  )

kde4_footer()



#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#INCLUDES = -I$(top_srcdir)/korganizer/interfaces \
#           -I$(top_srcdir)/korganizer \
#	   -I$(top_srcdir) \
#	   -I$(top_builddir) \
#	   $(all_includes) 
#
#
#lib_LTLIBRARIES = libkocorehelper.la libkorg_stdprinting.la
#
#
#libkocorehelper_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) \
#                             -no-undefined -version-info 1:0
#libkocorehelper_la_LIBADD  = $(LIB_KDEUI)
#libkocorehelper_la_SOURCES = cellitem.cpp
#
#libkorg_stdprinting_la_SOURCES = calprinter.cpp calprinthelper.cpp calprintpluginbase.cpp \
#        calprintdefaultplugins.cpp \
#        calprintdayconfig_base.ui calprintmonthconfig_base.ui \
#        calprinttodoconfig_base.ui calprintweekconfig_base.ui
#libkorg_stdprinting_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) \
#                                 -no-undefined -version-info 1:0
#libkorg_stdprinting_la_LIBADD = $(LIB_KDEUI) libkocorehelper.la \
#                                $(top_builddir)/libkdepim/libkdepim.la \
#                                $(top_builddir)/libkcal/libkcal.la \
#                                -lkdeprint 
#
#
#calprintdir = $(includedir)/korganizer
#calprint_HEADERS = calprinthelper.h calprintpluginbase.h calprinter.h
#noinst_HEADERS = cellitem.h
#
