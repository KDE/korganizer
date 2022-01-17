# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

#Laurent : don't remove korgac (break standalone build)
include_directories(
    ${korganizer_BINARY_DIR}/korgac
    )

set(korganizer_xml ${korganizer_SOURCE_DIR}/src/data/org.kde.korganizer.Korganizer.xml)

########### next target ###############
add_executable(testalarmdlg)


qt_add_dbus_interface(testalarmdlg_SRCS ${korganizer_xml}
    korganizer_interface
    )

qt_add_dbus_interface(testalarmdlg_SRCS ../org.freedesktop.Notifications.xml notifications_interface)
qt_add_dbus_interface(testalarmdlg_SRCS ../org.freedesktop.DBus.Properties.xml dbusproperties)

target_sources(testalarmdlg PRIVATE
    testalarmdlg.cpp
    ../alarmdialog.cpp
    ../alarmdialog.h
     ${testalarmdlg_SRCS}
    ${korganizer_BINARY_DIR}/korgac/koalarmclient_debug.cpp)

target_link_libraries(testalarmdlg
    KF5::AkonadiCalendar
    KF5::CalendarSupport
    KF5::IncidenceEditor
    KF5::KIOCore
    KF5::Mime
    korganizer_core
    KF5::AkonadiCore
    KF5::CalendarCore
    KF5::CalendarUtils
    Phonon::phonon4qt${QT_MAJOR_VERSION}
    KF5::Notifications
    KF5::IconThemes
    KF5::WindowSystem
    )
