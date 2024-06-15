########### next target ###############
add_library(picoftheday MODULE)

target_sources(picoftheday PRIVATE
    configdialog.cpp
    picoftheday.cpp
    element.cpp
    configdialog.h
    picoftheday.h
    element.h
    )
ecm_qt_declare_logging_category(picoftheday HEADER korganizer_picoftheday_plugin_debug.h IDENTIFIER KORGANIZERPICOFTHEDAYPLUGIN_LOG CATEGORY_NAME org.kde.pim.korganizer_picoftheday_plugins
    DESCRIPTION "kdepim-addons (korganizer picoftheday plugins)"
    OLD_CATEGORY_NAMES log_korganizer_picoftheday_plugins
    EXPORT KDEPIMADDONS
    )

if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(picoftheday PROPERTIES UNITY_BUILD ON)
endif()
target_link_libraries(picoftheday
    PRIVATE
    KPim6::EventViews
    KF6::KIOCore
    Qt::Xml
    )

install(TARGETS
    picoftheday
    DESTINATION ${KDE_INSTALL_PLUGINDIR}/pim6/korganizer
    )
