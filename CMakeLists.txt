cmake_minimum_required(VERSION 3.5)
# Do NOT add quote
set(KDEPIM_DEV_VERSION alpha)

# add an extra space
if(DEFINED KDEPIM_DEV_VERSION)
  set(KDEPIM_DEV_VERSION " ${KDEPIM_DEV_VERSION}")
endif()

set(PIM_VERSION "5.15.40")
set(RELEASE_SERVICE_VERSION "20.11.80")

set(KDEPIM_VERSION "${PIM_VERSION}${KDEPIM_DEV_VERSION} (${RELEASE_SERVICE_VERSION})")

project(korganizer VERSION ${PIM_VERSION})

if(POLICY CMP0063)
  cmake_policy(SET CMP0063 NEW)
endif()
set(KF5_MIN_VERSION "5.73.0")

find_package(ECM ${KF5_MIN_VERSION} CONFIG REQUIRED)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(ECMInstallIcons)
include(ECMSetupVersion)
include(ECMAddTests)

include(GenerateExportHeader)
include(ECMGenerateHeaders)

include(FeatureSummary)
include(CheckFunctionExists)
include(ECMGeneratePriFile)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(ECMAddAppIcon)
include(ECMQtDeclareLoggingCategory)


set(AKONADI_MIMELIB_VERSION "5.15.40")
set(AKONADI_CONTACT_VERSION "5.15.40")
set(IDENTITYMANAGEMENT_LIB_VERSION "5.15.40")
set(KLDAP_LIB_VERSION "5.15.40")
set(KMAILTRANSPORT_LIB_VERSION "5.15.40")
set(CALENDARUTILS_LIB_VERSION "5.15.40")
set(AKONADICALENDAR_LIB_VERSION "5.15.40")
set(KONTACTINTERFACE_LIB_VERSION "5.15.40")
set(KMIME_LIB_VERSION "5.15.40")
set(KPIMTEXTEDIT_LIB_VERSION "5.15.40")
set(AKONADI_VERSION "5.15.40")
set(GRANTLEETHEME_LIB_VERSION "5.15.40")

set(KDEPIM_LIB_VERSION "${PIM_VERSION}")
set(KDEPIM_LIB_SOVERSION "5")
set(AKONADINOTES_LIB_VERSION "5.15.40")

set(QT_REQUIRED_VERSION "5.13.0")
option(KDEPIM_ENTERPRISE_BUILD "Enable features specific to the enterprise branch, which are normally disabled. Also, it disables many components not needed for Kontact such as the Kolab client." FALSE)
find_package(Qt5 ${QT_REQUIRED_VERSION} CONFIG REQUIRED DBus Gui Widgets Test UiTools)

find_package(KUserFeedback 1.0.0 CONFIG) # Needs Provider::describeDataSources()
set_package_properties(KUserFeedback PROPERTIES DESCRIPTION "User Feedback lib" TYPE OPTIONAL PURPOSE "Allow to send Telemetry Information (optional). It can be disable in apps.")

if (TARGET KUserFeedbackWidgets)
    add_definitions(-DWITH_KUSERFEEDBACK)
endif()

set(KDEPIM_APPS_LIB_VERSION "5.15.40")
set(PIMCOMMON_LIB_VERSION "5.15.40")
set(LIBKDEPIM_LIB_VERSION "5.15.40")
set(LIBINCIDENCEEDITOR_LIB_VERSION "5.15.40")
set(CALENDARSUPPORT_LIB_VERSION "5.15.40")
set(EVENTVIEW_LIB_VERSION "5.15.40")
set(AKONADI_SEARCH_VERSION "5.15.40")

find_package(KF5AkonadiSearch ${AKONADI_SEARCH_VERSION} CONFIG REQUIRED)
set_package_properties(KF5AkonadiSearch PROPERTIES DESCRIPTION "The Akonadi Search libraries" URL "https://www.kde.org" TYPE REQUIRED PURPOSE "Provides search capabilities in KMail and Akonadi")

# Find KF5 package
find_package(KF5Codecs ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5Config ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5ConfigWidgets ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5CoreAddons ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5Crash ${KF5_MIN_VERSION} REQUIRED)
find_package(KF5DBusAddons ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5DocTools ${KF5_MIN_VERSION} REQUIRED)
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

# Find KdepimLibs Package
find_package(KF5PimTextEdit ${KPIMTEXTEDIT_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5Akonadi ${AKONADI_VERSION} CONFIG REQUIRED)
find_package(KF5Contacts ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5CalendarCore ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5AkonadiContact ${AKONADI_CONTACT_VERSION} CONFIG REQUIRED)
find_package(KF5IdentityManagement ${IDENTITYMANAGEMENT_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5MailTransportAkonadi ${KMAILTRANSPORT_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5AkonadiMime ${AKONADI_MIMELIB_VERSION} CONFIG REQUIRED)
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
find_package(KF5GrantleeTheme ${GRANTLEETHEME_LIB_VERSION} CONFIG REQUIRED)
if(NOT APPLE)
  find_package(X11)
endif()

set(KDEPIM_HAVE_X11 ${X11_FOUND})
configure_file(config-korganizer.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-korganizer.h)
configure_file(korgac/config-enterprise.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-enterprise.h)
include_directories(${korganizer_SOURCE_DIR} ${korganizer_BINARY_DIR} ${korgac_SOURCE_DIR} ${korgac_BINARY_DIR})
configure_file(korganizer-version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/korganizer-version.h @ONLY)


add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050f00)
add_definitions(-DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x054A00)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#add_definitions(-DQT_NO_FOREACH)
#add_definitions(-DQT_NO_KEYWORDS)
add_subdirectory(src)
add_subdirectory(korgac)

ecm_qt_install_logging_categories(
        EXPORT KORGANIZER
        FILE korganizer.categories
        DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
        )

add_subdirectory(doc)
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
