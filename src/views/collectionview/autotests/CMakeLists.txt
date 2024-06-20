# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${korganizer_SOURCE_DIR}/
)

add_executable(reparentingmodeltest ${reparentingmodeltest_SRCS})
target_sources(reparentingmodeltest PRIVATE
    reparentingmodeltest.cpp
    ../reparentingmodel.cpp
    ../../../korganizer_debug.cpp
   )
add_test(NAME reparentingmodeltest COMMAND reparentingmodeltest)
ecm_mark_as_test(reparentingmodeltest)
ecm_mark_nongui_executable(reparentingmodeltest)
target_link_libraries(reparentingmodeltest
    Qt::Test
    Qt::Core
    Qt::Gui
)
