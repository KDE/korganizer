set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${CMAKE_SOURCE_DIR}/korganizer
                     ${CMAKE_BINARY_DIR}/korganizer
                     ${CMAKE_SOURCE_DIR}/calendarviews/eventviews
                     ${CMAKE_SOURCE_DIR}/korganizer/interfaces )

########### next target ###############

set(korgplugins_SRCS korgplugins.cpp )


kde4_add_executable(korgplugins TEST ${korgplugins_SRCS})
kde4_add_executable(testkodaymatrix TEST testkodaymatrix.cpp ../kodaymatrix.cpp)

target_link_libraries(korgplugins korganizerprivate korganizer_core ${KDE4_KIO_LIBS} ${KDEPIMLIBS_KHOLIDAYS_LIBS} )
target_link_libraries(testkodaymatrix korganizerprivate korganizer_core ${QT_QTTEST_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})

kde4_add_executable(testkcmdesignerfields TEST testkcmdesignerfields.cpp ../kcmdesignerfields.cpp )
target_link_libraries(testkcmdesignerfields
                      ${QT_QTUITOOLS_LIBRARY}
                      ${KDE4_KIO_LIBS}
                     )
