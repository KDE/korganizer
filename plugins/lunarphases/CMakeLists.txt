# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause

########### next target ###############

add_library(
    lunarphases
    MODULE
    lunarphases.cpp
    lunarphases.h
)

target_link_libraries(
    lunarphases
    KF6::Holidays
    KPim6::EventViews
)

install(TARGETS lunarphases DESTINATION ${KDE_INSTALL_PLUGINDIR}/pim6/korganizer)

########### install files ###############

include(ECMInstallIcons)
ecm_install_icons(
    ICONS
  pics/48-apps-moon-full.png
  pics/48-apps-moon-new.png
  pics/48-apps-moon-waning-crescent-north.png
  pics/48-apps-moon-waning-crescent-south.png
  pics/48-apps-moon-waning-gibbous-north.png
  pics/48-apps-moon-waning-gibbous-south.png
  pics/48-apps-moon-waning-last-quarter-north.png
  pics/48-apps-moon-waning-last-quarter-south.png
  pics/48-apps-moon-waxing-crescent-north.png
  pics/48-apps-moon-waxing-crescent-south.png
  pics/48-apps-moon-waxing-first-quarter-north.png
  pics/48-apps-moon-waxing-first-quarter-south.png
  pics/48-apps-moon-waxing-gibbous-south.png
  pics/48-apps-moon-waxing-gibbous-south.png
    DESTINATION ${KDE_INSTALL_ICONDIR}
    THEME hicolor
)
