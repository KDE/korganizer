cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
# Do NOT add quote
set(KDEPIM_DEV_VERSION)

# add an extra space
if(DEFINED KDEPIM_DEV_VERSION)
  set(KDEPIM_DEV_VERSION " ${KDEPIM_DEV_VERSION}")
endif()

set(PIM_VERSION "5.19.2")
set(RELEASE_SERVICE_VERSION "21.12.2")

set(KDEPIM_VERSION "${PIM_VERSION}${KDEPIM_DEV_VERSION} (${RELEASE_SERVICE_VERSION})")

project(korganizer VERSION ${PIM_VERSION})

set(KF5_MIN_VERSION "5.87.0")

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


set(AKONADI_CONTACT_VERSION "5.19.2")
set(IDENTITYMANAGEMENT_LIB_VERSION "5.19.2")
set(KLDAP_LIB_VERSION "5.19.2")
set(KMAILTRANSPORT_LIB_VERSION "5.19.2")
set(CALENDARUTILS_LIB_VERSION "5.19.2")
set(AKONADICALENDAR_LIB_VERSION "5.19.2")
set(KONTACTINTERFACE_LIB_VERSION "5.19.2")
set(KMIME_LIB_VERSION "5.19.2")
set(KPIMTEXTEDIT_LIB_VERSION "5.19.2")
set(AKONADI_VERSION "5.19.2")

set(KDEPIM_LIB_VERSION "${PIM_VERSION}")
set(KDEPIM_LIB_SOVERSION "5")
set(AKONADINOTES_LIB_VERSION "5.19.2")

set(QT_REQUIRED_VERSION "5.15.2")
find_package(Qt5 ${QT_REQUIRED_VERSION} CONFIG REQUIRED DBus Gui Widgets Test UiTools)

find_package(KUserFeedback 1.0.0 CONFIG) # Needs Provider::describeDataSources()
set_package_properties(KUserFeedback PROPERTIES DESCRIPTION "User Feedback lib" TYPE OPTIONAL PURPOSE "Allow to send Telemetry Information (optional). It can be disable in apps.")

if (TARGET KUserFeedbackWidgets)
    add_definitions(-DWITH_KUSERFEEDBACK)
endif()

set(PIMCOMMON_LIB_VERSION "5.19.2")
set(LIBKDEPIM_LIB_VERSION "5.19.2")
set(LIBINCIDENCEEDITOR_LIB_VERSION "5.19.2")
set(CALENDARSUPPORT_LIB_VERSION "5.19.2")
set(EVENTVIEW_LIB_VERSION "5.19.2")

# Find KF5 package
find_package(KF5I18n ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5Config ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5ConfigWidgets ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5CoreAddons ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5Crash ${KF5_MIN_VERSION} REQUIRED)
find_package(KF5DBusAddons ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5IconThemes ${KF5_MIN_VERSION} REQUIRED)
find_package(KF5ItemViews ${KF5_MIN_VERSION} REQUIRED)
find_package(KF5JobWidgets ${KF5_MIN_VERSION} REQUIRED)
find_package(KF5KCMUtils ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5NewStuff ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5Parts ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5Service ${KF5_MIN_VERSION} CONFIG REQUIRED)
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
find_package(Phonon4Qt5 CONFIG REQUIRED)

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
include_directories(${korganizer_SOURCE_DIR} ${korganizer_BINARY_DIR} ${korgac_SOURCE_DIR} ${korgac_BINARY_DIR})
configure_file(korganizer-version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/korganizer-version.h @ONLY)


add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050f02)
add_definitions(-DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x055500)

option(USE_UNITY_CMAKE_SUPPORT "Use UNITY cmake support (speedup compile time)" OFF)

set(COMPILE_WITH_UNITY_CMAKE_SUPPORT OFF)
if (USE_UNITY_CMAKE_SUPPORT)
    set(COMPILE_WITH_UNITY_CMAKE_SUPPORT ON)
endif()


add_subdirectory(src)
add_subdirectory(korgac)

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
