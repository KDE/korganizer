
include_directories( ${CMAKE_SOURCE_DIR}/korganizer/interfaces ${CMAKE_SOURCE_DIR}/korganizer  )


########### next target ###############

set(korg_timespanview_PART_SRCS 
   lineview.cpp 
   timeline.cpp 
   timespanview.cpp 
   timespanwidget.cpp 
   kotimespanview.cpp )


kde4_add_plugin(korg_timespanview ${korg_timespanview_PART_SRCS})



target_link_libraries(korg_timespanview ${KDE4_KPARTS_LIBS} korganizer)

install(TARGETS korg_timespanview  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES timespanview.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/korganizer)
install( FILES timespanviewui.rc  DESTINATION ${DATA_INSTALL_DIR}/korganizer/plugins)
