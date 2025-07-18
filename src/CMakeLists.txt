# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause
add_definitions(-DTRANSLATION_DOMAIN=\"korganizer\")

include_directories(${CMAKE_CURRENT_BINARY_DIR})

if(BUILD_TESTING)
    add_subdirectory(tests)
    add_subdirectory(autotests)
endif()
add_subdirectory(interfaces)
add_subdirectory(pixmaps)
add_subdirectory(sounds)
add_subdirectory(kontactplugin)
add_subdirectory(views/collectionview)

########### next target ###############

set(korganizer_userfeedback_LIB)
if(TARGET KF6::UserFeedbackWidgets)
    set(korganizer_userfeedback_LIB KF6::UserFeedbackWidgets)
endif()

set(korganizer_common_SRCS)
ecm_qt_declare_logging_category(korganizer_common_SRCS HEADER korganizer_debug.h IDENTIFIER KORGANIZER_LOG CATEGORY_NAME org.kde.pim.korganizer
        DESCRIPTION "korganizer (korganizer)"
        OLD_CATEGORY_NAMES log_korganizer
        EXPORT KORGANIZER
)
ecm_qt_declare_logging_category(korganizer_common_SRCS HEADER korganizer_activities_debug.h IDENTIFIER KORGANIZER_ACIVITIES_LOG CATEGORY_NAME org.kde.pim.korganizer.activities
        DESCRIPTION "korganizer (korganizer) activities"
        EXPORT KORGANIZER
)
set(korganizer_SRCS
    main.cpp
    korganizer.cpp
    koapp.cpp
    plugininterface/korganizerplugininterface.cpp
    korganizer.h
    koapp.h
    plugininterface/korganizerplugininterface.h
    ${korganizer_common_SRCS}
)

file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/pixmaps/*-apps-korganizer.png")
ecm_add_app_icon(korganizer_SRCS ICONS ${ICONS_SRCS})

add_executable(korganizer ${korganizer_SRCS})
if(COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(
        korganizer
        PROPERTIES
            UNITY_BUILD
                ON
    )
endif()
target_link_libraries(
    korganizer
    KPim6::KontactInterface
    KF6::CalendarCore
    korganizerprivate
    korganizer_interfaces
    korganizer_core
    KPim6::PimCommonAkonadi
    KF6::Crash
    KF6::WindowSystem
    ${korganizer_userfeedback_LIB}
    KF6::IconThemes
)
if(TARGET KPim6::PimCommonActivities)
    target_link_libraries(korganizer KPim6::PimCommonActivities)
endif()

install(
    TARGETS
        korganizer
        ${KDE_INSTALL_TARGETS_DEFAULT_ARGS}
)

########### next target ###############
add_library(
    korganizer_kcm_static
    STATIC
    widgets/kitemiconcheckcombo.cpp
    prefs/kprefsdialog.cpp
    widgets/kitemiconcheckcombo.h
    prefs/kprefsdialog.h
    ${korganizer_common_SRCS}
)
set_property(
    TARGET
        korganizer_kcm_static
    PROPERTY
        POSITION_INDEPENDENT_CODE
            ON
)
target_link_libraries(
    korganizer_kcm_static
    KF6::KCMUtils
    KPim6::AkonadiCalendar
    KPim6::CalendarSupport
    KPim6::EventViews
    KPim6::IncidenceEditor
    korganizer_core
    korganizerprivate
    KF6::CalendarCore
    KF6::Holidays
    KPim6::MailTransport
    KPim6::PimCommon
    KPim6::AkonadiWidgets
    KF6::WindowSystem
    KF6::KIOWidgets
)
if(TARGET KPim6::PimCommonActivities)
    target_link_libraries(korganizer_kcm_static KPim6::PimCommonActivities)
endif()

add_library(
    korganizer_configcolorsandfonts
    MODULE
    prefs/koprefsdialogcolorsandfonts.cpp
    prefs/koprefsdialogcolorsandfonts.h
)
target_link_libraries(korganizer_configcolorsandfonts korganizer_kcm_static)
install(TARGETS korganizer_configcolorsandfonts DESTINATION ${KDE_INSTALL_PLUGINDIR}/pim6/kcms/korganizer)

add_library(
    korganizer_configfreebusy
    MODULE
    prefs/koprefsdialoggroupwarescheduling.h
    prefs/koprefsdialoggroupwarescheduling.cpp
)
target_link_libraries(korganizer_configfreebusy korganizer_kcm_static)
install(TARGETS korganizer_configfreebusy DESTINATION ${KDE_INSTALL_PLUGINDIR}/pim6/kcms/korganizer)

add_library(
    korganizer_configgroupscheduling
    MODULE
    prefs/koprefsdialogmain.h
    prefs/koprefsdialogmain.cpp
)
target_link_libraries(korganizer_configgroupscheduling korganizer_kcm_static)
install(TARGETS korganizer_configgroupscheduling DESTINATION ${KDE_INSTALL_PLUGINDIR}/pim6/kcms/korganizer)

add_library(
    korganizer_configmain
    MODULE
    prefs/koprefsdialoggroupscheduling.h
    prefs/koprefsdialoggroupscheduling.cpp
)
target_link_libraries(korganizer_configmain korganizer_kcm_static)
install(TARGETS korganizer_configmain DESTINATION ${KDE_INSTALL_PLUGINDIR}/pim6/kcms/korganizer)

add_library(
    korganizer_configplugins
    MODULE
    prefs/koprefsdialogplugins.cpp
    prefs/koprefsdialogplugins.h
)
target_link_libraries(korganizer_configplugins korganizer_kcm_static)
install(TARGETS korganizer_configplugins DESTINATION ${KDE_INSTALL_PLUGINDIR}/pim6/kcms/korganizer)

add_library(
    korganizer_configtime
    MODULE
    prefs/koprefsdialogtime.cpp
    prefs/koprefsdialogtime.h
)
target_link_libraries(korganizer_configtime korganizer_kcm_static)
install(TARGETS korganizer_configtime DESTINATION ${KDE_INSTALL_PLUGINDIR}/pim6/kcms/korganizer)

add_library(
    korganizer_configviews
    MODULE
    prefs/koprefsdialogviews.cpp
    prefs/koprefsdialogviews.h
)
target_link_libraries(korganizer_configviews korganizer_kcm_static)
install(TARGETS korganizer_configviews DESTINATION ${KDE_INSTALL_PLUGINDIR}/pim6/kcms/korganizer)

add_library(
    korganizer_userfeedback
    MODULE
    prefs/koprefsuserfeedback.cpp
    prefs/koprefsuserfeedback.h
)
target_link_libraries(
    korganizer_userfeedback
    korganizer_kcm_static
    ${korganizer_userfeedback_LIB}
)
install(TARGETS korganizer_userfeedback DESTINATION ${KDE_INSTALL_PLUGINDIR}/pim6/kcms/korganizer)

########### next target ###############

set(korganizerpart_PART_SRCS
    korganizer_part.cpp
    korganizer_part.h
    korganizer_debug.cpp
)

add_library(korganizerpart MODULE ${korganizerpart_PART_SRCS})
if(COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(
        korganizerpart
        PROPERTIES
            UNITY_BUILD
                ON
    )
endif()

target_link_libraries(
    korganizerpart
    KPim6::CalendarSupport
    korganizer_core
    korganizer_interfaces
    korganizerprivate
    KPim6::AkonadiCore
    KF6::CalendarCore
    KPim6::CalendarUtils
    KF6::KIOWidgets
)

install(TARGETS korganizerpart DESTINATION ${KDE_INSTALL_PLUGINDIR})

########### next target ###############

set(korganizer_core_LIB_SRCS
    prefs/koprefs.cpp
    kocore.cpp
    prefs/koprefs.h
    kocore.h
    ${korganizer_common_SRCS}
)

kconfig_add_kcfg_files(korganizer_core_LIB_SRCS settings/koprefs_base.kcfgc)

add_library(korganizer_core ${korganizer_core_LIB_SRCS})
generate_export_header(korganizer_core BASE_NAME korganizer_core)
if(COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(
        korganizer_core
        PROPERTIES
            UNITY_BUILD
                ON
    )
endif()
target_link_libraries(
    korganizer_core
    PUBLIC
        korganizer_interfaces
    PRIVATE
        KPim6::CalendarSupport
        KPim6::EventViews
        KPim6::IdentityManagementCore
        Qt::DBus
)

target_include_directories(
    korganizer_core
    PUBLIC
        "$<BUILD_INTERFACE:${korganizer_SOURCE_DIR};${korganizer_BINARY_DIR}>"
)

set_target_properties(
    korganizer_core
    PROPERTIES
        VERSION
            ${KDEPIM_LIB_VERSION}
        SOVERSION
            ${KDEPIM_LIB_SOVERSION}
)

install(
    TARGETS
        korganizer_core
        ${KDE_INSTALL_TARGETS_DEFAULT_ARGS}
    LIBRARY
        NAMELINK_SKIP
)

########### next target ###############
set(korganizerprivate_userfeedback_SRCS)
if(TARGET KF6::UserFeedbackWidgets)
    set(korganizerprivate_userfeedback_SRCS
        ${korganizerprivate_userfeedback_SRCS}
        userfeedback/userfeedbackmanager.cpp
        userfeedback/korganizeruserfeedbackprovider.cpp
        userfeedback/userfeedbackmanager.h
        userfeedback/korganizeruserfeedbackprovider.h
    )
endif()

set(korganizerprivate_LIB_SRCS
    ${korganizer_common_SRCS}
    ${korganizerprivate_userfeedback_SRCS}
    manageshowcollectionproperties.cpp
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
    kocorehelper.cpp
    kodaymatrix.cpp
    kodialogmanager.cpp
    koeventpopupmenu.cpp
    koeventview.cpp
    dialog/koeventviewerdialog.cpp
    koglobals.cpp
    kohelper.cpp
    impl/korganizerifaceimpl.cpp
    koviewmanager.cpp
    kowindowlist.cpp
    widgets/navigatorbar.cpp
    dialog/searchdialog.cpp
    views/agendaview/koagendaview.cpp
    views/journalview/kojournalview.cpp
    views/listview/kolistview.cpp
    views/monthview/monthview.cpp
    views/multiagendaview/multiagendaview.cpp
    views/timelineview/kotimelineview.cpp
    views/todoview/kotodoview.cpp
    views/whatsnextview/kowhatsnextview.cpp
    pimmessagebox.cpp
    korganizer.qrc
    agendaitem_pixmaps/agendaitemtheme.qrc
    manageshowcollectionproperties.h
    collectiongeneralpage.h
    aboutdata.h
    actionmanager.h
    akonadicollectionview.h
    views/collectionview/reparentingmodel.h
    views/collectionview/calendardelegate.h
    views/collectionview/quickview.h
    calendarview.h
    datechecker.h
    datenavigator.h
    datenavigatorcontainer.h
    dialog/filtereditdialog.h
    widgets/kdatenavigator.h
    kocorehelper.h
    kodaymatrix.h
    kodialogmanager.h
    koeventpopupmenu.h
    koeventview.h
    dialog/koeventviewerdialog.h
    koglobals.h
    kohelper.h
    impl/korganizerifaceimpl.h
    koviewmanager.h
    kowindowlist.h
    widgets/navigatorbar.h
    dialog/searchdialog.h
    views/agendaview/koagendaview.h
    views/journalview/kojournalview.h
    views/listview/kolistview.h
    views/monthview/monthview.h
    views/multiagendaview/multiagendaview.h
    views/timelineview/kotimelineview.h
    views/todoview/kotodoview.h
    views/whatsnextview/kowhatsnextview.h
    pimmessagebox.h
    korganizer_options.h
    collectionsortfilterproxymodel.h
    collectionsortfilterproxymodel.cpp
    whatsnew/whatsnewtranslations.cpp
    whatsnew/whatsnewtranslations.h
)

if(HAVE_ACTIVITY_SUPPORT)
    set(korganizerprivate_LIB_SRCS
        ${korganizerprivate_LIB_SRCS}
        activities/activitiesmanager.h
        activities/activitiesmanager.cpp
        activities/accountactivities.h
        activities/accountactivities.cpp
        activities/transportactivities.cpp
        activities/transportactivities.h
    )
    if(BUILD_TESTING)
        add_subdirectory(activities/autotests)
    endif()
endif()

set_property(
    SOURCE
        ${CMAKE_CURRENT_BINARY_DIR}/qrc_korganizer.cpp
    PROPERTY
        SKIP_AUTOMOC
            ON
)
set_property(
    SOURCE
        ${CMAKE_CURRENT_BINARY_DIR}/qrc_agendaitemtheme.cpp
    PROPERTY
        SKIP_AUTOMOC
            ON
)

ki18n_wrap_ui(korganizerprivate_LIB_SRCS
    ui/filteredit_base.ui
    ui/kogroupwareprefspage.ui
    views/multiagendaview/multiagendaviewconfigwidget.ui
    ui/searchdialog_base.ui
    views/collectionview/quickview.ui
)

qt_add_dbus_adaptor(korganizerprivate_LIB_SRCS
    data/org.kde.korganizer.Korganizer.xml impl/korganizerifaceimpl.h
    KOrganizerIfaceImpl
)

qt_add_dbus_adaptor(korganizerprivate_LIB_SRCS
    data/org.kde.Korganizer.Calendar.xml actionmanager.h
    ActionManager calendaradaptor KOrgCalendarAdaptor
)
qt_add_dbus_adaptor(korganizerprivate_LIB_SRCS
    data/org.kde.calendar.Calendar.xml actionmanager.h
    ActionManager calendarinterfaceadaptor CalendarInterfaceAdaptor
)

qt_add_dbus_interfaces(korganizerprivate_LIB_SRCS
    data/org.kde.korganizer.Korganizer.xml
)

qt_add_dbus_interfaces(korganizerprivate_LIB_SRCS
    ${korganizer_SOURCE_DIR}/src/data/org.kde.korganizer.Korganizer.xml
)

add_library(korganizerprivate ${korganizerprivate_LIB_SRCS})
if(COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(
        korganizerprivate
        PROPERTIES
            UNITY_BUILD
                ON
    )
    set_source_files_properties(
        ${CMAKE_CURRENT_BINARY_DIR}/qrc_korganizer.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/qrc_agendaitemtheme.cpp
        PROPERTIES
            SKIP_UNITY_BUILD_INCLUSION
                ON
    )
endif()

generate_export_header(korganizerprivate BASE_NAME korganizerprivate)

target_link_libraries(
    korganizerprivate
    PUBLIC
        KPim6::PimCommonAkonadi
    PRIVATE
        KPim6::CalendarSupport
        KPim6::EventViews
        KPim6::IncidenceEditor
        korganizer_core
        korganizer_interfaces
        KPim6::AkonadiCore
        KPim6::AkonadiCalendar
        KF6::Contacts
        KF6::CalendarCore
        KPim6::CalendarUtils
        KF6::Holidays
        KPim6::Mime
        KPim6::IdentityManagementCore
        KF6::KCMUtils
        KPim6::LdapWidgets
        KPim6::AkonadiWidgets
        KPim6::MailTransport
        KF6::IconThemes
        KF6::IconWidgets
        KF6::WindowSystem
        KF6::ItemViews
        KF6::JobWidgets
        KF6::KIOGui
        ${korganizer_userfeedback_LIB}
)

if(HAVE_ACTIVITY_SUPPORT)
    target_link_libraries(korganizerprivate PRIVATE KPim6::PimCommonActivities)
endif()

target_include_directories(
    korganizerprivate
    PUBLIC
        "$<BUILD_INTERFACE:${korganizer_SOURCE_DIR};${korganizer_BINARY_DIR}>"
)

set_target_properties(
    korganizerprivate
    PROPERTIES
        VERSION
            ${KDEPIM_LIB_VERSION}
        SOVERSION
            ${KDEPIM_LIB_SOVERSION}
)

install(
    TARGETS
        korganizerprivate
        ${KDE_INSTALL_TARGETS_DEFAULT_ARGS}
    LIBRARY
        NAMELINK_SKIP
)

########### install files ###############

install(FILES settings/korganizer.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})

install(
    FILES
        data/org.kde.Korganizer.Calendar.xml
        data/org.kde.korganizer.Korganizer.xml
    DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR}
)

install(
    PROGRAMS
        org.kde.korganizer.desktop
        korganizer-import.desktop
        korganizer-view.desktop
    DESTINATION ${KDE_INSTALL_APPDIR}
)

install(FILES org.kde.korganizer.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

configure_file(data/org.kde.korganizer.service.in org.kde.korganizer.service)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.korganizer.service DESTINATION ${KDE_INSTALL_DBUSSERVICEDIR})
