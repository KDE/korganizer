include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

set(reparentingmodeltest_SRCS
    reparentingmodeltest.cpp
    ../reparentingmodel.cpp
)

kde4_add_unit_test(reparentingmodeltest NOGUI ${reparentingmodeltest_SRCS})
target_link_libraries(reparentingmodeltest
    ${QT_QTTEST_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${KDE4_KDECORE_LIBS}
)
