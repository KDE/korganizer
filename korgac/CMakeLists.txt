# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause
add_definitions(-DTRANSLATION_DOMAIN=\"korgac\")


add_subdirectory(pixmaps)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

########### next target ###############
add_executable(korgac)

ecm_qt_declare_logging_category(korgac HEADER koalarmclient_debug.h IDENTIFIER KOALARMCLIENT_LOG CATEGORY_NAME org.kde.pim.koalarmclient
        DESCRIPTION "korganizer (koalarmclient)"
        OLD_CATEGORY_NAMES log_koalarmclient
        EXPORT KORGANIZER
    )


set(korganizer_xml ../src/data/org.kde.korganizer.Korganizer.xml)

qt_add_dbus_interface(korgac_SRCS ${korganizer_xml}
    korganizer_interface
    )

qt_add_dbus_adaptor(korgac_SRCS org.kde.korganizer.KOrgac.xml koalarmclient.h
    KOAlarmClient
    )

qt_add_dbus_interface(korgac_SRCS org.freedesktop.Notifications.xml notifications_interface)
qt_add_dbus_interface(korgac_SRCS org.freedesktop.DBus.Properties.xml dbusproperties)

file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/pixmaps/*-apps-korgac.png")
ecm_add_app_icon(korgac_SRCS ICONS ${ICONS_SRCS})
target_sources(korgac PRIVATE
    korgacmain.cpp
    alarmdialog.cpp
    alarmdockwindow.cpp
    koalarmclient.cpp
    alarmdialog.h
    alarmdockwindow.h
    koalarmclient.h
    ${korgac_SRCS}
    )

if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(korgac PROPERTIES UNITY_BUILD ON)
endif()
if( APPLE )
    set_target_properties(korgac PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST ${korganizer_SOURCE_DIR}/korgac/Info.plist.template
        )
    set_target_properties(korgac PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER "org.kde.pim.korgac"
        )
    set_target_properties(korgac PROPERTIES
        MACOSX_BUNDLE_BUNDLE_NAME "KOrganizer Reminder Client"
        )
endif()

target_link_libraries(korgac
    KF5::AkonadiCalendar
    KF5::CalendarSupport
    KF5::IncidenceEditor
    KF5::AkonadiCore
    KF5::CalendarCore
    KF5::CalendarUtils
    KF5::DBusAddons
    Phonon::phonon4qt${QT_MAJOR_VERSION}
    KF5::Notifications
    KF5::IconThemes
    KF5::Service
    KF5::KIOCore
    KF5::WindowSystem
    )

install(TARGETS
    korgac ${KDE_INSTALL_TARGETS_DEFAULT_ARGS}
    )

install(FILES
    autostart-org.kde.korgac.desktop
    RENAME org.kde.korgac.desktop
    DESTINATION ${KDE_INSTALL_AUTOSTARTDIR}
    )

install(FILES
    org.kde.korgac.desktop
    DESTINATION ${KDE_INSTALL_APPDIR}
    )

install(FILES
    org.kde.korganizer.KOrgac.xml
    DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR}
    )

configure_file(org.kde.korgac.service.in org.kde.korgac.service)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/org.kde.korgac.service
    DESTINATION ${KDE_INSTALL_DBUSSERVICEDIR}
    )
