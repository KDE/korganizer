
include_directories( ${CMAKE_SOURCE_DIR}/korganizer/interfaces ${CMAKE_SOURCE_DIR}/korganizer/printing  )


########### next target ###############

set(korg_journalprint_PART_SRCS journalprint.cpp )


kde4_add_ui_files(korg_journalprint_PART_SRCS calprintjournalconfig_base.ui )

kde4_add_plugin(korg_journalprint ${korg_journalprint_PART_SRCS})



target_link_libraries(korg_journalprint  ${KDE4_KDEUI_LIBS} korganizerprivate kdepim)

install(TARGETS korg_journalprint  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES journalprint.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/korganizer)
