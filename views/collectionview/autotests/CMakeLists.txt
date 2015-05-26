include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_SOURCE_DIR}/korganizer
)

set(reparentingmodeltest_SRCS
    reparentingmodeltest.cpp
    ../reparentingmodel.cpp
    modeltest.cpp
    ../../../korganizer_debug.cpp
)

add_executable(reparentingmodeltest ${reparentingmodeltest_SRCS})
add_test(reparentingmodeltest reparentingmodeltest)
ecm_mark_as_test(reparentingmodeltest)
ecm_mark_nongui_executable(reparentingmodeltest)
target_link_libraries(reparentingmodeltest
    Qt5::Test
    Qt5::Core
    Qt5::Gui
    KF5::KDELibs4Support
)
