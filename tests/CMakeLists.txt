set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

include_directories(
  ${CMAKE_SOURCE_DIR}/korganizer
  ${CMAKE_BINARY_DIR}/korganizer
  ${CMAKE_SOURCE_DIR}/calendarviews
  ${CMAKE_SOURCE_DIR}/korganizer/interfaces
)

########### next target ###############

set(korgplugins_SRCS korgplugins.cpp)

add_executable(korgplugins ${korgplugins_SRCS})

target_link_libraries(korgplugins
  korganizerprivate
  korganizer_core
  KF5::Holidays
  KF5::KIOCore
  Qt5::Test
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

########### next target ###############

add_executable(testkcmdesignerfields testkcmdesignerfields.cpp ../kcmdesignerfields.cpp)
set_target_properties(testkcmdesignerfields PROPERTIES COMPILE_FLAGS -DKDESRCDIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/\\"")
target_link_libraries(testkcmdesignerfields
  KF5::KIOCore
  KF5::KDELibs4Support
  Qt5::UiTools
)
