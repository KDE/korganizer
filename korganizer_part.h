/*
    This file is part of KOrganizer.

    Copyright (c) 2000,2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef KORGANIZER_PART_H
#define KORGANIZER_PART_H

#include <kurl.h>
#include <kparts/part.h>

#include <korganizer/mainwindow.h>


class KInstance;
class KAboutData;
class KProcess;

class CalendarView;
class ActionManager;

namespace KCal {
  class CalendarResources;
  class Calendar;
  class Incidence;
}
using namespace KCal;
namespace KParts {
class StatusBarExtension;
}
namespace KOrg {
class CalendarViewBase;
}

class KOrganizerPart: public KParts::ReadOnlyPart,
                      public KOrg::MainWindow
{
    Q_OBJECT
  public:
    KOrganizerPart( QWidget *parentWidget, const char *widgetName,
                    QObject *parent, const char *name, const QStringList & );
    virtual ~KOrganizerPart();

    static KAboutData *createAboutData();

    virtual KOrg::CalendarViewBase *view() const;

    /** Load calendar file from URL. Merge into current calendar, if \a merge is true. */
    virtual bool openURL( const KURL &url, bool merge = false );
    /** Save calendar file to URL of current calendar */
    virtual bool saveURL();
    /** Save calendar file to URL */
    virtual bool saveAsURL( const KURL &kurl );

    /** Get current URL */
    virtual KURL getCurrentURL() const;

    virtual KXMLGUIFactory *mainGuiFactory() { return factory(); }
    virtual KXMLGUIClient *mainGuiClient() { return this; }
    virtual QWidget *topLevelWidget();
    virtual ActionManager *actionManager();
    virtual KActionCollection *getActionCollection() const { return actionCollection(); }
    virtual void showStatusMessage( const QString &message );

    void setTitle();

  public slots:
    void slotChangeInfo( Incidence * );

  protected:
    virtual bool openFile();

  protected slots:
    void startCompleted( KProcess * );

  private:
    CalendarView *mView;
    ActionManager *mActionManager;
    KParts::StatusBarExtension *mStatusBarExtension;
    QWidget *mTopLevelWidget;

  signals:
    void textChanged( const QString & );
};

#endif
