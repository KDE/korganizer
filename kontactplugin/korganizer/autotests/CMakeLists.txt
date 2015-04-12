set(summaryeventtest_SRCS  summaryeventtest.cpp ../summaryeventinfo.cpp)

add_executable( summaryeventtest ${summaryeventtest_SRCS} )
add_test( summaryeventtest summaryeventtest)
ecm_mark_as_test(summaryeventtest)

target_link_libraries(
  summaryeventtest
  KF5::CalendarCore
  KF5::CalendarUtils
  Qt5::Test
)
