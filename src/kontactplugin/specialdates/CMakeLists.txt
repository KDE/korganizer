# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause
########### next target ###############



set(kontact_specialdatesplugin_PART_SRCS specialdates_plugin.cpp sdsummarywidget.cpp specialdates_plugin.h sdsummarywidget.h)
ecm_qt_declare_logging_category(kontact_specialdatesplugin_PART_SRCS HEADER korganizer_kontactplugins_specialdates_debug.h IDENTIFIER KORGANIZER_KONTACTPLUGINS_SPECIALDATES_LOG CATEGORY_NAME org.kde.pim.korganizer_kontactplugins_specialdates
        DESCRIPTION "korganizer (kontactplugins korganizer special dates)"
        OLD_CATEGORY_NAMES log_korganizer_kontactplugins_specialdates
        EXPORT KORGANIZER
    )


add_library(kontact_specialdatesplugin MODULE ${kontact_specialdatesplugin_PART_SRCS})

set(_korganizerprivate_lib "korganizerprivate")

target_link_libraries(kontact_specialdatesplugin
  KF5::Contacts
  KF5::CalendarCore
  KF5::Holidays
  KF5::KontactInterface
  KF5::AkonadiCore
  KF5::AkonadiContact
  KF5::CalendarSupport
)

########### next target ###############

set(kcmsdsummary_PART_SRCS kcmsdsummary.cpp kcmsdsummary.h)

ki18n_wrap_ui(kcmsdsummary_PART_SRCS sdsummaryconfig_base.ui)

add_library(kcmsdsummary MODULE ${kcmsdsummary_PART_SRCS})
target_link_libraries(kcmsdsummary Qt${QT_MAJOR_VERSION}::Widgets KF5::KCMUtils KF5::I18n)

########### install files ###############

install(TARGETS kontact_specialdatesplugin DESTINATION ${KDE_INSTALL_PLUGINDIR}/kontact5)
install(TARGETS kcmsdsummary DESTINATION ${KDE_INSTALL_PLUGINDIR}/pim/kcms/summary)
