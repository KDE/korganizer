# SPDX-FileCopyrightText: 2015-2026 Laurent Montel <montel@kde.org>
# SPDX-License-Identifier: BSD-3-Clause
add_definitions(-DTRANSLATION_DOMAIN=\"korganizer_calendarplugins\")

add_subdirectory(datenums)
#the hebrew plugin depends on kdelibs4support
#add_subdirectory(hebrew)
add_subdirectory(lunarphases)
add_subdirectory(picoftheday)
add_subdirectory(thisdayinhistory)
