



########### install files ###############

install( FILES calendarplugin.desktop calendardecoration.desktop  DESTINATION ${SERVICETYPES_INSTALL_DIR})
install( FILES plugin.h calendardecoration.h  DESTINATION ${INCLUDE_INSTALL_DIR}/calendar)




#original Makefile.am contents follow:

#
#calinclude_HEADERS = plugin.h calendardecoration.h
#calincludedir = $(includedir)/calendar
#
#servicetypedir = $(kde_servicetypesdir)
#servicetype_DATA = calendarplugin.desktop calendardecoration.desktop
