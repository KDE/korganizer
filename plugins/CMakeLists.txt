# SPDX-FileCopyrightText: 2015-2026 Laurent Montel <montel@kde.org>
# SPDX-License-Identifier: BSD-3-Clause
add_definitions(-DTRANSLATION_DOMAIN=\"korganizer_calendarplugins\")

add_subdirectory(datenums)
add_subdirectory(lunarphases)
add_subdirectory(picoftheday)
add_subdirectory(thisdayinhistory)

if(${KF6Holidays_VERSION} VERSION_GREATER_EQUAL "6.29.0")
    add_subdirectory(hebrewcalendar)
endif()
