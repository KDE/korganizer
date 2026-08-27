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
    pics/48-apps-realmoon-full.png
    pics/48-apps-realmoon-new.png
    pics/48-apps-realmoon-waning-last-quarter-north.png
    pics/48-apps-realmoon-waning-last-quarter-south.png
    pics/48-apps-realmoon-waxing-first-quarter-north.png
    pics/48-apps-realmoon-waxing-first-quarter-south.png
    DESTINATION ${KDE_INSTALL_ICONDIR}
    THEME hicolor
)
