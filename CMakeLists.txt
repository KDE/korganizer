project(korganizer)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5850)
add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )


# enable exception handling
kde_enable_exceptions()

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/calendarsupport
  ${CMAKE_BINARY_DIR}/calendarsupport
  ${CMAKE_SOURCE_DIR}/libkdepimdbusinterfaces
  ${CMAKE_BINARY_DIR}/libkdepimdbusinterfaces
  ${CMAKE_SOURCE_DIR}/incidenceeditor-ng
  ${CMAKE_SOURCE_DIR}/calendarviews
  ${CMAKE_SOURCE_DIR}/pimcommon/
  ${CMAKE_SOURCE_DIR}/libkdepim
  ${AKONADI_INCLUDE_DIR}
  ${Boost_INCLUDE_DIRS}
  ${QT_INCLUDES}
  ${ZLIB_INCLUDE_DIRS}
)

#REACTIVATE
#add_subdirectory(tests)
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
    korganizer ${INSTALL_TARGETS_DEFAULT_ARGS}
  )

########### next target ###############

  set(kcm_korganizer_PART_SRCS
    kcmdesignerfields.cpp
    koprefsdialog.cpp
    kitemiconcheckcombo.cpp
  )

  add_library(kcm_korganizer MODULE ${kcm_korganizer_PART_SRCS})

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
    
    ${QT_QTUITOOLS_LIBRARY}
  )

  install(TARGETS
    kcm_korganizer
    DESTINATION ${PLUGIN_INSTALL_DIR}
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
    DESTINATION ${PLUGIN_INSTALL_DIR}
  )

endif()

########### next target ###############

set(korganizer_core_LIB_SRCS
  koprefs.cpp
  kocore.cpp
)

kconfig_add_kcfg_files(korganizer_core_LIB_SRCS koprefs_base.kcfgc)

add_library(korganizer_core ${LIBRARY_TYPE} ${korganizer_core_LIB_SRCS})

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
  KF5::PimIdentities
  
  KF5::KIOCore
)

set_target_properties(korganizer_core PROPERTIES
  VERSION ${KDEPIM_LIB_VERSION}
  SOVERSION ${KDEPIM_LIB_SOVERSION}
)

install(TARGETS
  korganizer_core ${INSTALL_TARGETS_DEFAULT_ARGS}
)

########### next target ###############

if(KDEPIM_BUILD_DESKTOP)

set(korganizerprivate_LIB_SRCS
    aboutdata.cpp
    actionmanager.cpp
    akonadicollectionview.cpp
    calendarview.cpp
    datechecker.cpp
    datenavigator.cpp
    datenavigatorcontainer.cpp
    docprefs.cpp
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
    previewdialog.cpp
    publishdialog.cpp
    searchdialog.cpp
    statusdialog.cpp
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
    accountscalendarwidget.ui
    filteredit_base.ui
    kogroupwareprefspage.ui
    multiagendaviewconfigwidget.ui
    publishdialog_base.ui
    searchdialog_base.ui
    timescaleedit_base.ui
  )

  qt4_add_resources(korganizerprivate_LIB_SRCS
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
    KF5::Abc
    KF5::CalendarCore
    KF5::CalendarUtils
    KF5::Holidays
    KF5::Mime
    KF5::PimIdentities
    KF5::MailTransport
    KF5::KCMUtils
    KF5::NewStuff
    ${ZLIB_LIBRARIES}
  )

  set_target_properties(korganizerprivate PROPERTIES
    VERSION ${KDEPIM_LIB_VERSION}
    SOVERSION ${KDEPIM_LIB_SOVERSION}
  )

  install(TARGETS
    korganizerprivate ${INSTALL_TARGETS_DEFAULT_ARGS}
    LIBRARY NAMELINK_SKIP
  )

########### install files ###############

  install(FILES
    korganizer.kcfg
    DESTINATION ${KCFG_INSTALL_DIR}
  )

  install(FILES
    korganizer_part.rc
    DESTINATION ${DATA_INSTALL_DIR}/korganizer
  )

  install(FILES
    dbuscalendar.desktop
    DESTINATION ${SERVICETYPES_INSTALL_DIR}
  )

  install(FILES
    tips
    DESTINATION ${DATA_INSTALL_DIR}/korganizer
  )

  install(FILES
    korganizer_part.desktop webcal.protocol
    DESTINATION ${SERVICES_INSTALL_DIR}
  )

  install(FILES
    korganizerui.rc
    DESTINATION ${DATA_INSTALL_DIR}/korganizer
  )

  install(FILES
    korganizer.upd
    DESTINATION ${KCONF_UPDATE_INSTALL_DIR}
  )

  install(FILES
    org.kde.Korganizer.Calendar.xml org.kde.korganizer.Korganizer.xml
    DESTINATION ${DBUS_INTERFACES_INSTALL_DIR}
  )

  install(FILES
    korganizer.knsrc
    DESTINATION ${CONFIG_INSTALL_DIR}
  )

  install(PROGRAMS
    korganizer.desktop korganizer-import.desktop
    DESTINATION ${XDG_APPS_INSTALL_DIR}
  )

  install(PROGRAMS
    ${CMAKE_CURRENT_BINARY_DIR}/ical2vcal
    DESTINATION ${BIN_INSTALL_DIR}
  )

endif()
