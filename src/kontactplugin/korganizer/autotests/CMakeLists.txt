message(STATUS "Warning: the summaryevent tests needs to be ported to Akonadi")
return()

set(summaryeventtest_SRCS  summaryeventtest.cpp ../summaryeventinfo.cpp)

add_executable( summaryeventtest ${summaryeventtest_SRCS} )
add_test(NAME summaryeventtest COMMAND summaryeventtest)
ecm_mark_as_test(summaryeventtest)

target_link_libraries(
  summaryeventtest
  KF${KF_MAJOR_VERSION}::CalendarCore
  KF5::CalendarUtils
  Qt::Test
  KF${KF_MAJOR_VERSION}::I18n
)
