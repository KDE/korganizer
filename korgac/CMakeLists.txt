
add_subdirectory( pixmaps ) 
add_subdirectory( tests ) 

include_directories( ${CMAKE_SOURCE_DIR}/korganizer  )


########### next target ###############

set(korgac_SRCS 
   korgacmain.cpp 
   alarmdialog.cpp 
   alarmdockwindow.cpp 
   koalarmclient.cpp )

set(korganizer_xml  ${CMAKE_SOURCE_DIR}/korganizer/org.kde.korganizer.Korganizer.xml)
QT4_ADD_DBUS_INTERFACE( korgac_SRCS ${korganizer_xml} korganizer_interface )



qt4_add_dbus_adaptor(korgac_SRCS org.kde.korganizer.KOrgac.xml koalarmclient.h KOAlarmClient )


kde4_add_executable(korgac ${korgac_SRCS})

target_link_libraries(korgac  ${KDE4_KDEUI_LIBS} ${KDE4_PHONON_LIBS} korganizer_eventviewer)

install(TARGETS korgac  DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############

install( FILES korgac.desktop  DESTINATION ${AUTOSTART_INSTALL_DIR} )
install( FILES org.kde.korganizer.KOrgac.xml DESTINATION ${DBUS_INTERFACES_DIR} )
