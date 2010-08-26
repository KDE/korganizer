
include_directories(
                     ${CMAKE_SOURCE_DIR}/korganizer/interfaces
                     ${CMAKE_SOURCE_DIR}/korganizer
                   )


########### next target ###############

set( korg_hebrew_PART_SRCS
     converter.cpp
     parsha.cpp
     holiday.cpp
     configdialog.cpp
     hebrew.cpp
   )

kde4_add_plugin( korg_hebrew ${korg_hebrew_PART_SRCS} )

target_link_libraries( korg_hebrew
                       eventviews
                       ${KDE4_KDECORE_LIBS}
                       ${KDE4_KDEUI_LIBS}
                       korganizerprivate )

install( TARGETS korg_hebrew
         DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES hebrew.desktop
         DESTINATION ${SERVICES_INSTALL_DIR}/korganizer )
