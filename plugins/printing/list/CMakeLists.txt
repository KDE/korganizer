
include_directories( ${CMAKE_SOURCE_DIR}/korganizer/interfaces ${CMAKE_SOURCE_DIR}/korganizer/printing  )


########### next target ###############

set(korg_listprint_PART_SRCS listprint.cpp )


kde4_add_ui_files(korg_listprint_PART_SRCS calprintlistconfig_base.ui )

kde4_add_plugin(korg_listprint ${korg_listprint_PART_SRCS})



target_link_libraries(korg_listprint  ${KDE4_KDEUI_LIBS} korganizerprivate )

install(TARGETS korg_listprint  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES listprint.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/korganizer)
