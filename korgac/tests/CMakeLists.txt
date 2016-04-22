set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

include_directories(
    ${CMAKE_SOURCE_DIR}/korganizer
    ${CMAKE_BINARY_DIR}/korganizer
    )

set(korganizer_xml ${korganizer_SOURCE_DIR}/src/data/org.kde.korganizer.Korganizer.xml)

########### next target ###############

set(testalarmdlg_SRCS testalarmdlg.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../alarmdialog.cpp ../mailclient.cpp ${korganizer_BINARY_DIR}/korgac/koalarmclient_debug.cpp)

qt5_add_dbus_interface(testalarmdlg_SRCS ${korganizer_xml}
    korganizer_interface
    )

add_executable(testalarmdlg ${testalarmdlg_SRCS})

target_link_libraries(testalarmdlg
    KF5::AkonadiCalendar
    KF5::AkonadiMime
    KF5::CalendarSupport
    KF5::IncidenceEditor
    KF5::KdepimDBusInterfaces
    KF5::KIOCore
    KF5::Mime
    korganizer_core
    KF5::AkonadiCore
    KF5::CalendarCore
    KF5::CalendarUtils
    KF5::IdentityManagement
    KF5::MailTransport
    Phonon::phonon4qt5
    )
