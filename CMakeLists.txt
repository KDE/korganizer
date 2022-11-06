# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause
cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
# Do NOT add quote
set(KDEPIM_DEV_VERSION )

# add an extra space
if(DEFINED KDEPIM_DEV_VERSION)
  set(KDEPIM_DEV_VERSION " ${KDEPIM_DEV_VERSION}")
endif()

set(PIM_VERSION "5.22.40")
set(RELEASE_SERVICE_VERSION "23.03.40")

set(KDEPIM_VERSION "${PIM_VERSION}${KDEPIM_DEV_VERSION} (${RELEASE_SERVICE_VERSION})")

project(korganizer VERSION ${PIM_VERSION})

set(KF5_MIN_VERSION "5.99.0")

find_package(ECM ${KF5_MIN_VERSION} CONFIG REQUIRED)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(ECMInstallIcons)
include(ECMSetupVersion)
include(ECMAddTests)

include(GenerateExportHeader)
include(ECMGenerateHeaders)

include(FeatureSummary)
include(KDEGitCommitHooks)
include(KDEClangFormat)
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h *.c)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
include(CheckFunctionExists)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMAddAppIcon)
include(ECMQtDeclareLoggingCategory)
include(ECMDeprecationSettings)


set(AKONADI_CONTACT_VERSION "5.21.40")
set(IDENTITYMANAGEMENT_LIB_VERSION "5.21.40")
set(KLDAP_LIB_VERSION "5.21.40")
set(KMAILTRANSPORT_LIB_VERSION "5.21.40")
set(CALENDARUTILS_LIB_VERSION "5.21.40")
set(AKONADICALENDAR_LIB_VERSION "5.21.40")
set(KONTACTINTERFACE_LIB_VERSION "5.21.40")
set(KMIME_LIB_VERSION "5.21.40")
set(KPIMTEXTEDIT_LIB_VERSION "5.21.40")
set(AKONADI_VERSION "5.21.40")

set(KDEPIM_LIB_VERSION "${PIM_VERSION}")
set(KDEPIM_LIB_SOVERSION "5")
set(AKONADINOTES_LIB_VERSION "5.21.40")

set(QT_REQUIRED_VERSION "5.15.2")
if (QT_MAJOR_VERSION STREQUAL "6")
    set(QT_REQUIRED_VERSION "6.4.0")
endif()
find_package(Qt${QT_MAJOR_VERSION} ${QT_REQUIRED_VERSION} CONFIG REQUIRED DBus Gui Widgets Test UiTools)

find_package(KUserFeedback 1.2.0 CONFIG) 
set_package_properties(KUserFeedback PROPERTIES DESCRIPTION "User Feedback lib" TYPE OPTIONAL PURPOSE "Allow to send Telemetry Information (optional).")

if (TARGET KUserFeedbackWidgets)
    add_definitions(-DWITH_KUSERFEEDBACK)
endif()

set(PIMCOMMON_LIB_VERSION "5.21.40")
set(LIBKDEPIM_LIB_VERSION "5.21.40")
set(LIBINCIDENCEEDITOR_LIB_VERSION "5.21.40")
set(CALENDARSUPPORT_LIB_VERSION "5.21.40")
set(EVENTVIEW_LIB_VERSION "5.21.40")

# Find KF5 package
find_package(KF5I18n ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5Config ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5ConfigWidgets ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5CoreAddons ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5Crash ${KF5_MIN_VERSION} REQUIRED)
find_package(KF5IconThemes ${KF5_MIN_VERSION} REQUIRED)
find_package(KF5ItemViews ${KF5_MIN_VERSION} REQUIRED)
find_package(KF5JobWidgets ${KF5_MIN_VERSION} REQUIRED)
find_package(KF5KCMUtils ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5NewStuff ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5Parts ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5WidgetsAddons ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5WindowSystem ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5XmlGui ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5Notifications ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5Holidays ${KF5_MIN_VERSION} CONFIG REQUIRED)

find_package(KF5DocTools ${KF5_MIN_VERSION})
set_package_properties(KF5DocTools PROPERTIES DESCRIPTION
    "Tools to generate documentation"
    TYPE OPTIONAL
)


# Find KdepimLibs Package
find_package(KF5PimTextEdit ${KPIMTEXTEDIT_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5Akonadi ${AKONADI_VERSION} CONFIG REQUIRED)
find_package(KF5Contacts ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5CalendarCore ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5AkonadiContact ${AKONADI_CONTACT_VERSION} CONFIG REQUIRED)
find_package(KF5IdentityManagement ${IDENTITYMANAGEMENT_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5MailTransportAkonadi ${KMAILTRANSPORT_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5CalendarUtils ${CALENDARUTILS_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5Ldap ${KLDAP_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5AkonadiCalendar ${AKONADICALENDAR_LIB_VERSION} CONFIG REQUIRED)

find_package(KF5KontactInterface ${KONTACTINTERFACE_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5Mime ${KMIME_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5AkonadiNotes ${AKONADINOTES_LIB_VERSION} CONFIG REQUIRED)

find_package(KF5PimCommonAkonadi ${PIMCOMMON_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5Libkdepim ${LIBKDEPIM_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5IncidenceEditor ${LIBINCIDENCEEDITOR_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5CalendarSupport ${CALENDARSUPPORT_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5EventViews ${EVENTVIEW_LIB_VERSION} CONFIG REQUIRED)

if(NOT APPLE)
  find_package(X11)
endif()

set(KDEPIM_HAVE_X11 ${X11_FOUND})
configure_file(config-korganizer.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-korganizer.h)
include_directories(${korganizer_SOURCE_DIR} ${korganizer_BINARY_DIR})
configure_file(korganizer-version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/korganizer-version.h @ONLY)


ecm_set_disabled_deprecation_versions(QT 5.15.2
    KF 5.99.0
)

option(USE_UNITY_CMAKE_SUPPORT "Use UNITY cmake support (speedup compile time)" OFF)

set(COMPILE_WITH_UNITY_CMAKE_SUPPORT OFF)
if (USE_UNITY_CMAKE_SUPPORT)
    set(COMPILE_WITH_UNITY_CMAKE_SUPPORT ON)
endif()


add_subdirectory(src)

ecm_qt_install_logging_categories(
        EXPORT KORGANIZER
        FILE korganizer.categories
        DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
        )

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
ki18n_install(po)

if(KF5DocTools_FOUND)
    kdoctools_install(po)
    add_subdirectory(doc)
endif()

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
