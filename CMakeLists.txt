project(korganizer)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/interfaces 
                     ${CMAKE_SOURCE_DIR}/libkpimidentities 
                     ${CMAKE_SOURCE_DIR}/libkholidays 
                     ${CMAKE_SOURCE_DIR}/mimelib ${CMAKE_SOURCE_DIR}/libkleo ${CMAKE_SOURCE_DIR}/korganizer/printing 
                     ${CMAKE_SOURCE_DIR}/kdgantt 
		     ${CMAKE_BINARY_DIR}/libkdepim 
                     ${QT_INCLUDES} )


add_subdirectory( printing ) 
add_subdirectory( korgac ) 
add_subdirectory( pixmaps ) 
add_subdirectory( sounds ) 
add_subdirectory( plugins ) 
add_subdirectory( interfaces ) 

set( PERL ${PERL_EXECUTABLE} )
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ical2vcal.cmake ${CMAKE_CURRENT_BINARY_DIR}/ical2vcal @ONLY)


########### next target ###############

set(korganizer_bin_SRCS
   main.cpp 
   korganizer.cpp 
   koapp.cpp 
)

kde4_automoc(${korganizer_bin_SRCS})

kde4_add_executable(korganizer_bin ${korganizer_bin_SRCS})

set_target_properties(korganizer_bin  PROPERTIES OUTPUT_NAME korganizer)

target_link_libraries(korganizer_bin ${KDE4_KDECORE_LIBS} korganizer)

install(TARGETS korganizer_bin  DESTINATION ${BIN_INSTALL_DIR} )


########### next target ###############

set(kcm_korganizer_PART_SRCS koprefsdialog.cpp )

kde4_automoc(${kcm_korganizer_PART_SRCS})

kde4_add_plugin(kcm_korganizer ${kcm_korganizer_PART_SRCS})



target_link_libraries(kcm_korganizer ${KDE4_KDECORE_LIBS} korganizer kdepim kholidays)

kde4_add_kcfg_files(kcm_korganizer_SRCS koprefs_base.kcfgc )

install(TARGETS kcm_korganizer  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(korganizerpart_PART_SRCS korganizer_part.cpp )

kde4_automoc(${korganizerpart_PART_SRCS})

kde4_add_plugin(korganizerpart WITH_PREFIX ${korganizerpart_PART_SRCS})



target_link_libraries(korganizerpart  ${KDE4_KDECORE_LIBS} korganizer )

install(TARGETS korganizerpart  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

if(KDE4_BUILD_TESTS)
set(korgplugins_SRCS korgplugins.cpp )

kde4_automoc(${korgplugins_SRCS})


kde4_add_executable(korgplugins ${korgplugins_SRCS})

target_link_libraries(korgplugins  korganizer ${KDE4_KIO_LIBS} )


########### next target ###############

set(timezone_SRCS timezone.cpp )

kde4_automoc(${timezone_SRCS})


kde4_add_executable(timezone ${timezone_SRCS})

target_link_libraries(timezone  korganizer ${KDE4_KIO_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(korganizer_eventviewer_LIB_SRCS koeventviewer.cpp urihandler.cpp koeventviewerdialog.cpp )

kde4_automoc(${korganizer_eventviewer_LIB_SRCS})

qt4_add_dbus_interfaces(korganizer_eventviewer_LIB_SRCS org.kde.korganizer.Korganizer.xml)
qt4_add_dbus_interfaces(korganizer_eventviewer_LIB_SRCS ${CMAKE_SOURCE_DIR}/kmail/org.kde.kmail.kmail.xml)
qt4_add_dbus_interfaces(korganizer_eventviewer_LIB_SRCS ${CMAKE_SOURCE_DIR}/knode/org.kde.knode.xml)
qt4_add_dbus_interfaces(korganizer_eventviewer_LIB_SRCS ${CMAKE_SOURCE_DIR}/kaddressbook/org.kde.KAddressbook.Core.xml)

kde4_add_library(korganizer_eventviewer SHARED ${korganizer_eventviewer_LIB_SRCS})

target_link_libraries(korganizer_eventviewer ${KDE4_KPARTS_LIBS} ${KDE4_KCAL_LIBS})

set_target_properties(korganizer_eventviewer PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS korganizer_eventviewer  DESTINATION ${LIB_INSTALL_DIR})


########### next target ###############

set(korganizer_calendar_LIB_SRCS stdcalendar.cpp )

kde4_automoc(${korganizer_calendar_LIB_SRCS})

kde4_add_library(korganizer_calendar SHARED ${korganizer_calendar_LIB_SRCS})

target_link_libraries(korganizer_calendar ${KDE4_KPARTS_LIBS} kdepim ${KDE4_KCAL_LIBS})

set_target_properties(korganizer_calendar PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS korganizer_calendar  DESTINATION ${LIB_INSTALL_DIR})


########### next target ###############

set(korganizer_LIB_SRCS 
   ${libkdgantt_SRCS}
   komessagebox.cpp 
   koagendaview.cpp 
   koagenda.cpp 
   koagendaitem.cpp 
   datenavigator.cpp 
   kdatenavigator.cpp 
   datenavigatorcontainer.cpp 
   datechecker.cpp 
   komonthview.cpp 
   searchdialog.cpp 
   calendarview.cpp 
   koviewmanager.cpp 
   kodialogmanager.cpp 
   archivedialog.cpp 
   kohelper.cpp 
   komailclient.cpp 
   kotodoview.cpp 
   kotodoviewitem.cpp 
   kotodoviewquicksearch.cpp 
   klistviewnewsearchline.cpp 
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
   journalentry.cpp 
   kocore.cpp 
   mailscheduler.cpp 
   kodaymatrix.cpp 
   docprefs.cpp 
   statusdialog.cpp 
   koglobals.cpp 
   konewstuff.cpp 
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
   templatemanagementdialog.cpp )

kde4_automoc(${korganizer_LIB_SRCS})

kde4_add_kcfg_files(korganizer_LIB_SRCS koprefs_base.kcfgc )

kde4_add_ui_files(korganizer_LIB_SRCS filteredit_base.ui koeditoralarms_base.ui publishdialog_base.ui template_management_dialog_base.ui kogroupwareprefspage.ui searchdialog_base.ui)

qt4_add_dbus_adaptor(korganizer_LIB_SRCS org.kde.korganizer.Korganizer.xml korganizerifaceimpl.h KOrganizerIfaceImpl )
qt4_add_dbus_adaptor(korganizer_LIB_SRCS org.kde.Korganizer.Calendar.xml actionmanager.h ActionManager )
qt4_add_dbus_interfaces(korganizer_LIB_SRCS org.kde.korganizer.Korganizer.xml )

kde4_add_library(korganizer SHARED ${korganizer_LIB_SRCS})

target_link_libraries(korganizer ${KDE4_KPARTS_LIBS} ${KDE4_KNEWSTUFF_LIBS} ${KDE4_KDEPRINT_LIBS} ${KDE4_KABC_LIBS} ${KDE4_KUTILS_LIBS} ${KDE4_KCAL_LIBS} kpimidentities
                                                      kdepim korganizer_eventviewer korganizer_calendar kocorehelper 
                                                      korg_stdprinting kholidays korganizer_interfaces )

set_target_properties(korganizer PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )

install(TARGETS korganizer  DESTINATION ${LIB_INSTALL_DIR})


########### install files ###############

install( FILES korganizer.desktop  DESTINATION ${XDG_APPS_DIR})
install( FILES korganizer-import.desktop  DESTINATION ${XDG_APPS_DIR})
install( FILES korganizer.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES korganizer_part.rc  DESTINATION ${DATA_INSTALL_DIR}/korganizer)
install( FILES dbuscalendar.desktop  DESTINATION ${SERVICETYPES_INSTALL_DIR})
install( FILES tips  DESTINATION ${DATA_INSTALL_DIR}/korganizer)
install( FILES korganizer_configmain.desktop                      korganizer_configtime.desktop                      korganizer_configviews.desktop                      korganizer_configcolors.desktop                      korganizer_configfonts.desktop                      korganizer_configplugins.desktop                      korganizer_configgroupscheduling.desktop                      korganizer_configfreebusy.desktop                      korganizer_configdesignerfields.desktop                      webcal.protocol  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES korganizerui.rc  DESTINATION ${DATA_INSTALL_DIR}/korganizer)
install( FILES korganizer.upd  DESTINATION ${KCONF_UPDATE_INSTALL_DIR})
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/ical2vcal DESTINATION ${BIN_INSTALL_DIR})

install( FILES org.kde.Korganizer.Calendar.xml org.kde.korganizer.Korganizer.xml DESTINATION ${DBUS_INTERFACES_DIR})

kde4_install_icons( ${ICON_INSTALL_DIR}   )
