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
  KF${KF_MAJOR_VERSION}::Holidays
  KF${KF_MAJOR_VERSION}::KIOCore
)

########### next target ###############
if (QT_MAJOR_VERSION STREQUAL "5")
add_executable(testkcmdesignerfields testkcmdesignerfields.cpp ../kcmdesignerfields.cpp ../korganizer_debug.cpp)
set_target_properties(testkcmdesignerfields PROPERTIES COMPILE_FLAGS -DTESTKCMDESIGNERCURRENTDIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/\\"")
target_link_libraries(testkcmdesignerfields
  KF${KF_MAJOR_VERSION}::KIOCore
  KF${KF_MAJOR_VERSION}::I18n
  Qt::UiTools
  KF${KF_MAJOR_VERSION}::KCMUtils
  KF${KF_MAJOR_VERSION}::KIOWidgets
)
endif()
