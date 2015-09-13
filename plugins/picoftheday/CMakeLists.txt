
include_directories(
  ${CMAKE_SOURCE_DIR}/korganizer
)

########### next target ###############

set(korg_picoftheday_PART_SRCS
  configdialog.cpp
  picoftheday.cpp
)

ecm_qt_declare_logging_category(korg_picoftheday_PART_SRCS HEADER korganizer_picoftheday_plugin_debug.h IDENTIFIER KORGANIZERPICOFTHEDAYPLUGIN_LOG CATEGORY_NAME log_korganizer_picoftheday_plugins)

add_library(korg_picoftheday MODULE ${korg_picoftheday_PART_SRCS})

target_link_libraries(korg_picoftheday
  KF5::EventViews
  KF5::KIOCore
)

install(TARGETS
  korg_picoftheday
  DESTINATION ${KDE_INSTALL_PLUGINDIR}
)

########### install files ###############

install(FILES
  picoftheday.desktop
  DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/korganizer
)

