
include_directories( ${CMAKE_SOURCE_DIR}/korganizer/interfaces ${CMAKE_SOURCE_DIR}/korganizer/printing  )


########### next target ###############

set(korg_yearprint_PART_SRCS yearprint.cpp )


kde4_add_ui_files(korg_yearprint_PART_SRCS calprintyearconfig_base.ui )

kde4_add_plugin(korg_yearprint ${korg_yearprint_PART_SRCS})



target_link_libraries(korg_yearprint  ${KDE4_KDEUI_LIBS} korganizerprivate ${QT_QT3SUPPORT_LIBRARY})

install(TARGETS korg_yearprint  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES yearprint.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/korganizer)
