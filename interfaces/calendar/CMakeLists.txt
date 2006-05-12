



########### install files ###############

install_files( ${SERVICETYPES_INSTALL_DIR} FILES calendarplugin.desktop calendardecoration.desktop )
install_files( /include/calendar FILES plugin.h calendardecoration.h )




#original Makefile.am contents follow:

#
#calinclude_HEADERS = plugin.h calendardecoration.h
#calincludedir = $(includedir)/calendar
#
#servicetypedir = $(kde_servicetypesdir)
#servicetype_DATA = calendarplugin.desktop calendardecoration.desktop
