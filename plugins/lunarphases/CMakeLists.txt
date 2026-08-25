# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause

########### next target ###############

add_library(
    lunarphases
    MODULE
    configdialog.cpp
    configdialog.h
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
    pics/48-apps-goldmoon-full.png
    pics/48-apps-goldmoon-new.png
    pics/48-apps-goldmoon-waning-crescent-north.png
    pics/48-apps-goldmoon-waning-crescent-south.png
    pics/48-apps-goldmoon-waning-gibbous-north.png
    pics/48-apps-goldmoon-waning-gibbous-south.png
    pics/48-apps-goldmoon-waning-last-quarter-north.png
    pics/48-apps-goldmoon-waning-last-quarter-south.png
    pics/48-apps-goldmoon-waxing-crescent-north.png
    pics/48-apps-goldmoon-waxing-crescent-south.png
    pics/48-apps-goldmoon-waxing-first-quarter-north.png
    pics/48-apps-goldmoon-waxing-first-quarter-south.png
    pics/48-apps-goldmoon-waxing-gibbous-north.png
    pics/48-apps-goldmoon-waxing-gibbous-south.png
    DESTINATION ${KDE_INSTALL_ICONDIR}
    THEME hicolor
)
