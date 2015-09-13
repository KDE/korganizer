
include_directories(
  ${CMAKE_SOURCE_DIR}/korganizer
)

########### next target ###############

set(korg_thisdayinhistory_PART_SRCS
  thisdayinhistory.cpp
)

add_library(korg_thisdayinhistory MODULE ${korg_thisdayinhistory_PART_SRCS})

target_link_libraries(korg_thisdayinhistory
  KF5::EventViews
)

install(TARGETS
  korg_thisdayinhistory
  DESTINATION ${KDE_INSTALL_PLUGINDIR}
)

########### install files ###############

install(FILES
  thisdayinhistory.desktop
  DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/korganizer
)
