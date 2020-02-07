cmake_minimum_required(VERSION 3.5)
# Do NOT add quote
set(KDEPIM_DEV_VERSION)

# add an extra space
if(DEFINED KDEPIM_DEV_VERSION)
  set(KDEPIM_DEV_VERSION " ${KDEPIM_DEV_VERSION}")
endif()

set(KDEPIM_VERSION_NUMBER "5.13.3")
set(KDE_APPLICATIONS_VERSION "19.12.3")

set(KDEPIM_VERSION "${KDEPIM_VERSION_NUMBER}${KDEPIM_DEV_VERSION} (${KDE_APPLICATIONS_VERSION})")

project(korganizer VERSION ${KDEPIM_VERSION_NUMBER})

if(POLICY CMP0063)
  cmake_policy(SET CMP0063 NEW)
endif()
set(KF5_MIN_VERSION "5.63.0")

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


set(AKONADI_MIMELIB_VERSION "5.13.2")
set(AKONADI_CONTACT_VERSION "5.13.2")
set(IDENTITYMANAGEMENT_LIB_VERSION "5.13.2")
set(KLDAP_LIB_VERSION "5.13.2")
set(KMAILTRANSPORT_LIB_VERSION "5.13.2")
set(CALENDARUTILS_LIB_VERSION "5.13.2")
set(AKONADICALENDAR_LIB_VERSION "5.13.2")
set(KONTACTINTERFACE_LIB_VERSION "5.13.2")
set(KMIME_LIB_VERSION "5.13.2")
set(KPIMTEXTEDIT_LIB_VERSION "5.13.2")
set(AKONADI_VERSION "5.13.2")

set(KDEPIM_LIB_VERSION "${KDEPIM_VERSION_NUMBER}")
set(KDEPIM_LIB_SOVERSION "5")
set(AKONADINOTES_LIB_VERSION "5.13.2")

set(QT_REQUIRED_VERSION "5.11.0")
option(KDEPIM_ENTERPRISE_BUILD "Enable features specific to the enterprise branch, which are normally disabled. Also, it disables many components not needed for Kontact such as the Kolab client." FALSE)
find_package(Qt5 ${QT_REQUIRED_VERSION} CONFIG REQUIRED DBus Gui Widgets Test UiTools)
set(KDEPIM_APPS_LIB_VERSION_LIB "5.13.2")
set(PIMCOMMON_LIB_VERSION_LIB "5.13.2")
set(LIBKDEPIM_LIB_VERSION_LIB "5.13.2")
set(LIBINCIDENCEEDITOR_LIB_VERSION_LIB "5.13.2")
set(CALENDARSUPPORT_LIB_VERSION_LIB "5.13.2")
set(EVENTVIEW_LIB_VERSION_LIB "5.13.2")

find_package(KF5AkonadiSearch "5.13.2" CONFIG REQUIRED)
set_package_properties(KF5AkonadiSearch PROPERTIES DESCRIPTION "The Akonadi Search libraries" URL "https://www.kde.org" TYPE REQUIRED PURPOSE "Provides search capabilities in KMail and Akonadi")

# Find KF5 package
find_package(KF5Codecs ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5Completion ${KF5_MIN_VERSION} CONFIG REQUIRED)
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

find_package(KF5KdepimDBusInterfaces ${KDEPIM_APPS_LIB_VERSION_LIB} CONFIG REQUIRED)
find_package(KF5PimCommonAkonadi ${PIMCOMMON_LIB_VERSION_LIB} CONFIG REQUIRED)
find_package(KF5LibkdepimAkonadi ${LIBKDEPIM_LIB_VERSION_LIB} CONFIG REQUIRED)
find_package(KF5IncidenceEditor ${LIBINCIDENCEEDITOR_LIB_VERSION_LIB} CONFIG REQUIRED)
find_package(KF5CalendarSupport ${CALENDARSUPPORT_LIB_VERSION_LIB} CONFIG REQUIRED)
find_package(KF5EventViews ${EVENTVIEW_LIB_VERSION_LIB} CONFIG REQUIRED)
if(NOT APPLE)
  find_package(X11)
endif()

set(KDEPIM_HAVE_X11 ${X11_FOUND})
configure_file(config-korganizer.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-korganizer.h)
configure_file(korgac/config-enterprise.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-enterprise.h)
include_directories(${korganizer_SOURCE_DIR} ${korganizer_BINARY_DIR} ${korgac_SOURCE_DIR} ${korgac_BINARY_DIR})
configure_file(korganizer-version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/korganizer-version.h @ONLY)


if (EXISTS "${CMAKE_SOURCE_DIR}/.git")
   add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050e00)
   add_definitions(-DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x054000)
endif()

set(CMAKE_CXX_STANDARD 14)
#add_definitions(-DQT_NO_FOREACH)
add_subdirectory(src)
add_subdirectory(korgac)

install(FILES korganizer.renamecategories korganizer.categories  DESTINATION  ${KDE_INSTALL_LOGGINGCATEGORIESDIR})

add_subdirectory(doc)
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
