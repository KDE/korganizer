
include_directories(
  ${CMAKE_SOURCE_DIR}/korganizer/interfaces
  ${CMAKE_SOURCE_DIR}/korganizer
)

########### next target ###############

set(korg_thisdayinhistory_PART_SRCS
  thisdayinhistory.cpp
)

add_library(korg_thisdayinhistory MODULE ${korg_thisdayinhistory_PART_SRCS})

target_link_libraries(korg_thisdayinhistory
  eventviews
  korganizerprivate
  
  
)

install(TARGETS
  korg_thisdayinhistory
  DESTINATION ${PLUGIN_INSTALL_DIR}
)

########### install files ###############

install(FILES
  thisdayinhistory.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}/korganizer
)
