set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

########### next target ###############

add_executable(testkodaymatrix testkodaymatrix.cpp ../kodaymatrix.cpp)
add_test(NAME testkodaymatrix COMMAND testkodaymatrix)
ecm_mark_as_test(testkodaymatrix)
target_link_libraries(testkodaymatrix
  KF5::AkonadiCore
  KF5::CalendarCore
  KF5::CalendarSupport
  korganizer_core
  korganizerprivate
  Qt5::Test
)

add_executable(testkodaymatrix_us testkodaymatrix_us.cpp ../kodaymatrix.cpp)
add_test(NAME testkodaymatrix_us COMMAND testkodaymatrix_us)
ecm_mark_as_test(testkodaymatrix_us)
target_link_libraries(testkodaymatrix_us
  KF5::AkonadiCore
  KF5::CalendarCore
  KF5::CalendarSupport
  korganizer_core
  korganizerprivate
  Qt5::Test
)

set(koeventpopupmenutest_SRCS ../koeventpopupmenu.cpp ../kocorehelper.cpp ../dialog/noteeditdialog.cpp ../korganizer_debug.cpp)
set(koeventpopupmenutest_LIBS Qt5::Test
  Qt5::Gui
  Qt5::Core
  KF5::Mime
  KF5::AkonadiWidgets
  KF5::AkonadiCore
  KF5::AkonadiCalendar
  KF5::AkonadiNotes
  KF5::CalendarCore
  KF5::IncidenceEditor
  KF5::CalendarSupport
  korganizer_core
  korganizerprivate
)
add_akonadi_isolated_test_advanced(koeventpopupmenutest.cpp "${koeventpopupmenutest_SRCS}" "${koeventpopupmenutest_LIBS}")

add_executable(noteeditdialogtest noteeditdialogtest.cpp ../dialog/noteeditdialog.cpp ../korganizer_debug.cpp)
add_test(NAME noteeditdialogtest COMMAND noteeditdialogtest)
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
  KF5::IncidenceEditor
  KF5::CalendarSupport
  KF5::AkonadiWidgets
  korganizer_core
  korganizerprivate
  )
