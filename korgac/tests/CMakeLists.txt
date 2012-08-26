set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

include_directories(
  ${CMAKE_SOURCE_DIR}/korganizer
  ${CMAKE_BINARY_DIR}/korganizer
  ${CMAKE_SOURCE_DIR}/korganizer/korgac
)

set(korganizer_xml ${CMAKE_SOURCE_DIR}/korganizer/org.kde.korganizer.Korganizer.xml)

########### next target ###############

set(testalarmdlg_SRCS testalarmdlg.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../alarmdialog.cpp ../mailclient.cpp)

qt4_add_dbus_interface(testalarmdlg_SRCS ${korganizer_xml}
  korganizer_interface
)

kde4_add_executable(testalarmdlg TEST ${testalarmdlg_SRCS})

target_link_libraries(testalarmdlg
  akonadi-calendar
  akonadi-kmime
  calendarsupport
  incidenceeditorsng
  kdepimdbusinterfaces
  kio
  kmime
  kpimutils
  korganizer_core
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_KCALCORE_LIBS}
  ${KDEPIMLIBS_KCALUTILS_LIBS}
  ${KDEPIMLIBS_KPIMIDENTITIES_LIBS}
  ${KDEPIMLIBS_MAILTRANSPORT_LIBS}
  ${KDE4_KDEUI_LIBS}
  ${KDE4_PHONON_LIBS}
)

