# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

########### next target ###############

add_executable(testkodaymatrix testkodaymatrix.cpp testkodaymatrix.h ../kodaymatrix.cpp)
add_test(NAME testkodaymatrix COMMAND testkodaymatrix)
ecm_mark_as_test(testkodaymatrix)
target_link_libraries(testkodaymatrix
  KPim6::AkonadiCore
  KF6::CalendarCore
  KPim6::CalendarSupport
  korganizer_core
  korganizerprivate
  Qt::Test
)

add_executable(testkodaymatrix_us testkodaymatrix_us.cpp testkodaymatrix_us.h ../kodaymatrix.cpp)
add_test(NAME testkodaymatrix_us COMMAND testkodaymatrix_us)
ecm_mark_as_test(testkodaymatrix_us)
target_link_libraries(testkodaymatrix_us
  KPim6::AkonadiCore
  KF6::CalendarCore
  KPim6::CalendarSupport
  korganizer_core
  korganizerprivate
  Qt::Test
)

set(koeventpopupmenutest_SRCS ../koeventpopupmenu.cpp ../kocorehelper.cpp ../korganizer_debug.cpp)
set(koeventpopupmenutest_LIBS Qt::Test
  Qt::Gui
  Qt::Core
  KPim6::Mime
  KPim6::AkonadiWidgets
  KPim6::AkonadiCore
  KPim6::AkonadiCalendar
  KF6::CalendarCore
  KPim6::IncidenceEditor
  KPim6::CalendarSupport
  korganizer_core
  korganizerprivate
)
add_akonadi_isolated_test_advanced(koeventpopupmenutest.cpp "${koeventpopupmenutest_SRCS}" "${koeventpopupmenutest_LIBS}")

ecm_add_test(testtoggletodo.cpp
  LINK_LIBRARIES
    Qt::Test
    KF6::CalendarCore
    KPim6::IdentityManagementCore
    korganizer_core
    korganizerprivate
)
