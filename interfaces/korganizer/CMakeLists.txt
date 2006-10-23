


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
install(TARGETS korganizer_interfaces  DESTINATION ${LIB_INSTALL_DIR})


########### install files ###############

install( FILES korganizerpart.desktop korgprintplugin.desktop  DESTINATION ${SERVICETYPES_INSTALL_DIR})
install( FILES part.h baseview.h calendarviewbase.h mainwindow.h corehelper.h printplugin.h  DESTINATION ${INCLUDE_INSTALL_DIR}/korganizer)

