project(korganizer)

#
# The following macros can be defined to alter KOrganizer behavior.
# (if desired, use add_definitions() to define them)
#
# KORG_NOARCHIVE - turns-off the Archive dialog which allows incidences to be
#                  archived/deleted. No idea why you would want to define this.
#
option(KORG_NOARCHIVE "Turn-off the Archive dialog which allows incidences to be archived/deleted." FALSE)
# KORG_NODND     - turns-off the ability to drag-and-drop incidences in the
#                  views or between a view and the navigator. Probably necessary
#                  if KOrganizer is being used on small form factor devices.
option(KORG_NODND "Turn-off the ability to drag-and-drop incidences in the views or between a view and the date navigator." FALSE)
#
# KORG_NODECOS - turns-off decoration plugins in views.
#                No idea why you would want to define this, perhaps to save
#                screen real estate? But there are a config options for that.
option(KORG_NODECOS "Turn-off decoration plugins in views." FALSE)

# add C++ macro definitions for options passed to CMake
if(KORG_NOARCHIVE)
  add_definitions(-DKORG_NOARCHIVE)
endif(KORG_NOARCHIVE)
if(KORG_NODND)
  add_definitions(-DKORG_NODND)
endif(KORG_NODND)
if(KORG_NODECOS)
  add_definitions(-DKORG_NODECOS)
endif(KORG_NODECOS)

# enable exception handling
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

if (QT_QT3SUPPORT_FOUND)
add_subdirectory( tests )
endif (QT_QT3SUPPORT_FOUND)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5850)
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${CMAKE_SOURCE_DIR}/akonadi/kcal
                    ${CMAKE_BINARY_DIR}/akonadi/kcal
                    ${CMAKE_SOURCE_DIR}/akonadi/
                    ${CMAKE_SOURCE_DIR}/korganizer/printing
                    ${CMAKE_BINARY_DIR}/libkdepim
                    ${CMAKE_SOURCE_DIR}/libkdepimdbusinterfaces
                    ${CMAKE_BINARY_DIR}/libkdepimdbusinterfaces
                    ${CMAKE_SOURCE_DIR}/incidenceeditors
                    ${AKONADI_INCLUDE_DIR}
                    ${Boost_INCLUDE_DIRS}
                    ${QT_INCLUDES} ${ZLIB_INCLUDE_DIR} )

add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

add_subdirectory( interfaces )
add_subdirectory( korgac )
add_subdirectory( pixmaps )
add_subdirectory( sounds )
if (QT_QT3SUPPORT_FOUND)
add_subdirectory( plugins )
add_subdirectory( kcmconfigs )
endif (QT_QT3SUPPORT_FOUND)

set( PERL ${PERL_EXECUTABLE} )
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ical2vcal.cmake ${CMAKE_CURRENT_BINARY_DIR}/ical2vcal @ONLY)


########### next target ###############

if (QT_QT3SUPPORT_FOUND)
set(korganizer_SRCS
  main.cpp
  korganizer.cpp
  koapp.cpp
)

kde4_add_app_icon(korganizer_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/pixmaps/hi*-apps-korganizer.png")

kde4_add_executable(korganizer ${korganizer_SRCS})

target_link_libraries(korganizer
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KDE3SUPPORT_LIBRARY}
    ${KDE4_KPARTS_LIBRARY}
    ${KDEPIMLIBS_KCAL_LIBS}
    ${KDEPIMLIBS_AKONADI_KCAL_LIBS}
    ${KDEPIMLIBS_KHOLIDAYS_LIBS}
    ${KDEPIMLIBS_KONTACTINTERFACE_LIBS}
    ${KDEPIMLIBS_AKONADI_LIBS}
    kdepim
    kdepimdbusinterfaces
    korganizerprivate
    korganizer_interfaces
    korganizer_core
    akonadi_next
    akonadi-kcal_next
    incidenceeditors
)

install(TARGETS korganizer ${INSTALL_TARGETS_DEFAULT_ARGS})


########### next target ###############

set(kcm_korganizer_PART_SRCS
    kcmdesignerfields.cpp
    koprefsdialog.cpp
   )

kde4_add_plugin(kcm_korganizer ${kcm_korganizer_PART_SRCS} )

target_link_libraries(kcm_korganizer
  ${QT_QTUITOOLS_LIBRARY}
  ${KDE4_KDEUI_LIBS}
  ${KDEPIMLIBS_KCAL_LIBS} ${KDEPIMLIBS_KHOLIDAYS_LIBS} ${KDEPIMLIBS_AKONADI_LIBS} ${KDEPIMLIBS_AKONADI_KCAL_LIBS}
  mailtransport korganizerprivate korganizer_core kdepim incidenceeditors akonadi-kcal_next)

install(TARGETS kcm_korganizer  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(korganizerpart_PART_SRCS korganizer_part.cpp )

kde4_add_plugin(korganizerpart ${korganizerpart_PART_SRCS})

target_link_libraries(korganizerpart ${KDE4_KDECORE_LIBS} ${KDE4_KPARTS_LIBS} ${KDEPIMLIBS_KCAL_LIBS} ${KDEPIMLIBS_AKONADI_LIBS} ${KDEPIMLIBS_AKONADI_KCAL_LIBS} korganizer_interfaces akonadi-kcal_next korganizer_core korganizerprivate)

install(TARGETS korganizerpart  DESTINATION ${PLUGIN_INSTALL_DIR})
endif (QT_QT3SUPPORT_FOUND)

########### next target ###############

set(korganizer_core_LIB_SRCS
    koprefs.cpp
    kocore.cpp
    koidentitymanager.cpp
    categoryeditdialog.cpp
)

kde4_add_ui_files(korganizer_core_LIB_SRCS
  categoryeditdialog_base.ui
)

kde4_add_kcfg_files(korganizer_core_LIB_SRCS koprefs_base.kcfgc )

kde4_add_library(korganizer_core SHARED ${korganizer_core_LIB_SRCS})

target_link_libraries(korganizer_core ${KDE4_KDECORE_LIBS} ${KDEPIMLIBS_KPIMIDENTITIES_LIBRARY} ${KDE4_KMIME_LIBRARY} ${KDE4_KIO_LIBS} ${KDEPIMLIBS_KCAL_LIBS} ${KDEPIMLIBS_AKONADI_LIBS} ${KDEPIMLIBS_AKONADI_KCAL_LIBS} kdepim incidenceeditors akonadi-kcal_next korganizer_interfaces)

set_target_properties(korganizer_core PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )

install(TARGETS korganizer_core ${INSTALL_TARGETS_DEFAULT_ARGS})


########### next target ###############

if (QT_QT3SUPPORT_FOUND)
set(korganizerprivate_LIB_SRCS
  collectiongeneralpage.cpp
  koalternatelabel.cpp
  kodecorationlabel.cpp
  koeventviewerdialog.cpp
  timelabelszone.cpp
  timelabels.cpp
  timescaleconfigdialog.cpp
  datenavigator.cpp
  kdatenavigator.cpp
  datenavigatorcontainer.cpp
  datechecker.cpp
  views/agendaview/agendaview.cpp
  views/agendaview/koagenda.cpp
  views/agendaview/koagendaitem.cpp
  views/agendaview/koagendaview.cpp
  views/listview/kolistview.cpp
  views/journalview/kojournalview.cpp
  views/journalview/journalview.cpp
  views/monthview/monthview.cpp
  views/monthview/monthitem.cpp
  views/monthview/monthgraphicsitems.cpp
  views/monthview/monthscene.cpp
  views/multiagendaview/multiagendaview.cpp
  views/todoview/kotodomodel.cpp
  views/todoview/kotododelegates.cpp
  views/todoview/kotodoviewsortfilterproxymodel.cpp
  views/todoview/kotodoviewview.cpp
  views/todoview/kotodoview.cpp
  views/todoview/kotodoviewquicksearch.cpp
  views/todoview/kotodoviewquickaddline.cpp
  views/timelineview/kotimelineview.cpp
  views/timelineview/timelineitem.cpp
  views/timespentview/kotimespentview.cpp
  views/whatsnextview/kowhatsnextview.cpp
  searchdialog.cpp
  calendarview.cpp
  koviewmanager.cpp
  kodialogmanager.cpp
  archivedialog.cpp
  kowindowlist.cpp
  koeventpopupmenu.cpp
  publishdialog.cpp
  koeventview.cpp
  filtereditdialog.cpp
  kodaymatrix.cpp
  docprefs.cpp
  statusdialog.cpp
  koglobals.cpp
  actionmanager.cpp
  navigatorbar.cpp
  history.cpp
  aboutdata.cpp
  importdialog.cpp
  korganizerifaceimpl.cpp
  eventarchiver.cpp
  exportwebdialog.cpp
  kocorehelper.cpp
  kohelper.cpp
  cellitem.cpp
  akonadicollectionview.cpp
  htmlexportjob.cpp
  printing/calprinter.cpp
  printing/calprintpluginbase.cpp
  printing/calprintdefaultplugins.cpp
)

kde4_add_kcfg_files(korganizerprivate_LIB_SRCS htmlexportsettings.kcfgc )

kde4_add_ui_files(korganizerprivate_LIB_SRCS
  accountscalendarwidget.ui
  multiagendaviewconfigwidget.ui
  filteredit_base.ui publishdialog_base.ui kogroupwareprefspage.ui searchdialog_base.ui timescaleedit_base.ui
  printing/calprintincidenceconfig_base.ui printing/calprintdayconfig_base.ui printing/calprintmonthconfig_base.ui printing/calprinttodoconfig_base.ui printing/calprintweekconfig_base.ui
)

qt4_add_resources(korganizerprivate_LIB_SRCS agendaitem_pixmaps/agendaitemtheme.qrc )
qt4_add_dbus_adaptor(korganizerprivate_LIB_SRCS org.kde.korganizer.Korganizer.xml korganizerifaceimpl.h KOrganizerIfaceImpl )
qt4_add_dbus_adaptor(korganizerprivate_LIB_SRCS org.kde.Korganizer.Calendar.xml actionmanager.h ActionManager calendaradaptor KOrgCalendarAdaptor )
qt4_add_dbus_interfaces(korganizerprivate_LIB_SRCS org.kde.korganizer.Korganizer.xml )
qt4_add_dbus_interfaces(korganizerprivate_LIB_SRCS ${CMAKE_SOURCE_DIR}/korganizer/korgac/org.kde.korganizer.KOrgac.xml)
qt4_add_dbus_interfaces(korganizerprivate_LIB_SRCS ${CMAKE_SOURCE_DIR}/korganizer/org.kde.korganizer.Korganizer.xml)

kde4_add_library(korganizerprivate SHARED ${korganizerprivate_LIB_SRCS})

target_link_libraries(korganizerprivate
  ${KDE4_KNEWSTUFF2_LIBS} ${KDE4_KNEWSTUFF3_LIBS} ${KDE4_KCMUTILS_LIBS} ${KDE4_KPRINTUTILS_LIBS}  ${KDE4_KDE3SUPPORT_LIBS} ${ZLIB_LIBRARY}
  ${KDEPIMLIBS_KCAL_LIBS} ${KDEPIMLIBS_KABC_LIBS} ${KDEPIMLIBS_KPIMIDENTITIES_LIBRARY} ${KDE4_KMIME_LIBRARY}
  ${KDEPIMLIBS_KHOLIDAYS_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_AKONADI_KCAL_LIBS}
  ${KDEPIMLIBS_MAILTRANSPORT_LIBRARY}
  kdepim
  korganizer_interfaces
  kdepimdbusinterfaces
  korganizer_core
  akonadi-kcal_next
  akonadi_next
  incidenceeditors
  kdgantt2
)

set_target_properties(korganizerprivate PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )

install(TARGETS korganizerprivate ${INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)
endif (QT_QT3SUPPORT_FOUND)

########### install files ###############

install( FILES korganizer.knsrc  DESTINATION  ${CONFIG_INSTALL_DIR})
install( FILES korganizer.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install( FILES korganizer-import.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install( FILES korganizer.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES korganizer_part.rc  DESTINATION ${DATA_INSTALL_DIR}/korganizer)
install( FILES dbuscalendar.desktop  DESTINATION ${SERVICETYPES_INSTALL_DIR})
install( FILES tips  DESTINATION ${DATA_INSTALL_DIR}/korganizer)
install( FILES korganizer_part.desktop webcal.protocol  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES korganizerui.rc  DESTINATION ${DATA_INSTALL_DIR}/korganizer)
install( FILES korganizer.upd  DESTINATION ${KCONF_UPDATE_INSTALL_DIR})
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/ical2vcal DESTINATION ${BIN_INSTALL_DIR})

install( FILES org.kde.Korganizer.Calendar.xml org.kde.korganizer.Korganizer.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})
