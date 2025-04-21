# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

include_directories(
    ${korganizer_SOURCE_DIR}/
    ${korganizer_BINARY_DIR}/
)

########### next target ###############

add_executable(korgplugins korgplugins.cpp)

target_link_libraries(
    korgplugins
    korganizerprivate
    korganizer_core
    KF6::Holidays
    KF6::KIOCore
)
