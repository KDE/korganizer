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
    pics/sc-apps-realmoon-full.svg
    pics/sc-apps-realmoon-new.svg
    pics/sc-apps-realmoon-waning-last-quarter-north.svg
    pics/sc-apps-realmoon-waning-last-quarter-south.svg
    pics/sc-apps-realmoon-waxing-first-quarter-north.svg
    pics/sc-apps-realmoon-waxing-first-quarter-south.svg
    DESTINATION ${KDE_INSTALL_ICONDIR}
    THEME hicolor
)
