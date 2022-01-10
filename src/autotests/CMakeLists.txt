# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause
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
  Qt${QT_MAJOR_VERSION}::Test
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
  Qt${QT_MAJOR_VERSION}::Test
)

set(koeventpopupmenutest_SRCS ../koeventpopupmenu.cpp ../kocorehelper.cpp ../korganizer_debug.cpp)
set(koeventpopupmenutest_LIBS Qt${QT_MAJOR_VERSION}::Test
  Qt${QT_MAJOR_VERSION}::Gui
  Qt${QT_MAJOR_VERSION}::Core
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
