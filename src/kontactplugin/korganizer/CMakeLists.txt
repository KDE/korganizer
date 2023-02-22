# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause
########### next target ###############
set(libcommon_SRCS korg_uniqueapp.cpp korg_uniqueapp.h)
ecm_qt_declare_logging_category(libcommon_SRCS HEADER korganizerplugin_debug.h IDENTIFIER KORGANIZERPLUGIN_LOG CATEGORY_NAME org.kde.pim.korganizer_plugin
        DESCRIPTION "korganizer (korganizer kontact plugins)"
        OLD_CATEGORY_NAMES log_korganizer_plugin
        EXPORT KORGANIZER
    )

configure_file(korganizerplugin.json.cmake ${CMAKE_CURRENT_BINARY_DIR}/korganizerplugin.json)

set(kontact_korganizerplugin_PART_SRCS korganizerplugin.cpp apptsummarywidget.cpp summaryeventinfo.cpp korganizerplugin.h apptsummarywidget.h summaryeventinfo.h ${libcommon_SRCS})

qt_add_dbus_interfaces(kontact_korganizerplugin_PART_SRCS ${korganizer_SOURCE_DIR}/src/data/org.kde.Korganizer.Calendar.xml  ${korganizer_SOURCE_DIR}/src/data/org.kde.korganizer.Korganizer.xml)

add_library(kontact_korganizerplugin MODULE ${kontact_korganizerplugin_PART_SRCS})

target_link_libraries(kontact_korganizerplugin KF5::AkonadiCalendar KPim${KF_MAJOR_VERSION}::CalendarUtils KF${KF_MAJOR_VERSION}::Contacts KF${KF_MAJOR_VERSION}::CalendarCore KPim${KF_MAJOR_VERSION}::KontactInterface korganizerprivate KF5::CalendarSupport KF5::AkonadiCalendar KF${KF_MAJOR_VERSION}::WindowSystem KF${KF_MAJOR_VERSION}::I18n)

########### next target ###############

set(kontact_todoplugin_PART_SRCS todoplugin.cpp todosummarywidget.cpp todoplugin.h todosummarywidget.h ${libcommon_SRCS})

qt_add_dbus_interfaces(kontact_todoplugin_PART_SRCS ${korganizer_SOURCE_DIR}/src/data/org.kde.Korganizer.Calendar.xml  ${korganizer_SOURCE_DIR}/src/data/org.kde.korganizer.Korganizer.xml)

add_library(kontact_todoplugin MODULE ${kontact_todoplugin_PART_SRCS})

target_link_libraries(kontact_todoplugin KF5::AkonadiCalendar  KF${KF_MAJOR_VERSION}::Contacts KPim${KF_MAJOR_VERSION}::KontactInterface KF${KF_MAJOR_VERSION}::CalendarCore KPim${KF_MAJOR_VERSION}::CalendarUtils KF5::CalendarSupport KF5::AkonadiCalendar KF${KF_MAJOR_VERSION}::WindowSystem)

########### next target ###############

set(kontact_journalplugin_PART_SRCS journalplugin.cpp journalplugin.h ${libcommon_SRCS})

qt_add_dbus_interfaces(kontact_journalplugin_PART_SRCS ${korganizer_SOURCE_DIR}/src/data/org.kde.Korganizer.Calendar.xml)

add_library(kontact_journalplugin MODULE ${kontact_journalplugin_PART_SRCS})

target_link_libraries(kontact_journalplugin KF${KF_MAJOR_VERSION}::I18n KPim${KF_MAJOR_VERSION}::KontactInterface KF${KF_MAJOR_VERSION}::WindowSystem)

########### next target ###############

set(kcmapptsummary_PART_SRCS kcmapptsummary.cpp kcmapptsummary.h)

ki18n_wrap_ui(kcmapptsummary_PART_SRCS apptsummaryconfig_base.ui)

add_library(kcmapptsummary MODULE ${kcmapptsummary_PART_SRCS})
target_link_libraries(kcmapptsummary  KF${KF_MAJOR_VERSION}::KCMUtils KF${KF_MAJOR_VERSION}::I18n KF${KF_MAJOR_VERSION}::CoreAddons)

########### next target ###############

set(kcmtodosummary_PART_SRCS kcmtodosummary.cpp kcmtodosummary.h)

ki18n_wrap_ui(kcmtodosummary_PART_SRCS todosummaryconfig_base.ui)

add_library(kcmtodosummary MODULE ${kcmtodosummary_PART_SRCS})
target_link_libraries(kcmtodosummary Qt::Widgets KF${KF_MAJOR_VERSION}::KCMUtils KF${KF_MAJOR_VERSION}::I18n KF${KF_MAJOR_VERSION}::CoreAddons)

########## Unit Test ###########
if (BUILD_TESTING)
   add_subdirectory(autotests)
endif()
########### install files ###############

install(TARGETS kcmapptsummary DESTINATION ${KDE_INSTALL_PLUGINDIR}/pim${QT_MAJOR_VERSION}/kcms/summary)
install(TARGETS kcmtodosummary DESTINATION ${KDE_INSTALL_PLUGINDIR}/pim${QT_MAJOR_VERSION}/kcms/summary)
install(TARGETS kontact_korganizerplugin DESTINATION ${KDE_INSTALL_PLUGINDIR}/pim${QT_MAJOR_VERSION}/kontact)
install(TARGETS kontact_todoplugin DESTINATION ${KDE_INSTALL_PLUGINDIR}/pim${QT_MAJOR_VERSION}/kontact)
install(TARGETS kontact_journalplugin DESTINATION ${KDE_INSTALL_PLUGINDIR}/pim${QT_MAJOR_VERSION}/kontact)
