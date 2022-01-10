# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

include_directories(
  ${korganizer_SOURCE_DIR}/
  ${korganizer_BINARY_DIR}/
)

########### next target ###############

add_executable(korgplugins korgplugins.cpp)

target_link_libraries(korgplugins
  korganizerprivate
  korganizer_core
  KF5::Holidays
  KF5::KIOCore
)

########### next target ###############

add_executable(testkcmdesignerfields testkcmdesignerfields.cpp ../kcmdesignerfields.cpp ../korganizer_debug.cpp)
set_target_properties(testkcmdesignerfields PROPERTIES COMPILE_FLAGS -DTESTKCMDESIGNERCURRENTDIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/\\"")
target_link_libraries(testkcmdesignerfields
  KF5::KIOCore
  KF5::I18n
  Qt${QT_MAJOR_VERSION}::UiTools
  KF5::KCMUtils
  KF5::KIOWidgets
)
