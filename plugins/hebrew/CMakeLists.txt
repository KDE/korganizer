
########### next target ###############
find_package(KF6KDELibs4Support ${KF_MIN_VERSION} CONFIG REQUIRED) #for the korganizer hebrew plugin
add_library(hebrew MODULE)
target_sources(hebrew PRIVATE
  qcalendarsystem.cpp
  configdialog.cpp
  converter.cpp
  hebrew.cpp
  holiday.cpp
  parsha.cpp
)

ecm_qt_declare_logging_category(hebrew HEADER korganizer_hebrew_debug.h IDENTIFIER KORGANIZER_HEBREWPLUGIN_LOG CATEGORY_NAME org.kde.pim.korganizer_hebrew_plugin)


target_link_libraries(hebrew
  KPim6::EventViews
  KF6::I18n
  KF6::KDELibs4Support
)

install(TARGETS
  hebrew
  DESTINATION ${KDE_INSTALL_PLUGINDIR}/korganizer
)
