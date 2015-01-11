project(korganizer)

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )


# enable exception handling


include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/calendarsupport
  ${CMAKE_BINARY_DIR}/calendarsupport
  ${CMAKE_BINARY_DIR}/calendarviews
  ${CMAKE_SOURCE_DIR}/libkdepimdbusinterfaces
  ${CMAKE_BINARY_DIR}/libkdepimdbusinterfaces
  ${CMAKE_SOURCE_DIR}/incidenceeditor-ng
  ${CMAKE_SOURCE_DIR}/calendarviews
  ${CMAKE_SOURCE_DIR}/pimcommon/
  ${CMAKE_SOURCE_DIR}/libkdepim
  ${AKONADI_INCLUDE_DIR}
  ${Boost_INCLUDE_DIRS}
  ${ZLIB_INCLUDE_DIRS}
)

add_subdirectory(tests)
add_subdirectory(autotests)
add_subdirectory(interfaces)
if(KDEPIM_BUILD_DESKTOP)
  add_subdirectory(pixmaps)
  add_subdirectory(sounds)
  add_subdirectory(plugins)
  add_subdirectory(kcmconfigs)
endif()

set(PERL ${PERL_EXECUTABLE})
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/ical2vcal.cmake ${CMAKE_CURRENT_BINARY_DIR}/ical2vcal @ONLY
)

########### next target ###############

if(KDEPIM_BUILD_DESKTOP)
  set(korganizer_SRCS
    main.cpp
    korganizer.cpp
    koapp.cpp
    korganizer_debug.cpp
  )

  kde4_add_app_icon(korganizer_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/pixmaps/hi*-apps-korganizer.png")

  add_executable(korganizer ${korganizer_SRCS})

  target_link_libraries(korganizer
    KF5::Holidays
    KF5::KontactInterface
    KF5::AkonadiCore
    KF5::CalendarCore
    KF5::CalendarUtils
    kdepim
    kdepimdbusinterfaces
    korganizerprivate
    korganizer_interfaces
    korganizer_core
    calendarsupport
    KF5::AkonadiCalendar
  )

  install(TARGETS
    korganizer ${KDE_INSTALL_TARGETS_DEFAULT_ARGS}
  )

########### next target ###############

  set(kcm_korganizer_PART_SRCS
    kcmdesignerfields.cpp
    koprefsdialog.cpp
    kitemiconcheckcombo.cpp
    korganizer_debug.cpp
  )

  add_library(kcm_korganizer MODULE ${kcm_korganizer_PART_SRCS})
  generate_export_header(kcm_korganizer BASE_NAME kcm_korganizer) 

  target_link_libraries(kcm_korganizer
    KF5::AkonadiCalendar
    calendarsupport
    eventviews
    incidenceeditorsng
    kdepim
    korganizer_core
    korganizerprivate
    Qt5::UiTools
    KF5::AkonadiCore
    KF5::CalendarCore
    KF5::CalendarUtils
    KF5::Holidays
    KF5::MailTransport
    
    Qt5::UiTools
    pimcommon
  )

  install(TARGETS
    kcm_korganizer
    DESTINATION ${KDE_INSTALL_PLUGINDIR}
  )

########### next target ###############

  set(korganizerpart_PART_SRCS korganizer_part.cpp)

  add_library(korganizerpart MODULE ${korganizerpart_PART_SRCS})

  target_link_libraries(korganizerpart
    calendarsupport
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

endif()

########### next target ###############

set(korganizer_core_LIB_SRCS
  koprefs.cpp
  kocore.cpp
  korganizer_debug.cpp
)

kconfig_add_kcfg_files(korganizer_core_LIB_SRCS koprefs_base.kcfgc)

add_library(korganizer_core ${LIBRARY_TYPE} ${korganizer_core_LIB_SRCS})
generate_export_header(korganizer_core BASE_NAME korganizer_core)

target_link_libraries(korganizer_core
  calendarsupport
  eventviews
  incidenceeditorsng
  kdepim
  korganizer_interfaces
  KF5::AkonadiCore
  KF5::CalendarCore
  KF5::CalendarUtils
  KF5::Mime
  KF5::IdentityManagement
  
  KF5::KIOCore
)

set_target_properties(korganizer_core PROPERTIES
  VERSION ${KDEPIM_LIB_VERSION}
  SOVERSION ${KDEPIM_LIB_SOVERSION}
)

install(TARGETS
  korganizer_core ${KDE_INSTALL_TARGETS_DEFAULT_ARGS}
)

########### next target ###############

if(KDEPIM_BUILD_DESKTOP)

set(korganizerprivate_LIB_SRCS
    korganizer_debug.cpp
    korgstartup.cpp
    aboutdata.cpp
    actionmanager.cpp
    akonadicollectionview.cpp
    calendarview.cpp
    datechecker.cpp
    datenavigator.cpp
    datenavigatorcontainer.cpp
    exportwebdialog.cpp
    filtereditdialog.cpp
    htmlexportjob.cpp
    kdatenavigator.cpp
    kocheckableproxymodel.cpp
    kocorehelper.cpp
    kodaymatrix.cpp
    kodialogmanager.cpp
    koeventpopupmenu.cpp
    koeventview.cpp
    koeventviewerdialog.cpp
    koglobals.cpp
    kohelper.cpp
    korganizerifaceimpl.cpp
    koviewmanager.cpp
    kowindowlist.cpp
    navigatorbar.cpp
    searchdialog.cpp
    views/agendaview/koagendaview.cpp
    views/journalview/kojournalview.cpp
    views/listview/kolistview.cpp
    views/monthview/monthview.cpp
    views/multiagendaview/multiagendaview.cpp
    views/timelineview/kotimelineview.cpp
    views/timespentview/kotimespentview.cpp
    views/todoview/kotodoview.cpp
    views/whatsnextview/kowhatsnextview.cpp
  )

  kconfig_add_kcfg_files(korganizerprivate_LIB_SRCS htmlexportsettings.kcfgc)

  ki18n_wrap_ui(korganizerprivate_LIB_SRCS
    filteredit_base.ui
    kogroupwareprefspage.ui
    views/multiagendaview/multiagendaviewconfigwidget.ui
    searchdialog_base.ui
  )

  qt5_add_resources(korganizerprivate_LIB_SRCS
    agendaitem_pixmaps/agendaitemtheme.qrc)

  qt5_add_dbus_adaptor(korganizerprivate_LIB_SRCS
    org.kde.korganizer.Korganizer.xml korganizerifaceimpl.h
    KOrganizerIfaceImpl
  )

  qt5_add_dbus_adaptor(korganizerprivate_LIB_SRCS
    org.kde.Korganizer.Calendar.xml actionmanager.h
    ActionManager calendaradaptor KOrgCalendarAdaptor
  )

  qt5_add_dbus_interfaces(korganizerprivate_LIB_SRCS
    org.kde.korganizer.Korganizer.xml
  )

  qt5_add_dbus_interfaces(korganizerprivate_LIB_SRCS
    ${CMAKE_SOURCE_DIR}/korgac/org.kde.korganizer.KOrgac.xml
  )

  qt5_add_dbus_interfaces(korganizerprivate_LIB_SRCS
    ${CMAKE_SOURCE_DIR}/korganizer/org.kde.korganizer.Korganizer.xml
  )

  add_library(korganizerprivate ${LIBRARY_TYPE} ${korganizerprivate_LIB_SRCS})
  generate_export_header(korganizerprivate BASE_NAME korganizerprivate)

  target_link_libraries(korganizerprivate
    pimcommon
    akonadi_next
    KF5::AkonadiCalendar
    calendarsupport
    eventviews
    incidenceeditorsng
    kdepim
    kdepimdbusinterfaces
    kdgantt2
    korganizer_core
    korganizer_interfaces
    KF5::AkonadiCore
    KF5::Contacts
    KF5::CalendarCore
    KF5::CalendarUtils
    KF5::Holidays
    KF5::Mime
    KF5::IdentityManagement
    KF5::MailTransport
    KF5::KCMUtils
    KF5::NewStuff
    calendarsupportcollectionpage
    ${ZLIB_LIBRARIES}
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
    korganizer.kcfg
    DESTINATION ${KDE_INSTALL_KCFGDIR}
  )

  install(FILES
    korganizer_part.rc
    DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/korganizer
  )

  install(FILES
    dbuscalendar.desktop
    DESTINATION ${KDE_INSTALL_KSERVICETYPES5DIR}
  )

  install(FILES
    tips
    DESTINATION ${KDE_INSTALL_DATADIR}/korganizer
  )

  install(FILES
    korganizer_part.desktop webcal.protocol
    DESTINATION ${KDE_INSTALL_KSERVICES5DIR}
  )

  install(FILES
    korganizerui.rc
    DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/korganizer
  )

  install(FILES
    korganizer.upd
    DESTINATION ${KDE_INSTALL_KCONFUPDATEDIR}
  )

  install(FILES
    org.kde.Korganizer.Calendar.xml org.kde.korganizer.Korganizer.xml
    DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR}
  )

  install(FILES
    korganizer.knsrc
    DESTINATION ${KDE_INSTALL_CONFDIR}
  )

  install(PROGRAMS
    org.kde.korganizer.desktop korganizer-import.desktop
    DESTINATION ${KDE_INSTALL_APPDIR}
  )

  install(FILES
    korganizer.appdata.xml
    DESTINATION ${KDE_INSTALL_METAINFODIR}
  )

  install(PROGRAMS
    ${CMAKE_CURRENT_BINARY_DIR}/ical2vcal
    DESTINATION ${KDE_INSTALL_BINDIR}
  )

endif()
