kde4_header()



########### next target ###############

set(korganizer_interfaces_LIB_SRCS
  baseview.cpp
  incidencechangerbase.cpp
  mainwindow.cpp
)

kde4_automoc(${korganizer_interfaces_LIB_SRCS})

kde4_add_library(korganizer_interfaces SHARED ${korganizer_interfaces_LIB_SRCS})

target_link_libraries(korganizer_interfaces  ${KDE4_KIO_LIBS}  )

set_target_properties(korganizer_interfaces PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} korganizer_interfaces )


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
