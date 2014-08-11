set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

include_directories(
  ${CMAKE_SOURCE_DIR}/korganizer
  ${CMAKE_BINARY_DIR}/korganizer
  ${CMAKE_SOURCE_DIR}/calendarviews
  ${CMAKE_SOURCE_DIR}/korganizer/interfaces
)

########### next target ###############

add_executable(testkodaymatrix testkodaymatrix.cpp ../kodaymatrix.cpp)

target_link_libraries(testkodaymatrix
  KF5::AkonadiCore
  KF5::CalendarCore
  calendarsupport
  korganizer_core
  korganizerprivate
  Qt5::Test
)
