message(STATUS "Warning: the summaryevent tests needs to be ported to Akonadi")
return()

set(summaryeventtest_SRCS  summaryeventtest.cpp ../summaryeventinfo.cpp)

add_executable( summaryeventtest ${summaryeventtest_SRCS} )
add_test(NAME summaryeventtest COMMAND summaryeventtest)
ecm_mark_as_test(summaryeventtest)

target_link_libraries(
  summaryeventtest
  KF6::CalendarCore
  KPim6::CalendarUtils
  Qt::Test
  KF6::I18n
)
