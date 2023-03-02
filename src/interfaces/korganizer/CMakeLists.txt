# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause
########### next target ###############

add_library(korganizer_interfaces)
target_sources(korganizer_interfaces PRIVATE
    baseview.cpp
    mainwindow.cpp
    baseview.h
    mainwindow.h
    )
generate_export_header(korganizer_interfaces BASE_NAME korganizer_interfaces)
target_link_libraries(korganizer_interfaces
  KPim${KF_MAJOR_VERSION}::CalendarSupport
  KPim${KF_MAJOR_VERSION}::EventViews
  KPim${KF_MAJOR_VERSION}::AkonadiCore
  Qt::PrintSupport
  KF${KF_MAJOR_VERSION}::Parts
)

set_target_properties(korganizer_interfaces PROPERTIES
  VERSION ${KDEPIM_LIB_VERSION}
  SOVERSION ${KDEPIM_LIB_SOVERSION}
)

install(TARGETS
  korganizer_interfaces ${KDE_INSTALL_TARGETS_DEFAULT_ARGS}  LIBRARY NAMELINK_SKIP
)
