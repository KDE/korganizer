/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2011 Sérgio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kitemiconcheckcombo.h"

#include <KLocalizedString>

KItemIconCheckCombo::KItemIconCheckCombo(ViewType viewType, QWidget *parent)
    : KPIM::KCheckComboBox(parent)
{
    addItem(i18n("Calendar's custom icon"));
    addItem(QIcon::fromTheme(QStringLiteral("view-calendar-tasks")), i18n("To-do"));
    addItem(QIcon::fromTheme(QStringLiteral("view-pim-journal")), i18n("Journal"));
    addItem(QIcon::fromTheme(QStringLiteral("appointment-recurring")), i18n("Recurring"));
    addItem(QIcon::fromTheme(QStringLiteral("appointment-reminder")), i18n("Alarm"));
    addItem(QIcon::fromTheme(QStringLiteral("object-locked")), i18nc("the incidence is read-only", "Read Only"));
    addItem(QIcon::fromTheme(QStringLiteral("mail-reply-sender")), i18n("Needs Reply"));
    addItem(QIcon::fromTheme(QStringLiteral("meeting-attending")), i18n("Attending"));
    addItem(QIcon::fromTheme(QStringLiteral("meeting-attending-tentative")), i18n("Maybe Attending"));
    addItem(QIcon::fromTheme(QStringLiteral("meeting-organizer")), i18n("Organizer"));

    // Agenda view doesn't support journals yet
    setItemEnabled(EventViews::EventView::JournalIcon, viewType != ViewType::AgendaType);
    setItemEnabled(EventViews::EventView::ReplyIcon, viewType == ViewType::AgendaType);
    setItemEnabled(EventViews::EventView::AttendingIcon, viewType == ViewType::AgendaType);
    setItemEnabled(EventViews::EventView::TentativeIcon, viewType == ViewType::AgendaType);
    setItemEnabled(EventViews::EventView::OrganizerIcon, viewType == ViewType::AgendaType);

    setDefaultText(i18nc("@item:inlistbox", "Icons to use"));
    setAlwaysShowDefaultText(true);
}

KItemIconCheckCombo::~KItemIconCheckCombo() = default;

void KItemIconCheckCombo::setCheckedIcons(const QSet<EventViews::EventView::ItemIcon> &icons)
{
    const int itemCount = count();
    for (int i = 0; i < itemCount; ++i) {
        if (itemEnabled(i)) {
            setItemCheckState(i, icons.contains(static_cast<EventViews::EventView::ItemIcon>(i)) ? Qt::Checked : Qt::Unchecked);
        } else {
            setItemCheckState(i, Qt::Unchecked);
        }
    }
}

QSet<EventViews::EventView::ItemIcon> KItemIconCheckCombo::checkedIcons() const
{
    QSet<EventViews::EventView::ItemIcon> icons;
    const int itemCount = count();
    for (int i = 0; i < itemCount; ++i) {
        const QVariant value = itemCheckState(i);
        if (value.toBool()) {
            icons.insert(static_cast<EventViews::EventView::ItemIcon>(i));
        }
    }
    return icons;
}

#include "moc_kitemiconcheckcombo.cpp"
