project(korganizer)

find_package( Akonadi REQUIRED )

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
#
# KORG_NOPRINTER - turns-off printing support. Useful if KOrganizer is used
#                  in a situation where no printers would be accessible.
#
option(KORG_NOPRINTER "Turn-off printing support." FALSE)

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
if(KORG_NOPRINTER)
  add_definitions(-DKORG_NOPRINTER)
endif(KORG_NOPRINTER)

# enable exception handling
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

if(KMAIL_SUPPORTED)
#TODO: make korganizer buildable without kmail (i.e. w/o the kmail dbus stuff)
#TODO: in this case, pass -DKORG_NOKMAIL_DBUS. course, this reduces functionality

  add_subdirectory( tests )

  add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5850)
  include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
                      ${CMAKE_SOURCE_DIR}/mimelib
                      ${CMAKE_SOURCE_DIR}/korganizer/printing
                      ${CMAKE_SOURCE_DIR}/kdgantt1
                      ${CMAKE_BINARY_DIR}/libkdepim
                      ${AKONADI_INCLUDE_DIR}
                      ${Boost_INCLUDE_DIRS}
                      ${QT_INCLUDES} ${ZLIB_INCLUDE_DIR} )

  add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

if(NOT KORG_NOPRINTER)
  add_subdirectory( printing )
endif(NOT KORG_NOPRINTER)
  add_subdirectory( korgac )
  add_subdirectory( pixmaps )
  add_subdirectory( sounds )
  add_subdirectory( plugins )
  add_subdirectory( interfaces )
  add_subdirectory( kcmconfigs )

  set( PERL ${PERL_EXECUTABLE} )
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ical2vcal.cmake ${CMAKE_CURRENT_BINARY_DIR}/ical2vcal @ONLY)


  ########### next target ###############

  set(korganizer_SRCS
    main.cpp
    korganizer.cpp
    koapp.cpp
  )

  kde4_add_app_icon(korganizer_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/pixmaps/hi*-apps-korganizer.png")

  kde4_add_executable(korganizer ${korganizer_SRCS})

  target_link_libraries(korganizer ${KDE4_KDEUI_LIBS} ${KDE4_KDE3SUPPORT_LIBRARY} ${KDE4_KPARTS_LIBRARY} ${KDEPIMLIBS_KCAL_LIBS} ${KDEPIMLIBS_KHOLIDAYS_LIBS} kdepim korganizerprivate korganizer_interfaces)

  install(TARGETS korganizer ${INSTALL_TARGETS_DEFAULT_ARGS})


  ########### next target ###############

  set(kcm_korganizer_PART_SRCS koprefsdialog.cpp )

  kde4_add_plugin(kcm_korganizer ${kcm_korganizer_PART_SRCS})

  target_link_libraries(kcm_korganizer ${QT_QT3SUPPORT_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDEPIMLIBS_KCAL_LIBS} ${KDEPIMLIBS_KHOLIDAYS_LIBS} korganizerprivate kdepim )

  install(TARGETS kcm_korganizer  DESTINATION ${PLUGIN_INSTALL_DIR})


  ########### next target ###############

  set(korganizerpart_PART_SRCS korganizer_part.cpp )

  kde4_add_plugin(korganizerpart ${korganizerpart_PART_SRCS})

  target_link_libraries(korganizerpart ${KDE4_KDECORE_LIBS} ${KDE4_KPARTS_LIBS} ${KDEPIMLIBS_KCAL_LIBS} korganizerprivate korganizer_interfaces)

  install(TARGETS korganizerpart  DESTINATION ${PLUGIN_INSTALL_DIR})


  ########### next target ###############

  set(korganizer_eventviewer_LIB_SRCS koeventviewer.cpp urihandler.cpp koeventviewerdialog.cpp )


  qt4_add_dbus_interfaces(korganizer_eventviewer_LIB_SRCS org.kde.korganizer.Korganizer.xml)
  qt4_add_dbus_interfaces(korganizer_eventviewer_LIB_SRCS ${CMAKE_BINARY_DIR}/kmail/org.kde.kmail.kmail.xml)
  qt4_add_dbus_interfaces(korganizer_eventviewer_LIB_SRCS ${CMAKE_SOURCE_DIR}/knode/org.kde.knode.xml)
  qt4_add_dbus_interfaces(korganizer_eventviewer_LIB_SRCS ${CMAKE_SOURCE_DIR}/kaddressbook/org.kde.KAddressbook.Core.xml)

  kde4_add_library(korganizer_eventviewer SHARED ${korganizer_eventviewer_LIB_SRCS})
  add_dependencies(korganizer_eventviewer kmail_xml)

  target_link_libraries(korganizer_eventviewer ${KDE4_KPARTS_LIBS} ${KDEPIMLIBS_KCAL_LIBS} kdepim)

  set_target_properties(korganizer_eventviewer PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
  install(TARGETS korganizer_eventviewer ${INSTALL_TARGETS_DEFAULT_ARGS})


  ########### next target ###############

  set(korganizerprivate_LIB_SRCS
    ${libkdgantt1_SRCS}
    themeimporter.cpp
    theme.cpp
    komessagebox.cpp
    koalternatelabel.cpp
    kodecorationlabel.cpp
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
    views/todoview/kcheckcombobox.cpp
    views/timelineview/kotimelineview.cpp
    views/timelineview/timelineitem.cpp
    views/timespentview/kotimespentview.cpp
    views/whatsnextview/kowhatsnextview.cpp
    searchdialog.cpp
    calendarview.cpp
    koviewmanager.cpp
    kodialogmanager.cpp
    archivedialog.cpp
    komailclient.cpp
    koincidenceeditor.cpp
    koeventeditor.cpp
    kotodoeditor.cpp
    kojournaleditor.cpp
    kowindowlist.cpp
    koeditorgeneral.cpp
    koeditorgeneralevent.cpp
    koeditorgeneraltodo.cpp
    koeditorgeneraljournal.cpp
    koeditordetails.cpp
    koeditoralarms.cpp
    koeditorrecurrence.cpp
    koeditorattachments.cpp
    koeventpopupmenu.cpp
    koeditorfreebusy.cpp
    publishdialog.cpp
    koeventview.cpp
    filtereditdialog.cpp
    mailscheduler.cpp
    kodaymatrix.cpp
    docprefs.cpp
    statusdialog.cpp
    koglobals.cpp
    actionmanager.cpp
    navigatorbar.cpp
    kogroupware.cpp
    history.cpp
    aboutdata.cpp
    importdialog.cpp
    korganizerifaceimpl.cpp
    freebusymanager.cpp
    freebusyurldialog.cpp
    eventarchiver.cpp
    exportwebdialog.cpp
    kocorehelper.cpp
    incidencechanger.cpp
    koattendeeeditor.cpp
    templatemanagementdialog.cpp
    stdcalendar.cpp
    koprefs.cpp kocore.cpp kohelper.cpp koidentitymanager.cpp cellitem.cpp
    akonadicalendar.cpp
    akonadicollectionview.cpp
  )

  kde4_add_kcfg_files(korganizerprivate_LIB_SRCS koprefs_base.kcfgc )
  kde4_add_ui_files(korganizerprivate_LIB_SRCS filteredit_base.ui koeditoralarms_base.ui publishdialog_base.ui template_management_dialog_base.ui kogroupwareprefspage.ui searchdialog_base.ui timescaleedit_base.ui)

  qt4_add_resources(korganizerprivate_LIB_SRCS agendaitem_pixmaps/agendaitemtheme.qrc )

  qt4_add_dbus_adaptor(korganizerprivate_LIB_SRCS org.kde.korganizer.Korganizer.xml korganizerifaceimpl.h KOrganizerIfaceImpl )
  qt4_add_dbus_adaptor(korganizerprivate_LIB_SRCS org.kde.Korganizer.Calendar.xml actionmanager.h ActionManager )
  qt4_add_dbus_interfaces(korganizerprivate_LIB_SRCS org.kde.korganizer.Korganizer.xml )
  qt4_add_dbus_interfaces(korganizerprivate_LIB_SRCS ${CMAKE_BINARY_DIR}/kmail/org.kde.kmail.kmail.xml)

  kde4_add_library(korganizerprivate SHARED ${korganizerprivate_LIB_SRCS})

  add_dependencies(korganizerprivate kmail)

  target_link_libraries(korganizerprivate
    ${KDE4_KNEWSTUFF2_LIBS} ${KDE4_KUTILS_LIBS}  ${KDE4_KDE3SUPPORT_LIBS} ${ZLIB_LIBRARY}
    ${KDEPIMLIBS_KCAL_LIBS} ${KDEPIMLIBS_KABC_LIBS} ${KDEPIMLIBS_KPIMIDENTITIES_LIBRARY} ${KDE4_KMIME_LIBRARY}
    ${KDEPIMLIBS_KHOLIDAYS_LIBS}
    ${KDEPIMLIBS_AKONADI_LIBS}
    korganizer_eventviewer korganizer_interfaces korg_stdprinting
    kdepim
  )

  set_target_properties(korganizerprivate PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )

  install(TARGETS korganizerprivate ${INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)


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

  install( FILES org.kde.Korganizer.Calendar.xml org.kde.korganizer.Korganizer.xml DESTINATION ${KDE4_DBUS_INTERFACES_DIR})

else(KMAIL_SUPPORTED)
  message(STATUS "KOrganizer will NOT be built.")
endif(KMAIL_SUPPORTED)
