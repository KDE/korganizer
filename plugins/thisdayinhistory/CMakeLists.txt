# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause

########### next target ###############

add_library(thisdayinhistory MODULE)
target_sources(thisdayinhistory PRIVATE
  thisdayinhistory.cpp
  thisdayinhistory.h
)

target_link_libraries(thisdayinhistory
  KPim6::EventViews
)

install(TARGETS
  thisdayinhistory
  DESTINATION ${KDE_INSTALL_PLUGINDIR}/pim6/korganizer
)
