
########### next target ###############

set(korganizer_interfaces_LIB_SRCS
  baseview.cpp
  mainwindow.cpp
)

kde4_add_library(korganizer_interfaces ${LIBRARY_TYPE} ${korganizer_interfaces_LIB_SRCS})

target_link_libraries(korganizer_interfaces
  akonadi_next
  calendarsupport
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDE4_KIO_LIBS}
)

set_target_properties(korganizer_interfaces PROPERTIES
  VERSION ${KDEPIM_LIB_VERSION}
  SOVERSION ${KDEPIM_LIB_SOVERSION}
)

install(TARGETS
  korganizer_interfaces ${INSTALL_TARGETS_DEFAULT_ARGS}
)
