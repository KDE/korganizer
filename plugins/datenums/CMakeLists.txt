
include_directories(
  ${CMAKE_SOURCE_DIR}/korganizer/interfaces
  ${CMAKE_SOURCE_DIR}/korganizer
)

########### next target ###############

set(korg_datenums_PART_SRCS
  configdialog.cpp
  datenums.cpp
)

add_library(korg_datenums MODULE ${korg_datenums_PART_SRCS})

target_link_libraries(korg_datenums
  eventviews
  korganizerprivate
  
  
)
install(TARGETS
  korg_datenums
  DESTINATION ${PLUGIN_INSTALL_DIR}
)

########### install files ###############

install(FILES
  datenums.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}/korganizer
)
