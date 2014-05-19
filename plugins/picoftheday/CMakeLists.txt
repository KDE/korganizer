
include_directories(
  ${CMAKE_SOURCE_DIR}/korganizer/interfaces
  ${CMAKE_SOURCE_DIR}/korganizer
)

########### next target ###############

set(korg_picoftheday_PART_SRCS
  configdialog.cpp
  picoftheday.cpp
)

add_library(korg_picoftheday MODULE ${korg_picoftheday_PART_SRCS})

target_link_libraries(korg_picoftheday
  eventviews
  korganizerprivate
  
  KF5::KIOCore
)

install(TARGETS
  korg_picoftheday
  DESTINATION ${PLUGIN_INSTALL_DIR}
)

########### install files ###############

install(FILES
  picoftheday.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}/korganizer
)

