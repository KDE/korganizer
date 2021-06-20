########### next target ###############



set(kontact_specialdatesplugin_PART_SRCS specialdates_plugin.cpp sdsummarywidget.cpp)
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

kcoreaddons_desktop_to_json(kontact_specialdatesplugin specialdatesplugin.desktop)

########### next target ###############

set(kcmsdsummary_PART_SRCS kcmsdsummary.cpp)

ki18n_wrap_ui(kcmsdsummary_PART_SRCS sdsummaryconfig_base.ui)

add_library(kcmsdsummary MODULE ${kcmsdsummary_PART_SRCS})
kcoreaddons_desktop_to_json(kcmsdsummary "kcmsdsummary.desktop" SERVICE_TYPES kcmodule.desktop)
target_link_libraries(kcmsdsummary Qt::Widgets KF5::KCMUtils KF5::I18n)

########### install files ###############

install(TARGETS kontact_specialdatesplugin DESTINATION ${KDE_INSTALL_PLUGINDIR}/kontact5)
install(TARGETS kcmsdsummary DESTINATION ${KDE_INSTALL_PLUGINDIR}/pim/kcms/summary)

install(FILES specialdatesplugin.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/kontact)
install(FILES kcmsdsummary.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})

install(FILES specialdates.setdlg DESTINATION ${KDE_INSTALL_DATADIR}/kontact/ksettingsdialog)

