
include_directories(
  ${CMAKE_SOURCE_DIR}/korganizer/interfaces
)

########### next target ###############

set(korg_whatsnextprint_PART_SRCS whatsnextprint.cpp)

kde4_add_ui_files(korg_whatsnextprint_PART_SRCS calprintwhatsnextconfig_base.ui)

kde4_add_plugin(korg_whatsnextprint ${korg_whatsnextprint_PART_SRCS})

target_link_libraries(korg_whatsnextprint
  korganizerprivate
  ${KDE4_KDEUI_LIBS}
)

install(TARGETS
  korg_whatsnextprint
  DESTINATION ${PLUGIN_INSTALL_DIR}
)

########### install files ###############

install(FILES
  whatsnextprint.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}/korganizer
)
