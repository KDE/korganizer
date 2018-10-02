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

ecm_install_icons(ICONS
    16-actions-smallclock.png
    16-actions-upindicator.png
    22-actions-checkmark.png
    DESTINATION ${KDE_INSTALL_ICONDIR}
    THEME oxygen
)

