kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${SERVICETYPES_INSTALL_DIR} FILES korganizerpart.desktop korgprintplugin.desktop )
install_files( /include/korganizer FILES part.h baseview.h calendarviewbase.h mainwindow.h corehelper.h printplugin.h )

kde4_footer()



#original Makefile.am contents follow:

#
#korginclude_HEADERS = part.h baseview.h calendarviewbase.h mainwindow.h corehelper.h printplugin.h
#korgincludedir = $(includedir)/korganizer
#
#servicetypedir = $(kde_servicetypesdir)
#servicetype_DATA = korganizerpart.desktop korgprintplugin.desktop
