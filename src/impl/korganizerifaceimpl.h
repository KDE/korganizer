/*
  This file is part of KOrganizer

  SPDX-FileCopyrightText: 2004 Bo Thorsen <bo@sonofthor.dk>
  SPDX-FileCopyrightText: 2005 Rafal Rzepecki <divide@users.sourceforge.net>

  SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "korganizerprivate_export.h"

#include <QObject>

class ActionManager;

class KORGANIZERPRIVATE_EXPORT KOrganizerIfaceImpl : public QObject
{
    Q_OBJECT
public:
    explicit KOrganizerIfaceImpl(ActionManager *mActionManager, QObject *parent = nullptr, const QString &name = QString());
    ~KOrganizerIfaceImpl() override;

public Q_SLOTS:
    [[nodiscard]] bool openURL(const QString &url);
    [[nodiscard]] bool mergeURL(const QString &url);
    [[nodiscard]] bool saveURL();
    [[nodiscard]] bool saveAsURL(const QString &url);
    [[nodiscard]] QString getCurrentURLasString();

    [[nodiscard]] bool editIncidence(const QString &uid);
    /** @reimp from KOrganizerIface::deleteIncidence()
        @param uid the Akonadi Id of the item to delete. if no such item exists,
        nothing happens.
        @return true if the item could be deleted, false otherwise
    */
    [[nodiscard]] bool deleteIncidence(const QString &uid)
    {
        return deleteIncidence(uid, false);
    }

    /**
      Delete the incidence with the given Akonadi Id from the active calendar.
      @param uid the Akonadi Id of the item to delete.if no such item exists,
      nothing happens.
      @param force If true, all recurrences and sub-todos (if applicable) will
      be deleted without prompting for confirmation.
    */
    [[nodiscard]] bool deleteIncidence(const QString &uid, bool force);

    /**
      Add an incidence to the active calendar.
      @param iCal A calendar in iCalendar format containing the incidence. The
                  calendar must consist of a VCALENDAR component which contains
                  the incidence (VEVENT, VTODO, VJOURNAL or VFREEBUSY) and
                  optionally a VTIMEZONE component. If there is more than one
                  incidence, only the first is added to KOrganizer's calendar.
    */
    [[nodiscard]] bool addIncidence(const QString &iCal);

    /**
      Show a HTML representation of the incidence (the "View.." dialog).
      If no incidence with the given Akonadi Item URL exists, nothing happens.
      @param uid he Akonadi Id of the item to show.
    */
    [[nodiscard]] bool showIncidence(const QString &uid);

    /**
      Show an incidence in context. This means showing the todo, agenda or
      journal view (as appropriate) and scrolling it to show the incidence.
      @param uid he Akonadi Id of the item to scroll to and show.
    */
    [[nodiscard]] bool showIncidenceContext(const QString &uid);

    /**
     * Called by KOrganizerUniqueAppHandler in the kontact plugin
     * Returns true if the command line was successfully handled
     * false otherwise.
     */
    [[nodiscard]] bool handleCommandLine(const QStringList &args);

private:
    ActionManager *const mActionManager;
};
