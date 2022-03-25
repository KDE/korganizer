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
    Q_REQUIRED_RESULT bool openURL(const QString &url);
    Q_REQUIRED_RESULT bool mergeURL(const QString &url);
    Q_REQUIRED_RESULT bool saveURL();
    Q_REQUIRED_RESULT bool saveAsURL(const QString &url);
    Q_REQUIRED_RESULT QString getCurrentURLasString() const;

    Q_REQUIRED_RESULT bool editIncidence(const QString &akonadiUrl);
    /** @reimp from KOrganizerIface::deleteIncidence()
        @param akonadiUrl the akonadi Item URL of the item to delete. if no such item exists,
        nothing happens
        @return true if the item could be deleted, false otherwise
    */
    Q_REQUIRED_RESULT bool deleteIncidence(const QString &akonadiUrl)
    {
        return deleteIncidence(akonadiUrl, false);
    }

    /**
      Delete the incidence with the given akonadi item URL from the active calendar.
      @param akonadiUrl The Akonadi Item URL.
      @param force If true, all recurrences and sub-todos (if applicable) will
      be deleted without prompting for confirmation.
    */
    Q_REQUIRED_RESULT bool deleteIncidence(const QString &akonadiUrl, bool force);

    /**
      Add an incidence to the active calendar.
      @param iCal A calendar in iCalendar format containing the incidence. The
                  calendar must consist of a VCALENDAR component which contains
                  the incidence (VEVENT, VTODO, VJOURNAL or VFREEBUSY) and
                  optionally a VTIMEZONE component. If there is more than one
                  incidence, only the first is added to KOrganizer's calendar.
    */
    Q_REQUIRED_RESULT bool addIncidence(const QString &iCal);

    /**
      Show a HTML representation of the incidence (the "View.." dialog).
      If no incidence with the given Akonadi Item URL exists, nothing happens.
      @param akonadiUrl The Akonadi Item URL of the incidence to be shown.
    */
    Q_REQUIRED_RESULT bool showIncidence(const QString &akonadiUrl);

    /**
      Show an incidence in context. This means showing the todo, agenda or
      journal view (as appropriate) and scrolling it to show the incidence.
      @param akonadiUrl the Akonadi Item URL of the incidence to show.
    */
    Q_REQUIRED_RESULT bool showIncidenceContext(const QString &akonadiUrl);

    /**
     * Called by KOrganizerUniqueAppHandler in the kontact plugin
     * Returns true if the command line was successfully handled
     * false otherwise.
     */
    Q_REQUIRED_RESULT bool handleCommandLine(const QStringList &args);

private:
    ActionManager *const mActionManager;
};
