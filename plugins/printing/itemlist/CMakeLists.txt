
include_directories(
  ${CMAKE_SOURCE_DIR}/korganizer/interfaces
  ${CMAKE_SOURCE_DIR}/korganizer/printing
)

########### next target ###############

set(korg_itemlistprint_PART_SRCS itemlistprint.cpp)

kde4_add_ui_files(korg_itemlistprint_PART_SRCS calprintitemlistconfig_base.ui)

kde4_add_plugin(korg_itemlistprint ${korg_itemlistprint_PART_SRCS})

target_link_libraries(korg_itemlistprint
  korganizerprivate
  ${KDE4_KDEUI_LIBS}
)

install(TARGETS
  korg_itemlistprint
  DESTINATION ${PLUGIN_INSTALL_DIR}
)

########### install files ###############

install(FILES
  itemlistprint.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}/korganizer
)
