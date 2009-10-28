


########### next target ###############

set(korganizer_interfaces_LIB_SRCS
  baseview.cpp
  incidencechangerbase.cpp
  mainwindow.cpp
)


kde4_add_library(korganizer_interfaces SHARED ${korganizer_interfaces_LIB_SRCS})

target_link_libraries(korganizer_interfaces akonadi-kcal ${KDE4_KIO_LIBS}  )

set_target_properties(korganizer_interfaces PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS korganizer_interfaces ${INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############

install( FILES korganizerpart.desktop korgprintplugin.desktop  DESTINATION ${SERVICETYPES_INSTALL_DIR})

