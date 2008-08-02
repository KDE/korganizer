
include_directories(
                     ${CMAKE_SOURCE_DIR}/korganizer/interfaces
                     ${CMAKE_SOURCE_DIR}/korganizer
                   )


########### next target ###############

set( korg_picoftheday_PART_SRCS
     ${CMAKE_SOURCE_DIR}/korganizer/interfaces/calendar/calendardecoration.cpp
     configdialog.cpp
     picoftheday.cpp
   )


kde4_add_plugin( korg_picoftheday ${korg_picoftheday_PART_SRCS} )



target_link_libraries( korg_picoftheday
                       ${KDE4_KIO_LIBS}
                       ${KDE4_KDEUI_LIBS}
                       korganizerprivate )

install( TARGETS korg_picoftheday
         DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES picoftheday.desktop
         DESTINATION ${SERVICES_INSTALL_DIR}/korganizer )


