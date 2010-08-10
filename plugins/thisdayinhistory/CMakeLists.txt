
include_directories(
                     ${CMAKE_SOURCE_DIR}/korganizer/interfaces
                     ${CMAKE_SOURCE_DIR}/korganizer
                   )


########### next target ###############

set( korg_thisdayinhistory_PART_SRCS
     ${CMAKE_SOURCE_DIR}/korganizer/interfaces/calendar/calendardecoration.cpp
     thisdayinhistory.cpp
   )


kde4_add_plugin( korg_thisdayinhistory ${korg_thisdayinhistory_PART_SRCS} )



target_link_libraries( korg_thisdayinhistory
                       ${KDE4_KDECORE_LIBS}
                       ${KDE4_KDEUI_LIBS}
                       korganizerprivate )

install( TARGETS korg_thisdayinhistory
         DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES thisdayinhistory.desktop
         DESTINATION ${SERVICES_INSTALL_DIR}/korganizer )
