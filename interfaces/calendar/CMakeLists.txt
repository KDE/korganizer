kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${SERVICETYPES_INSTALL_DIR} FILES calendarplugin.desktop calendardecoration.desktop )
install_files( /include/calendar FILES plugin.h calendardecoration.h )

kde4_footer()



#original Makefile.am contents follow:

#
#calinclude_HEADERS = plugin.h calendardecoration.h
#calincludedir = $(includedir)/calendar
#
#servicetypedir = $(kde_servicetypesdir)
#servicetype_DATA = calendarplugin.desktop calendardecoration.desktop
