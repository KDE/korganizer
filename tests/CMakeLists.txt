set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

include_directories(
  ${CMAKE_SOURCE_DIR}/korganizer
  ${CMAKE_BINARY_DIR}/korganizer
  ${CMAKE_SOURCE_DIR}/calendarviews
  ${CMAKE_SOURCE_DIR}/korganizer/interfaces
)

########### next target ###############

set(korgplugins_SRCS korgplugins.cpp)

kde4_add_executable(korgplugins TEST ${korgplugins_SRCS})

target_link_libraries(korgplugins
  korganizerprivate
  korganizer_core
  ${KDEPIMLIBS_KHOLIDAYS_LIBS}
  ${KDE4_KIO_LIBS}
)

########### next target ###############

kde4_add_executable(testkodaymatrix TEST testkodaymatrix.cpp ../kodaymatrix.cpp)

target_link_libraries(testkodaymatrix
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_KCALCORE_LIBS}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${QT_QTTEST_LIBRARY}
  calendarsupport
  korganizer_core
  korganizerprivate
)

########### next target ###############

kde4_add_executable(testkcmdesignerfields TEST testkcmdesignerfields.cpp ../kcmdesignerfields.cpp)

target_link_libraries(testkcmdesignerfields
  ${KDE4_KIO_LIBS}
  ${QT_QTUITOOLS_LIBRARY}
)
