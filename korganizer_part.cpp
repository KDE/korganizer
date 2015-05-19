/*
  This file is part of KOrganizer.

  Copyright (c) 2000 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "korganizer_part.h"
#include "aboutdata.h"
#include "actionmanager.h"
#include "calendarview.h"
#include "kocore.h"
#include "impl/korganizerifaceimpl.h"
#include "korgstartup.h"
#include <calendarsupport/utils.h>

#include <KCalUtils/IncidenceFormatter>

#include <QStatusBar>
#include <KParts/StatusBarExtension>
#include "korganizer_debug.h"

#include <QVBoxLayout>


K_PLUGIN_FACTORY(KOrganizerFactory, registerPlugin<KOrganizerPart>();)

KOrganizerPart::KOrganizerPart(QWidget *parentWidget, QObject *parent, const QVariantList &)
    : KParts::ReadOnlyPart(parent)
{
    KOrgStartup::migrateConfig();
    if (parentWidget) {
        mTopLevelWidget = parentWidget->topLevelWidget();
    } else if (parent && parent->isWidgetType()) {
        mTopLevelWidget = (QWidget *)parent;
    } else {
        qCCritical(KORGANIZER_LOG) << "Cannot initialize the part without a top level widget.";
    }

    KOCore::self()->addXMLGUIClient(mTopLevelWidget, this);

    // create a canvas to insert our widget
    QWidget *canvas = new QWidget(parentWidget);
    canvas->setFocusPolicy(Qt::ClickFocus);
    setWidget(canvas);
    mView = new CalendarView(canvas);

    mActionManager = new ActionManager(this, mView, this, this, true);
    (void)new KOrganizerIfaceImpl(mActionManager, this, "IfaceImpl");

    mActionManager->createCalendarAkonadi();
    setHasDocument(false);

    mStatusBarExtension = new KParts::StatusBarExtension(this);
    setComponentName(QStringLiteral("korganizer"), QStringLiteral("korganizer"));

    QVBoxLayout *topLayout = new QVBoxLayout(canvas);
    topLayout->addWidget(mView);
    topLayout->setMargin(0);

    connect(mView, &CalendarView::incidenceSelected, this, &KOrganizerPart::slotChangeInfo);

    mActionManager->init();
    mActionManager->readSettings();

    setXMLFile(QLatin1String("korganizer_part.rc"), true);
    mActionManager->loadParts();
    setTitle();
}

KOrganizerPart::~KOrganizerPart()
{
    mActionManager->writeSettings();

    delete mActionManager;
    mActionManager = Q_NULLPTR;

    KOCore::self()->removeXMLGUIClient(mTopLevelWidget);
}

void KOrganizerPart::slotChangeInfo(const Akonadi::Item &item, const QDate &date)
{
    Q_UNUSED(date);
    const KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence(item);
    if (incidence) {
        Q_EMIT textChanged(incidence->summary() + QLatin1String(" / ") +
                           KCalUtils::IncidenceFormatter::timeToString(incidence->dtStart()));
    } else {
        Q_EMIT textChanged(QString());
    }
}

QWidget *KOrganizerPart::topLevelWidget()
{
    return mView->topLevelWidget();
}

ActionManager *KOrganizerPart::actionManager()
{
    return mActionManager;
}

void KOrganizerPart::showStatusMessage(const QString &message)
{
    QStatusBar *statusBar = mStatusBarExtension->statusBar();
    if (statusBar) {
        statusBar->showMessage(message);
    }
}

KOrg::CalendarViewBase *KOrganizerPart::view() const
{
    return mView;
}

bool KOrganizerPart::openURL(const QUrl &url, bool merge)
{
    return mActionManager->importURL(url, merge);
}

bool KOrganizerPart::saveURL()
{
    return mActionManager->saveURL();
}

bool KOrganizerPart::saveAsURL(const QUrl &url)
{
    return mActionManager->saveAsURL(url);
}

QUrl KOrganizerPart::getCurrentURL() const
{
    return mActionManager->url();
}

bool KOrganizerPart::openFile()
{
    mActionManager->importCalendar(localFilePath());
    return true;
}

// FIXME: This is copied verbatim from the KOrganizer class. Move it to the common base class!
void KOrganizerPart::setTitle()
{
//  qCDebug(KORGANIZER_LOG) <<"KOrganizer::setTitle";
// FIXME: Inside kontact we want to have different titles depending on the
//        type of view (calendar, to-do, journal). How can I add the filter
//        name in that case?
    /*
      QString title;
      if ( !hasDocument() ) {
        title = i18n("Calendar");
      } else {
        QUrl url = mActionManager->url();

        if ( !url.isEmpty() ) {
          if ( url.isLocalFile() ) title = url.fileName();
          else title = url.prettyUrl();
        } else {
          title = i18n("New Calendar");
        }

        if ( mView->isReadOnly() ) {
          title += " [" + i18n("read-only") + ']';
        }
      }

      title += " - <" + mView->currentFilterName() + "> ";

      Q_EMIT setWindowCaption( title );*/
}

#include "korganizer_part.moc"
