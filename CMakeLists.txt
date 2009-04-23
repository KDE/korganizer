project(korganizer)
add_subdirectory( tests )

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5850)
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
                     ${CMAKE_SOURCE_DIR}/libkholidays
                     ${CMAKE_SOURCE_DIR}/mimelib
                     ${CMAKE_SOURCE_DIR}/korganizer/printing
                     ${CMAKE_SOURCE_DIR}/kdgantt1
                     ${CMAKE_BINARY_DIR}/libkdepim
                     ${QT_INCLUDES} ${ZLIB_INCLUDE_DIR} )

add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

add_subdirectory( printing )
add_subdirectory( korgac )
add_subdirectory( pixmaps )
add_subdirectory( sounds )
add_subdirectory( plugins )
add_subdirectory( interfaces )

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

target_link_libraries(korganizer ${KDE4_KDECORE_LIBS} korganizerprivate)

install(TARGETS korganizer  DESTINATION ${BIN_INSTALL_DIR} )


########### next target ###############

set(kcm_korganizer_PART_SRCS koprefsdialog.cpp )


kde4_add_plugin(kcm_korganizer ${kcm_korganizer_PART_SRCS})


target_link_libraries(kcm_korganizer ${KDE4_KDEUI_LIBS} korganizerprivate kdepim kholidays ${QT_QT3SUPPORT_LIBRARY} ${KDE4_KCAL_LIBS} ${KDE4_KRESOURCES_LIBS} korganizer_calendar)

kde4_add_kcfg_files(kcm_korganizer_SRCS koprefs_base.kcfgc )

install(TARGETS kcm_korganizer  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(korganizerpart_PART_SRCS korganizer_part.cpp )


kde4_add_plugin(korganizerpart ${korganizerpart_PART_SRCS})



target_link_libraries(korganizerpart  ${KDE4_KDECORE_LIBS} ${KDE4_KPARTS_LIBS} korganizerprivate korganizer_calendar 
   ${KDE4_KCAL_LIBS}
   korganizer_interfaces
)

install(TARGETS korganizerpart  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(korganizer_eventviewer_LIB_SRCS koeventviewer.cpp urihandler.cpp koeventviewerdialog.cpp )


qt4_add_dbus_interfaces(korganizer_eventviewer_LIB_SRCS org.kde.korganizer.Korganizer.xml)
qt4_add_dbus_interfaces(korganizer_eventviewer_LIB_SRCS ${CMAKE_BINARY_DIR}/kmail/org.kde.kmail.kmail.xml)
qt4_add_dbus_interfaces(korganizer_eventviewer_LIB_SRCS ${CMAKE_SOURCE_DIR}/knode/org.kde.knode.xml)
qt4_add_dbus_interfaces(korganizer_eventviewer_LIB_SRCS ${CMAKE_SOURCE_DIR}/kaddressbook/org.kde.KAddressbook.Core.xml)

kde4_add_library(korganizer_eventviewer SHARED ${korganizer_eventviewer_LIB_SRCS})
add_dependencies(korganizer_eventviewer kmail_xml)

target_link_libraries(korganizer_eventviewer ${KDE4_KPARTS_LIBS} ${KDE4_KCAL_LIBS})

set_target_properties(korganizer_eventviewer PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS korganizer_eventviewer  DESTINATION ${LIB_INSTALL_DIR})


########### next target ###############

set(korganizer_calendar_LIB_SRCS stdcalendar.cpp )


kde4_add_library(korganizer_calendar SHARED ${korganizer_calendar_LIB_SRCS})

target_link_libraries(korganizer_calendar ${KDE4_KPARTS_LIBS} ${KDE4_KCAL_LIBS} ${KDE4_KRESOURCES_LIBS} kdepim ${KDE4_KDE3SUPPORT_LIBS})

set_target_properties(korganizer_calendar PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS korganizer_calendar  DESTINATION ${LIB_INSTALL_DIR})


########### next target ###############

set(korganizerprivate_LIB_SRCS
   ${libkdgantt1_SRCS}
   themeimporter.cpp
   theme.cpp
   komessagebox.cpp
   koalternatelabel.cpp
   kodecorationlabel.cpp
   koagendaview.cpp
   timelabelszone.cpp
   timelabels.cpp
   timescaleconfigdialog.cpp
   agendaview.cpp
   multiagendaview.cpp
   koagenda.cpp
   koagendaitem.cpp
   timelineitem.cpp
   datenavigator.cpp
   kdatenavigator.cpp
   datenavigatorcontainer.cpp
   datechecker.cpp
   views/monthview/monthview.cpp
   views/monthview/monthitem.cpp
   views/monthview/monthgraphicsitems.cpp
   views/monthview/monthscene.cpp
   views/todoview/kotodomodel.cpp
   views/todoview/kotododelegates.cpp
   views/todoview/kotodoviewsortfilterproxymodel.cpp
   views/todoview/kotodoviewview.cpp
   views/todoview/kotodoview.cpp
   views/todoview/kotodoviewquicksearch.cpp
   views/todoview/kotodoviewquickaddline.cpp
   views/todoview/kcheckcombobox.cpp
   searchdialog.cpp
   calendarview.cpp
   koviewmanager.cpp
   kodialogmanager.cpp
   archivedialog.cpp
   kohelper.cpp
   komailclient.cpp
   kolistview.cpp
   koincidenceeditor.cpp
   koeventeditor.cpp
   kotodoeditor.cpp
   kojournaleditor.cpp
   koprefs.cpp
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
   kowhatsnextview.cpp
   kojournalview.cpp
   journalview.cpp
   kotimespentview.cpp
   kocore.cpp
   mailscheduler.cpp
   kodaymatrix.cpp
   docprefs.cpp
   statusdialog.cpp
   koglobals.cpp
   actionmanager.cpp
   resourceview.cpp
   navigatorbar.cpp
   kogroupware.cpp
   history.cpp
   aboutdata.cpp
   importdialog.cpp
   korganizerifaceimpl.cpp
   freebusymanager.cpp
   freebusyurldialog.cpp
   eventarchiver.cpp
   koidentitymanager.cpp
   exportwebdialog.cpp
   kocorehelper.cpp
   incidencechanger.cpp
   kotimelineview.cpp
   koattendeeeditor.cpp
   templatemanagementdialog.cpp )


kde4_add_kcfg_files(korganizerprivate_LIB_SRCS koprefs_base.kcfgc )

kde4_add_ui_files(korganizerprivate_LIB_SRCS filteredit_base.ui koeditoralarms_base.ui publishdialog_base.ui template_management_dialog_base.ui kogroupwareprefspage.ui searchdialog_base.ui timescaleedit_base.ui)

qt4_add_resources(korganizerprivate_LIB_SRCS agendaitem_pixmaps/agendaitemtheme.qrc )

qt4_add_dbus_adaptor(korganizerprivate_LIB_SRCS org.kde.korganizer.Korganizer.xml korganizerifaceimpl.h KOrganizerIfaceImpl )
qt4_add_dbus_adaptor(korganizerprivate_LIB_SRCS org.kde.Korganizer.Calendar.xml actionmanager.h ActionManager )
qt4_add_dbus_interfaces(korganizerprivate_LIB_SRCS org.kde.korganizer.Korganizer.xml )
qt4_add_dbus_interfaces(korganizerprivate_LIB_SRCS ${CMAKE_BINARY_DIR}/kmail/org.kde.kmail.kmail.xml)

kde4_add_library(korganizerprivate SHARED ${korganizerprivate_LIB_SRCS})

add_dependencies(korganizerprivate kmail)

target_link_libraries(korganizerprivate ${KDE4_KNEWSTUFF2_LIBS} ${KDE4_KABC_LIBS} ${KDE4_KUTILS_LIBS} 
   ${KDE4_KCAL_LIBS} ${KDE4_KPIMIDENTITIES_LIBRARY} ${KDE4_KMIME_LIBRARY} ${KDE4_KPIMUTILS_LIBS} ${KDE4_KRESOURCES_LIBS}
   kdepim korganizer_eventviewer korganizer_calendar kocorehelper ${ZLIB_LIBRARY}
   korg_stdprinting kholidays korganizer_interfaces ${KDE4_KDE3SUPPORT_LIBS}
)

set_target_properties(korganizerprivate PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )

install(TARGETS korganizerprivate  DESTINATION ${LIB_INSTALL_DIR})


########### install files ###############

install( FILES korganizer.knsrc  DESTINATION  ${CONFIG_INSTALL_DIR})
install( FILES korganizer.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install( FILES korganizer-import.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install( FILES korganizer.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES korganizer_part.rc  DESTINATION ${DATA_INSTALL_DIR}/korganizer)
install( FILES dbuscalendar.desktop  DESTINATION ${SERVICETYPES_INSTALL_DIR})
install( FILES tips  DESTINATION ${DATA_INSTALL_DIR}/korganizer)
install( FILES korganizer_configmain.desktop
               korganizer_configtime.desktop
               korganizer_configviews.desktop
               korganizer_configcolorsandfonts.desktop
               korganizer_configplugins.desktop
               korganizer_configgroupscheduling.desktop
               korganizer_configfreebusy.desktop
               korganizer_configdesignerfields.desktop
               webcal.protocol  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES korganizerui.rc  DESTINATION ${DATA_INSTALL_DIR}/korganizer)
install( FILES korganizer.upd  DESTINATION ${KCONF_UPDATE_INSTALL_DIR})
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/ical2vcal DESTINATION ${BIN_INSTALL_DIR})

install( FILES org.kde.Korganizer.Calendar.xml org.kde.korganizer.Korganizer.xml DESTINATION ${KDE4_DBUS_INTERFACES_DIR})
