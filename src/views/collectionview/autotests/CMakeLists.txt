include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${korganizer_SOURCE_DIR}/
)

set(reparentingmodeltest_SRCS
    reparentingmodeltest.cpp
    ../reparentingmodel.cpp
    ../../../korganizer_debug.cpp
)

add_executable(reparentingmodeltest ${reparentingmodeltest_SRCS})
add_test(NAME reparentingmodeltest COMMAND reparentingmodeltest)
ecm_mark_as_test(reparentingmodeltest)
ecm_mark_nongui_executable(reparentingmodeltest)
target_link_libraries(reparentingmodeltest
    Qt::Test
    Qt::Core
    Qt::Gui
)
