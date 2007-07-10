
include_directories( ${CMAKE_SOURCE_DIR}/korganizer/interfaces  )


########### next target ###############

set(korg_hebrew_PART_SRCS 
   hebrew.cpp 
   configdialog.cpp 
   holiday.cpp 
   parsha.cpp 
   converter.cpp )


kde4_add_plugin(korg_hebrew WITH_PREFIX ${korg_hebrew_PART_SRCS})



target_link_libraries(korg_hebrew  ${KDE4_KDEUI_LIBS} )

install(TARGETS korg_hebrew  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES hebrew.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/korganizer)

