project(korganizer)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5850)
#add_definitions( -DQT_NO_CAST_FROM_ASCII )
#add_definitions( -DQT_NO_CAST_TO_ASCII )


# enable exception handling
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/calendarsupport
  ${CMAKE_BINARY_DIR}/calendarsupport
  ${CMAKE_SOURCE_DIR}/korganizer/printing
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

if(NOT WINCE)
  add_subdirectory(tests)
  add_subdirectory(interfaces)
  if(KDEPIM_BUILD_DESKTOP)
    add_subdirectory(pixmaps)
    add_subdirectory(sounds)
    add_subdirectory(plugins)
    add_subdirectory(kcmconfigs)
  endif()
endif()
add_subdirectory(korgac)

if(NOT WINCE)
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

    kde4_add_executable(korganizer ${korganizer_SRCS})

    target_link_libraries(korganizer
      ${KDE4_KDEUI_LIBS}
      ${KDE4_KPARTS_LIBRARY}
      ${KDEPIMLIBS_KHOLIDAYS_LIBS}
      ${KDEPIMLIBS_KONTACTINTERFACE_LIBS}
      ${KDEPIMLIBS_AKONADI_LIBS}
      ${KDEPIMLIBS_KCALCORE_LIBS}
      ${KDEPIMLIBS_KCALUTILS_LIBS}
      kdepim
      kdepimdbusinterfaces
      korganizerprivate
      korganizer_interfaces
      korganizer_core
      calendarsupport
      akonadi-calendar
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

    kde4_add_plugin(kcm_korganizer ${kcm_korganizer_PART_SRCS})

    target_link_libraries(kcm_korganizer
      akonadi-calendar
      calendarsupport
      eventviews
      incidenceeditorsng
      kdepim
      korganizer_core
      korganizerprivate
      ${KDEPIMLIBS_AKONADI_LIBS}
      ${KDEPIMLIBS_KCALCORE_LIBS}
      ${KDEPIMLIBS_KCALUTILS_LIBS}
      ${KDEPIMLIBS_KHOLIDAYS_LIBS}
      ${KDEPIMLIBS_MAILTRANSPORT_LIBS}
      ${KDE4_KDEUI_LIBS}
      ${QT_QTUITOOLS_LIBRARY}
    )

    install(TARGETS
      kcm_korganizer
      DESTINATION ${PLUGIN_INSTALL_DIR}
    )

########### next target ###############

    set(korganizerpart_PART_SRCS korganizer_part.cpp)

    kde4_add_plugin(korganizerpart ${korganizerpart_PART_SRCS})

    target_link_libraries(korganizerpart
      calendarsupport
      korganizer_core
      korganizer_interfaces
      korganizerprivate
      ${KDEPIMLIBS_AKONADI_LIBS}
      ${KDEPIMLIBS_KCALCORE_LIBS}
      ${KDEPIMLIBS_KCALUTILS_LIBS}
      ${KDE4_KDECORE_LIBS}
      ${KDE4_KPARTS_LIBS}
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

  kde4_add_kcfg_files(korganizer_core_LIB_SRCS koprefs_base.kcfgc)

  kde4_add_library(korganizer_core ${LIBRARY_TYPE} ${korganizer_core_LIB_SRCS})

  target_link_libraries(korganizer_core
    calendarsupport
    eventviews
    incidenceeditorsng
    kdepim
    korganizer_interfaces
    ${KDEPIMLIBS_AKONADI_LIBS}
    ${KDEPIMLIBS_KCALCORE_LIBS}
    ${KDEPIMLIBS_KCALUTILS_LIBS}
    ${KDEPIMLIBS_KMIME_LIBS}
    ${KDEPIMLIBS_KPIMIDENTITIES_LIBS}
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KIO_LIBS}
  )

  set_target_properties(korganizer_core PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
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
      printing/calprintdefaultplugins.cpp
      printing/calprinter.cpp
      printing/calprintpluginbase.cpp
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

    kde4_add_kcfg_files(korganizerprivate_LIB_SRCS htmlexportsettings.kcfgc)

    kde4_add_ui_files(korganizerprivate_LIB_SRCS
      accountscalendarwidget.ui
      filteredit_base.ui
      kogroupwareprefspage.ui
      multiagendaviewconfigwidget.ui
      publishdialog_base.ui
      searchdialog_base.ui
      timescaleedit_base.ui
      printing/calprintdayconfig_base.ui
      printing/calprintincidenceconfig_base.ui
      printing/calprintmonthconfig_base.ui
      printing/calprinttodoconfig_base.ui
      printing/calprintweekconfig_base.ui
    )

    qt4_add_resources(korganizerprivate_LIB_SRCS
      agendaitem_pixmaps/agendaitemtheme.qrc)

    qt4_add_dbus_adaptor(korganizerprivate_LIB_SRCS
      org.kde.korganizer.Korganizer.xml korganizerifaceimpl.h
      KOrganizerIfaceImpl
    )

    qt4_add_dbus_adaptor(korganizerprivate_LIB_SRCS
      org.kde.Korganizer.Calendar.xml actionmanager.h
      ActionManager calendaradaptor KOrgCalendarAdaptor
    )

    qt4_add_dbus_interfaces(korganizerprivate_LIB_SRCS
      org.kde.korganizer.Korganizer.xml
    )

    qt4_add_dbus_interfaces(korganizerprivate_LIB_SRCS
      ${CMAKE_SOURCE_DIR}/korganizer/korgac/org.kde.korganizer.KOrgac.xml
    )

    qt4_add_dbus_interfaces(korganizerprivate_LIB_SRCS
      ${CMAKE_SOURCE_DIR}/korganizer/org.kde.korganizer.Korganizer.xml
    )

    kde4_add_library(korganizerprivate ${LIBRARY_TYPE} ${korganizerprivate_LIB_SRCS})

    target_link_libraries(korganizerprivate
      pimcommon
      akonadi_next
      akonadi-calendar
      calendarsupport
      eventviews
      incidenceeditorsng
      kdepim
      kdepimdbusinterfaces
      kdgantt2
      korganizer_core
      korganizer_interfaces
      ${KDEPIMLIBS_AKONADI_CALENDAR_LIBS}
      ${KDEPIMLIBS_AKONADI_LIBS}
      ${KDEPIMLIBS_KABC_LIBS}
      ${KDEPIMLIBS_KCALCORE_LIBS}
      ${KDEPIMLIBS_KCALUTILS_LIBS}
      ${KDEPIMLIBS_KHOLIDAYS_LIBS}
      ${KDEPIMLIBS_KMIME_LIBS}
      ${KDEPIMLIBS_KPIMIDENTITIES_LIBS}
      ${KDEPIMLIBS_MAILTRANSPORT_LIBS}
      ${KDE4_KCMUTILS_LIBS}
      ${KDE4_KNEWSTUFF3_LIBS}
      ${KDE4_KPRINTUTILS_LIBS}
      ${ZLIB_LIBRARIES}
    )

    set_target_properties(korganizerprivate PROPERTIES
      VERSION ${GENERIC_LIB_VERSION}
      SOVERSION ${GENERIC_LIB_SOVERSION}
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

endif()
