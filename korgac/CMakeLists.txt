remove_definitions(-DKDE_DEFAULT_DEBUG_AREA=5850) #remove korganizer debug area
add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5890)

option(KORGAC_AKONADI_AGENT FALSE "Build Korgac as Akonadi agent instead of a stand-alone process")
if(KORGAC_AKONADI_AGENT)
  add_definitions(-DKORGAC_AKONADI_AGENT)
endif()

add_subdirectory(pixmaps)
add_subdirectory(tests)

include_directories(
  ${CMAKE_SOURCE_DIR}/korganizer
  ${CMAKE_BINARY_DIR}/korganizer
)

########### next target ###############

set(korgac_SRCS
  koalarmclient.cpp
)

if(KORGAC_AKONADI_AGENT)
  set(korgac_SRCS ${korgac_SRCS} korgacagent.cpp)
else()
  set(korgac_SRCS ${korgac_SRCS} korgacmain.cpp)
endif()

if(NOT WINCE)
  set(korgac_SRCS
    ${korgac_SRCS}
    alarmdialog.cpp
    alarmdockwindow.cpp
    mailclient.cpp
  )
endif()

set(korganizer_xml ${CMAKE_SOURCE_DIR}/korganizer/org.kde.korganizer.Korganizer.xml)

qt4_add_dbus_interface(korgac_SRCS ${korganizer_xml}
  korganizer_interface
)

qt4_add_dbus_adaptor(korgac_SRCS org.kde.korganizer.KOrgac.xml koalarmclient.h
  KOAlarmClient
)

if(RUNTIME_PLUGINS_STATIC)
  add_definitions(-DSERIALIZER_PLUGIN_STATIC)
endif()

kde4_add_app_icon(korgac_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/pixmap/hi*-apps-korganizer.png")

if(KORGAC_AKONADI_AGENT)
  if(NOT WINCE)
    kde4_add_plugin(korgac ${korgac_SRCS})
  else()
    kde4_add_plugin(korgac STATIC ${korgac_SRCS})
  endif()
else()
  kde4_add_executable(korgac ${korgac_SRCS})
  if(Q_WS_MAC)
    set_target_properties(korgac PROPERTIES
      MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/Info.plist.template
    )
    set_target_properties(korgac PROPERTIES
      MACOSX_BUNDLE_GUI_IDENTIFIER "org.kde.pim.korgac"
    )
    set_target_properties(korgac PROPERTIES
      MACOSX_BUNDLE_BUNDLE_NAME "KOrganizer Reminder Client"
    )
  endif()
endif()

target_link_libraries(korgac
  akonadi-calendar
  akonadi-kmime
  calendarsupport
  incidenceeditorsng
  kdepim
  kdepimdbusinterfaces
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_KCALCORE_LIBS}
  ${KDEPIMLIBS_KCALUTILS_LIBS}
  ${KDEPIMLIBS_KPIMIDENTITIES_LIBS}
  ${KDEPIMLIBS_MAILTRANSPORT_LIBS}
  ${KDE4_KDEUI_LIBS}
  ${KDE4_PHONON_LIBS}
)

if(RUNTIME_PLUGINS_STATIC)
  target_link_libraries(korgac akonadi_serializer_kcalcore)
endif()

if(NOT WINCE)
  target_link_libraries(korgac korganizer_core)
endif()

# TODO: move this to FindQt4.cmake
find_library(QT_QMAEMO5_LIBRARY QtMaemo5 HINTS ${QT_LIBRARY_DIR})
if(QT_QMAEMO5_LIBRARY)
  target_link_libraries(korgac ${QT_QMAEMO5_LIBRARY})
endif()

if(KORGAC_AKONADI_AGENT)
  install(TARGETS
    korgac
    DESTINATION ${PLUGIN_INSTALL_DIR}
  )
  install(FILES
    korgacagent.desktop
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/akonadi/agents
  )
else()
  install(TARGETS
    korgac ${INSTALL_TARGETS_DEFAULT_ARGS}
  )
endif()

########### install files ###############

install(PROGRAMS
  korgac.desktop
  DESTINATION ${AUTOSTART_INSTALL_DIR}
)

install(FILES
  org.kde.korganizer.KOrgac.xml
  DESTINATION ${DBUS_INTERFACES_INSTALL_DIR}
)
