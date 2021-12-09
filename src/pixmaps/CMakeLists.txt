# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause
file(GLOB korganizer_icons "*-apps-korganizer.*")
file(GLOB korg_todo_icons "*-apps-korg-todo.*")
file(GLOB korg_journal_icons "*-apps-korg-journal.*")
ecm_install_icons(ICONS
    ${korganizer_icons}
    ${korg_todo_icons}
    ${korg_journal_icons}
    DESTINATION ${KDE_INSTALL_ICONDIR}
    THEME hicolor
)
