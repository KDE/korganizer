########### next target ###############
set(libcommon_SRCS korg_uniqueapp.cpp)
ecm_qt_declare_logging_category(libcommon_SRCS HEADER korganizerplugin_debug.h IDENTIFIER KORGANIZERPLUGIN_LOG CATEGORY_NAME org.kde.pim.korganizer_plugin
        DESCRIPTION "korganizer (korganizer kontact plugins)"
        OLD_CATEGORY_NAMES log_korganizer_plugin
        EXPORT KORGANIZER
    )


set(kontact_korganizerplugin_PART_SRCS korganizerplugin.cpp apptsummarywidget.cpp summaryeventinfo.cpp ${libcommon_SRCS})

qt_add_dbus_interfaces(kontact_korganizerplugin_PART_SRCS ${korganizer_SOURCE_DIR}/src/data/org.kde.Korganizer.Calendar.xml  ${korganizer_SOURCE_DIR}/src/data/org.kde.korganizer.Korganizer.xml)

add_library(kontact_korganizerplugin MODULE ${kontact_korganizerplugin_PART_SRCS})

target_link_libraries(kontact_korganizerplugin KF5::AkonadiCalendar KF5::CalendarUtils KF5::Contacts KF5::CalendarCore KF5::KontactInterface korganizerprivate KF5::CalendarSupport KF5::AkonadiCalendar KF5::WindowSystem KF5::I18n)

kcoreaddons_desktop_to_json(kontact_korganizerplugin korganizerplugin.desktop)

########### next target ###############

set(kontact_todoplugin_PART_SRCS todoplugin.cpp todosummarywidget.cpp ${libcommon_SRCS})

qt_add_dbus_interfaces(kontact_todoplugin_PART_SRCS ${korganizer_SOURCE_DIR}/src/data/org.kde.Korganizer.Calendar.xml  ${korganizer_SOURCE_DIR}/src/data/org.kde.korganizer.Korganizer.xml)

add_library(kontact_todoplugin MODULE ${kontact_todoplugin_PART_SRCS})

target_link_libraries(kontact_todoplugin KF5::AkonadiCalendar  KF5::Contacts KF5::KontactInterface KF5::CalendarCore KF5::CalendarUtils KF5::CalendarSupport KF5::AkonadiCalendar KF5::WindowSystem)

kcoreaddons_desktop_to_json(kontact_todoplugin todoplugin.desktop)

########### next target ###############

set(kontact_journalplugin_PART_SRCS journalplugin.cpp ${libcommon_SRCS})

qt_add_dbus_interfaces(kontact_journalplugin_PART_SRCS ${korganizer_SOURCE_DIR}/src/data/org.kde.Korganizer.Calendar.xml)

add_library(kontact_journalplugin MODULE ${kontact_journalplugin_PART_SRCS})

target_link_libraries(kontact_journalplugin KF5::I18n KF5::KontactInterface KF5::WindowSystem)

kcoreaddons_desktop_to_json(kontact_journalplugin journalplugin.desktop)

########### next target ###############

set(kcmapptsummary_PART_SRCS kcmapptsummary.cpp)

ki18n_wrap_ui(kcmapptsummary_PART_SRCS apptsummaryconfig_base.ui)

add_library(kcmapptsummary MODULE ${kcmapptsummary_PART_SRCS})
kcoreaddons_desktop_to_json(kcmapptsummary "kcmapptsummary.desktop" SERVICE_TYPES kcmodule.desktop)
target_link_libraries(kcmapptsummary  KF5::KCMUtils KF5::I18n)

########### next target ###############

set(kcmtodosummary_PART_SRCS kcmtodosummary.cpp)

ki18n_wrap_ui(kcmtodosummary_PART_SRCS todosummaryconfig_base.ui)

add_library(kcmtodosummary MODULE ${kcmtodosummary_PART_SRCS})
kcoreaddons_desktop_to_json(kcmtodosummary "kcmtodosummary.desktop" SERVICE_TYPES kcmodule.desktop)
target_link_libraries(kcmtodosummary Qt::Widgets KF5::KCMUtils KF5::I18n)

########## Unit Test ###########
if (BUILD_TESTING)
   add_subdirectory(autotests)
endif()
########### install files ###############

install(TARGETS kcmapptsummary DESTINATION ${KDE_INSTALL_PLUGINDIR}/pim/kcms/summary)
install(TARGETS kcmtodosummary DESTINATION ${KDE_INSTALL_PLUGINDIR}/pim/kcms/summary)
install(TARGETS kontact_korganizerplugin DESTINATION ${KDE_INSTALL_PLUGINDIR}/kontact5)
install(TARGETS kontact_todoplugin DESTINATION ${KDE_INSTALL_PLUGINDIR}/kontact5)
install(TARGETS kontact_journalplugin DESTINATION ${KDE_INSTALL_PLUGINDIR}/kontact5)

install(FILES korganizerplugin.desktop todoplugin.desktop journalplugin.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/kontact)
install(FILES kcmapptsummary.desktop kcmtodosummary.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})

install(FILES korganizer.setdlg DESTINATION ${KDE_INSTALL_DATADIR}/kontact/ksettingsdialog)


