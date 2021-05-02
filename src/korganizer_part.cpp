/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2000 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "korganizer_part.h"
#include "aboutdata.h"
#include "actionmanager.h"
#include "calendarview.h"
#include "impl/korganizerifaceimpl.h"
#include "kocore.h"
#include <CalendarSupport/Utils>

#include <KCalUtils/IncidenceFormatter>

#include "korganizer_debug.h"
#include <kcoreaddons_version.h>
#if KCOREADDONS_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include "korgmigrateapplication.h"
#endif
#include <KParts/StatusBarExtension>
#include <QStatusBar>

#include <QVBoxLayout>

K_PLUGIN_FACTORY(KOrganizerFactory, registerPlugin<KOrganizerPart>();)

KOrganizerPart::KOrganizerPart(QWidget *parentWidget, QObject *parent, const QVariantList &)
    : KParts::ReadOnlyPart(parent)
{
#if KCOREADDONS_VERSION < QT_VERSION_CHECK(6, 0, 0)
    KOrgMigrateApplication migrate;
    migrate.migrate();
#endif
    if (parentWidget) {
        mTopLevelWidget = parentWidget->topLevelWidget();
    } else if (parent && parent->isWidgetType()) {
        mTopLevelWidget = qobject_cast<QWidget *>(parent);
    } else {
        qCCritical(KORGANIZER_LOG) << "Cannot initialize the part without a top level widget.";
    }
    Q_ASSERT(mTopLevelWidget);

    KOCore::self()->addXMLGUIClient(mTopLevelWidget, this);

    // create a canvas to insert our widget
    auto canvas = new QWidget(parentWidget);
    canvas->setFocusPolicy(Qt::ClickFocus);
    setWidget(canvas);
    mView = new CalendarView(canvas);

    mActionManager = new ActionManager(this, mView, this, this, true);
    (void)new KOrganizerIfaceImpl(mActionManager, this, QStringLiteral("IfaceImpl"));

    mActionManager->createCalendarAkonadi();
    setHasDocument(false);

    mStatusBarExtension = new KParts::StatusBarExtension(this);
    setComponentName(QStringLiteral("korganizer"), i18n("KOrganizer"));

    auto topLayout = new QVBoxLayout(canvas);
    topLayout->addWidget(mView);
    topLayout->setContentsMargins({});

    connect(mView, &CalendarView::incidenceSelected, this, &KOrganizerPart::slotChangeInfo);

    mActionManager->init();
    mActionManager->readSettings();

    setXMLFile(QStringLiteral("korganizer_part.rc"), true);
    mActionManager->loadParts();
    setTitle();
}

KOrganizerPart::~KOrganizerPart()
{
    mActionManager->writeSettings();

    delete mActionManager;
    mActionManager = nullptr;

    KOCore::self()->removeXMLGUIClient(mTopLevelWidget);
}

void KOrganizerPart::slotChangeInfo(const Akonadi::Item &item, const QDate &date)
{
    Q_UNUSED(date)
    const KCalendarCore::Incidence::Ptr incidence = CalendarSupport::incidence(item);
    if (incidence) {
        Q_EMIT textChanged(incidence->summary() + QLatin1String(" / ")
                           + KCalUtils::IncidenceFormatter::timeToString(incidence->dtStart().toLocalTime().time()));
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
    mActionManager->importCalendar(QUrl::fromLocalFile(localFilePath()));
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
