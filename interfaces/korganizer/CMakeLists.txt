
########### next target ###############

set(korganizer_interfaces_LIB_SRCS
  baseview.cpp
  mainwindow.cpp
)

add_library(korganizer_interfaces ${LIBRARY_TYPE} ${korganizer_interfaces_LIB_SRCS})
generate_export_header(korganizer_interfaces BASE_NAME korganizer_interfaces)
target_link_libraries(korganizer_interfaces
  akonadi_next
  calendarsupport
  KF5::AkonadiCore
  KF5::KIOCore
)

set_target_properties(korganizer_interfaces PROPERTIES
  VERSION ${KDEPIM_LIB_VERSION}
  SOVERSION ${KDEPIM_LIB_SOVERSION}
)

install(TARGETS
  korganizer_interfaces ${INSTALL_TARGETS_DEFAULT_ARGS}
)
