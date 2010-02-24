


########### next target ###############

set(korganizer_interfaces_LIB_SRCS
  baseview.cpp
  incidencechangerbase.cpp
  mainwindow.cpp
)


kde4_add_library(korganizer_interfaces SHARED ${korganizer_interfaces_LIB_SRCS})

target_link_libraries(korganizer_interfaces ${KDE4_KIO_LIBS} ${KDEPIMLIBS_AKONADI_LIBS} ${KDEPIMLIBS_AKONADI_KCAL_LIBS} akonadi_next akonadi-kcal_next)

set_target_properties(korganizer_interfaces PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS korganizer_interfaces ${INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############

install( FILES korganizerpart.desktop korgprintplugin.desktop  DESTINATION ${SERVICETYPES_INSTALL_DIR})

