set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

########### next target ###############

add_executable(testkodaymatrix testkodaymatrix.cpp ../kodaymatrix.cpp)

target_link_libraries(testkodaymatrix
  KF5::AkonadiCore
  KF5::CalendarCore
  KF5::CalendarSupport
  korganizer_core
  korganizerprivate
  Qt5::Test
)

add_executable(koeventpopmenutest koeventpopupmenutest.cpp ../koeventpopupmenu.cpp ../kocorehelper.cpp ../dialog/noteeditdialog.cpp ../korganizer_debug.cpp)
add_test(koeventpopmenutest koeventpopmenutest)
ecm_mark_as_test(koeventpopmenutest)
target_link_libraries(koeventpopmenutest
  Qt5::Test
  Qt5::Gui
  Qt5::Core
  KF5::Mime
  KF5::AkonadiCore
  KF5::AkonadiCalendar
  KF5::AkonadiNotes
  KF5::CalendarCore
  incidenceeditorsng
  KF5::CalendarSupport
  korganizer_core
  korganizerprivate
)

add_executable(noteeditdialogtest noteeditdialogtest.cpp ../dialog/noteeditdialog.cpp ../korganizer_debug.cpp)
add_test(noteeditdialogtest noteeditdialogtest)
ecm_mark_as_test(noteeditdialogtest)
target_link_libraries(noteeditdialogtest
  Qt5::Test
  Qt5::Gui
  Qt5::Core
  KF5::Mime
  KF5::AkonadiCore
  KF5::AkonadiCalendar
  KF5::AkonadiNotes
  KF5::CalendarCore
  incidenceeditorsng
  KF5::CalendarSupport
  korganizer_core
  korganizerprivate
  )
