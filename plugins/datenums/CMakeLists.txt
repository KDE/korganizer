
include_directories( ${CMAKE_SOURCE_DIR}/korganizer/interfaces ${CMAKE_SOURCE_DIR}/korganizer  )


########### next target ###############

set(korg_datenums_PART_SRCS configdialog.cpp datenums.cpp )

kde4_automoc(${korg_datenums_PART_SRCS})

kde4_add_plugin(korg_datenums WITH_PREFIX ${korg_datenums_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} korg_datenums )

target_link_libraries(korg_datenums  ${KDE4_KDEUI_LIBS} korganizer )

install(TARGETS korg_datenums  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES datenums.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/korganizer)

