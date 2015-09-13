
include_directories(
  ${CMAKE_SOURCE_DIR}/korganizer
)

########### next target ###############

set(korg_hebrew_PART_SRCS
  configdialog.cpp
  converter.cpp
  hebrew.cpp
  holiday.cpp
  parsha.cpp
)

add_library(korg_hebrew MODULE ${korg_hebrew_PART_SRCS})

target_link_libraries(korg_hebrew
  KF5::EventViews
)

install(TARGETS
  korg_hebrew
  DESTINATION ${KDE_INSTALL_PLUGINDIR}
)

########### install files ###############

install(FILES
  hebrew.desktop
  DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/korganizer
)
