set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${CMAKE_SOURCE_DIR}/korganizer ${CMAKE_SOURCE_DIR}/korganizer/korgac )


set(korganizer_xml  ${CMAKE_SOURCE_DIR}/korganizer/org.kde.korganizer.Korganizer.xml)

########### next target ###############
set(testalarmdlg_SRCS testalarmdlg.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../alarmdialog.cpp )
QT4_ADD_DBUS_INTERFACE( testalarmdlg_SRCS ${korganizer_xml} korganizer_interface )


kde4_add_executable(testalarmdlg TEST ${testalarmdlg_SRCS})

target_link_libraries(testalarmdlg  ${KDE4_KDEUI_LIBS} ${KDE4_PHONON_LIBS} ${KDEPIMLIBS_KCAL_LIBS} korganizer_eventviewer)

