add_definitions(-DTRANSLATION_DOMAIN=\"korgac\")
add_definitions(-DQT_NO_CAST_FROM_ASCII)
add_definitions(-DQT_NO_CAST_TO_ASCII)


add_subdirectory(pixmaps)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

########### next target ###############

set(korgac_SRCS
    koalarmclient.cpp
    )

ecm_qt_declare_logging_category(korgac_SRCS HEADER koalarmclient_debug.h IDENTIFIER KOALARMCLIENT_LOG CATEGORY_NAME org.kde.pim.koalarmclient)

set(korgac_SRCS ${korgac_SRCS} korgacmain.cpp)

set(korgac_SRCS
    ${korgac_SRCS}
    alarmdialog.cpp
    alarmdockwindow.cpp
    mailclient.cpp
    )

set(korganizer_xml ../src/data/org.kde.korganizer.Korganizer.xml)

qt5_add_dbus_interface(korgac_SRCS ${korganizer_xml}
    korganizer_interface
    )

qt5_add_dbus_adaptor(korgac_SRCS org.kde.korganizer.KOrgac.xml koalarmclient.h
    KOAlarmClient
    )


file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/pixmaps/*-apps-korgac.png")
ecm_add_app_icon(korgac_SRCS ICONS ${ICONS_SRCS})

add_executable(korgac ${korgac_SRCS})
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
    KF5::AkonadiMime
    KF5::CalendarSupport
    KF5::IncidenceEditor
    KF5::AkonadiCore
    KF5::CalendarCore
    KF5::CalendarUtils
    KF5::IdentityManagement
    KF5::MailTransportAkonadi
    KF5::Codecs
    KF5::DBusAddons
    Phonon::phonon4qt5
    KF5::Notifications
    KF5::Completion
    KF5::IconThemes
    KF5::Service
    KF5::KIOCore
    KF5::WindowSystem
    )

install(TARGETS
    korgac ${KDE_INSTALL_TARGETS_DEFAULT_ARGS}
    )

install(FILES
    org.kde.korgac.desktop
    DESTINATION ${KDE_INSTALL_AUTOSTARTDIR}
    )

install(FILES
    org.kde.korganizer.KOrgac.xml
    DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR}
    )
