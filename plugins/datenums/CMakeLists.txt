
include_directories(
                     ${CMAKE_SOURCE_DIR}/korganizer/interfaces
                     ${CMAKE_SOURCE_DIR}/korganizer
                   )


########### next target ###############

set( korg_datenums_PART_SRCS
     ${CMAKE_SOURCE_DIR}/korganizer/interfaces/calendar/calendardecoration.cpp
     configdialog.cpp
     datenums.cpp
   )

kde4_add_plugin( korg_datenums ${korg_datenums_PART_SRCS} )

target_link_libraries( korg_datenums
                       ${KDE4_KDECORE_LIBS}
                       ${KDE4_KDEUI_LIBS}
                       korganizerprivate )

install( TARGETS korg_datenums
         DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES datenums.desktop
         DESTINATION ${SERVICES_INSTALL_DIR}/korganizer )
