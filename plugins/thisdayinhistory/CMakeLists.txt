
include_directories( ${CMAKE_SOURCE_DIR}/korganizer/interfaces ${CMAKE_SOURCE_DIR}/korganizer  )


########### next target ###############

set(korg_thisdayinhistory_PART_SRCS thisdayinhistory.cpp tdihwidget.cpp)

kde4_automoc(${korg_thisdayinhistory_PART_SRCS})

kde4_add_plugin(korg_thisdayinhistory WITH_PREFIX ${korg_thisdayinhistory_PART_SRCS})



target_link_libraries(korg_thisdayinhistory  ${KDE4_KDEUI_LIBS} korganizer )

install(TARGETS korg_thisdayinhistory  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES thisdayinhistory.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/korganizer)

