add_definitions(-DTRANSLATION_DOMAIN=\"korganizer\")

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    )

if(BUILD_TESTING)
    add_subdirectory(tests)
    add_subdirectory(autotests)
endif()
add_subdirectory(interfaces)
add_subdirectory(pixmaps)
add_subdirectory(sounds)
add_subdirectory(kcmconfigs)
add_subdirectory(kontactplugin)
add_subdirectory(views/collectionview)
add_subdirectory(kconf_update)

########### next target ###############

set(korganizer_common_SRCS)
ecm_qt_declare_logging_category(korganizer_common_SRCS HEADER korganizer_debug.h IDENTIFIER KORGANIZER_LOG CATEGORY_NAME org.kde.pim.korganizer)

set(korganizer_SRCS
    main.cpp
    korganizer.cpp
    koapp.cpp
    plugininterface/korganizerplugininterface.cpp
    ${korganizer_common_SRCS}
    )

file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/pixmaps/*-apps-korganizer.png")
ecm_add_app_icon(korganizer_SRCS ICONS ${ICONS_SRCS})

add_executable(korganizer ${korganizer_SRCS})

target_link_libraries(korganizer
    KF5::KontactInterface
    KF5::CalendarCore
    KF5::LibkdepimAkonadi
    KF5::KdepimDBusInterfaces
    korganizerprivate
    korganizer_interfaces
    korganizer_core
    KF5::PimCommonAkonadi
    KF5::Crash
    KF5::WindowSystem
    )

install(TARGETS
    korganizer ${KDE_INSTALL_TARGETS_DEFAULT_ARGS}
    )

########### next target ###############

set(kcm_korganizer_PART_SRCS
    kcmdesignerfields.cpp
    korganizer_debug.cpp
    prefs/koprefsdialog.cpp
    widgets/kitemiconcheckcombo.cpp
    )

add_library(kcm_korganizer MODULE ${kcm_korganizer_PART_SRCS})
generate_export_header(kcm_korganizer BASE_NAME kcm_korganizer)

target_link_libraries(kcm_korganizer
    KF5::AkonadiCalendar
    KF5::CalendarSupport
    KF5::EventViews
    KF5::IncidenceEditor
    KF5::LibkdepimAkonadi
    korganizer_core
    korganizerprivate
    Qt5::UiTools
    KF5::CalendarCore
    KF5::Holidays
    KF5::MailTransport
    KF5::PimCommon
    KF5::AkonadiWidgets
    KF5::WindowSystem
    )

install(TARGETS
    kcm_korganizer
    DESTINATION ${KDE_INSTALL_PLUGINDIR}
    )

########### next target ###############

set(korganizerpart_PART_SRCS korganizer_part.cpp korganizer_debug.cpp)

add_library(korganizerpart MODULE ${korganizerpart_PART_SRCS})

target_link_libraries(korganizerpart
    KF5::CalendarSupport
    korganizer_core
    korganizer_interfaces
    korganizerprivate
    KF5::AkonadiCore
    KF5::CalendarCore
    KF5::CalendarUtils
    )

install(TARGETS
    korganizerpart
    DESTINATION ${KDE_INSTALL_PLUGINDIR}
    )


########### next target ###############

set(korganizer_core_LIB_SRCS
    prefs/koprefs.cpp
    kocore.cpp
    ${korganizer_common_SRCS}
    )

kconfig_add_kcfg_files(korganizer_core_LIB_SRCS settings/koprefs_base.kcfgc)

add_library(korganizer_core ${korganizer_core_LIB_SRCS})
generate_export_header(korganizer_core BASE_NAME korganizer_core)

target_link_libraries(korganizer_core
    PUBLIC
    korganizer_interfaces
    PRIVATE
    KF5::CalendarSupport
    KF5::EventViews
    KF5::IdentityManagement
    )

target_include_directories(korganizer_core PUBLIC
    "$<BUILD_INTERFACE:${korganizer_SOURCE_DIR};${korganizer_BINARY_DIR}>"
    )

set_target_properties(korganizer_core PROPERTIES
    VERSION ${KDEPIM_LIB_VERSION}
    SOVERSION ${KDEPIM_LIB_SOVERSION}
    )

install(TARGETS
    korganizer_core ${KDE_INSTALL_TARGETS_DEFAULT_ARGS}
    )

########### next target ###############

set(korganizerprivate_LIB_SRCS
    ${korganizer_common_SRCS}
    manageshowcollectionproperties.cpp
    korgmigrateapplication.cpp
    collectiongeneralpage.cpp
    aboutdata.cpp
    actionmanager.cpp
    akonadicollectionview.cpp
    views/collectionview/reparentingmodel.cpp
    views/collectionview/calendardelegate.cpp
    views/collectionview/quickview.cpp
    calendarview.cpp
    datechecker.cpp
    datenavigator.cpp
    datenavigatorcontainer.cpp
    dialog/filtereditdialog.cpp
    widgets/kdatenavigator.cpp
    kocheckableproxymodel.cpp
    kocorehelper.cpp
    kodaymatrix.cpp
    kodialogmanager.cpp
    koeventpopupmenu.cpp
    dialog/noteeditdialog.cpp
    koeventview.cpp
    dialog/koeventviewerdialog.cpp
    koglobals.cpp
    kohelper.cpp
    impl/korganizerifaceimpl.cpp
    koviewmanager.cpp
    kowindowlist.cpp
    widgets/navigatorbar.cpp
    dialog/searchdialog.cpp
    helper/searchcollectionhelper.cpp
    views/agendaview/koagendaview.cpp
    views/journalview/kojournalview.cpp
    views/listview/kolistview.cpp
    views/monthview/monthview.cpp
    views/multiagendaview/multiagendaview.cpp
    views/timelineview/kotimelineview.cpp
    views/todoview/kotodoview.cpp
    views/whatsnextview/kowhatsnextview.cpp
    )

qt5_add_resources(korganizerprivate_LIB_SRCS korganizer.qrc)

set_property(SOURCE ${CMAKE_CURRENT_BINARY_DIR}/qrc_korganizer.cpp PROPERTY SKIP_AUTOMOC ON)
set_property(SOURCE ${CMAKE_CURRENT_BINARY_DIR}/qrc_agendaitemtheme.cpp PROPERTY SKIP_AUTOMOC ON)

ki18n_wrap_ui(korganizerprivate_LIB_SRCS
    ui/filteredit_base.ui
    ui/kogroupwareprefspage.ui
    views/multiagendaview/multiagendaviewconfigwidget.ui
    ui/searchdialog_base.ui
    views/collectionview/quickview.ui
    )

qt5_add_resources(korganizerprivate_LIB_SRCS
    agendaitem_pixmaps/agendaitemtheme.qrc)

qt5_add_dbus_adaptor(korganizerprivate_LIB_SRCS
    data/org.kde.korganizer.Korganizer.xml impl/korganizerifaceimpl.h
    KOrganizerIfaceImpl
    )

qt5_add_dbus_adaptor(korganizerprivate_LIB_SRCS
    data/org.kde.Korganizer.Calendar.xml actionmanager.h
    ActionManager calendaradaptor KOrgCalendarAdaptor
    )

qt5_add_dbus_interfaces(korganizerprivate_LIB_SRCS
    data/org.kde.korganizer.Korganizer.xml
    )

qt5_add_dbus_interfaces(korganizerprivate_LIB_SRCS
    ../korgac/org.kde.korganizer.KOrgac.xml
    )

qt5_add_dbus_interfaces(korganizerprivate_LIB_SRCS
    ${korganizer_SOURCE_DIR}/src/data/org.kde.korganizer.Korganizer.xml
    )

add_library(korganizerprivate ${korganizerprivate_LIB_SRCS})
generate_export_header(korganizerprivate BASE_NAME korganizerprivate)

target_link_libraries(korganizerprivate
    PUBLIC
    KF5::PimCommonAkonadi
    PRIVATE
    KF5::CalendarSupport
    KF5::EventViews
    KF5::IncidenceEditor
    KF5::LibkdepimAkonadi
    KF5::KdepimDBusInterfaces
    korganizer_core
    korganizer_interfaces
    KF5::AkonadiCore
    KF5::AkonadiCalendar
    KF5::AkonadiNotes
    KF5::AkonadiSearchPIM
    KF5::Contacts
    KF5::CalendarCore
    KF5::CalendarUtils
    KF5::Holidays
    KF5::Mime
    KF5::IdentityManagement
    KF5::KCMUtils
    KF5::NewStuff
    KF5::Ldap
    KF5::AkonadiWidgets
    KF5::IconThemes
    KF5::WindowSystem
    KF5::Notifications
    KF5::ItemViews
    )

target_include_directories(korganizerprivate PUBLIC
    "$<BUILD_INTERFACE:${korganizer_SOURCE_DIR};${korganizer_BINARY_DIR}>"
    )

set_target_properties(korganizerprivate PROPERTIES
    VERSION ${KDEPIM_LIB_VERSION}
    SOVERSION ${KDEPIM_LIB_SOVERSION}
    )

install(TARGETS
    korganizerprivate ${KDE_INSTALL_TARGETS_DEFAULT_ARGS}
    LIBRARY NAMELINK_SKIP
    )

########### install files ###############

install(FILES
    settings/korganizer.kcfg
    DESTINATION ${KDE_INSTALL_KCFGDIR}
    )

install(FILES
    dbuscalendar.desktop
    DESTINATION ${KDE_INSTALL_KSERVICETYPES5DIR}
    )

install(FILES
    korganizer_part.desktop data/webcal.protocol
    DESTINATION ${KDE_INSTALL_KSERVICES5DIR}
    )

install(FILES
    data/org.kde.Korganizer.Calendar.xml data/org.kde.korganizer.Korganizer.xml
    DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR}
    )

install(FILES
    data/korganizer.knsrc
    DESTINATION ${KDE_INSTALL_KNSRCDIR}
    )

install(PROGRAMS
    org.kde.korganizer.desktop korganizer-import.desktop
    DESTINATION ${KDE_INSTALL_APPDIR}
    )

install(FILES
    org.kde.korganizer.appdata.xml
    DESTINATION ${KDE_INSTALL_METAINFODIR}
    )

configure_file(data/org.kde.korganizer.service.in org.kde.korganizer.service)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/org.kde.korganizer.service
    DESTINATION ${KDE_INSTALL_DBUSSERVICEDIR}
    )

