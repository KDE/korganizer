# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause

########### next target ###############
add_library(hebrew MODULE)
target_sources(
    hebrew
    PRIVATE
        configdialog.cpp
        configdialog.h
        hebrew.cpp
        hebrew.h
        holiday.cpp
        holiday.h
        parsha.cpp
        parsha.h
)

ecm_qt_declare_logging_category(hebrew HEADER korganizer_hebrew_debug.h IDENTIFIER KORGANIZER_HEBREWPLUGIN_LOG CATEGORY_NAME org.kde.pim.korganizer_hebrew_plugin)

target_link_libraries(
    hebrew
    KPim6::EventViews
    KF6::Holidays
)

install(TARGETS hebrew DESTINATION ${KDE_INSTALL_PLUGINDIR}/pim6/korganizer)
