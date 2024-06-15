# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause

########### next target ###############
add_library(datenums MODULE)

target_sources(datenums PRIVATE
  configdialog.cpp
  datenums.cpp
  configdialog.h
  datenums.h
)

if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(datenums PROPERTIES UNITY_BUILD ON)
endif()

target_link_libraries(datenums
  KPim6::EventViews
)
install(TARGETS
  datenums
  DESTINATION ${KDE_INSTALL_PLUGINDIR}/pim6/korganizer
)
