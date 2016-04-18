
########### next target ###############

set(korganizer_interfaces_LIB_SRCS
  baseview.cpp
  mainwindow.cpp
)

add_library(korganizer_interfaces ${korganizer_interfaces_LIB_SRCS})
generate_export_header(korganizer_interfaces BASE_NAME korganizer_interfaces)
target_link_libraries(korganizer_interfaces
  KF5::CalendarSupport
  KF5::EventViews
  KF5::AkonadiCore
)

target_include_directories(korganizer_interfaces PUBLIC "$<BUILD_INTERFACE:${korganizer_SOURCE_DIR}/interfaces/korganizer;${korganizer_BINARY_DIR}/interfaces/korganizer>")

set_target_properties(korganizer_interfaces PROPERTIES
  VERSION ${KDEPIM_LIB_VERSION}
  SOVERSION ${KDEPIM_LIB_SOVERSION}
)

install(TARGETS
  korganizer_interfaces ${KDE_INSTALL_TARGETS_DEFAULT_ARGS}
)
