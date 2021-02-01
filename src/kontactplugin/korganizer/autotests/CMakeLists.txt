message(STATUS "Warning: the summaryevent tests needs to be ported to Akonadi")
return()

set(summaryeventtest_SRCS  summaryeventtest.cpp ../summaryeventinfo.cpp)

add_executable( summaryeventtest ${summaryeventtest_SRCS} )
add_test(NAME summaryeventtest COMMAND summaryeventtest)
ecm_mark_as_test(summaryeventtest)

target_link_libraries(
  summaryeventtest
  KF5::CalendarCore
  KF5::CalendarUtils
  Qt5::Test
  KF5::I18n
)
