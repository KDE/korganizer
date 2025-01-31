/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 1997, 1998, 1999 Preston Brown <preston.brown@yale.edu>
  SPDX-FileCopyrightText: Fester Zigterman <F.J.F.ZigtermanRustenburg@student.utwente.nl>
  SPDX-FileCopyrightText: Ian Dawes <iadawes@globalserve.net>
  SPDX-FileCopyrightText: Laszlo Boloni <boloni@cs.purdue.edu>

  SPDX-FileCopyrightText: 2000-2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "korganizer.h"
#include "actionmanager.h"
#include "calendarview.h"
#include "config-korganizer.h"
#include "impl/korganizerifaceimpl.h"
#include "kocore.h"
#include "plugininterface/korganizerplugininterface.h"

#include <Libkdepim/ProgressStatusBarWidget>
#include <Libkdepim/StatusbarProgressWidget>

// #include "korganizer_debug.h"
#include <KAboutData>
#include <KActionCollection>
#include <KSharedConfig>
#include <KShortcutsDialog>
#include <KStandardAction>
#include <KToolBar>
#include <PimCommon/NeedUpdateVersionUtils>
#include <PimCommon/NeedUpdateVersionWidget>
#include <QLabel>
#include <QStatusBar>
#include <QVBoxLayout>
#ifdef WITH_KUSERFEEDBACK
#include "userfeedback/userfeedbackmanager.h"

#include <KUserFeedback/NotificationPopup>
#include <KUserFeedback/Provider>
#endif

KOrganizer::KOrganizer()
    : KParts::MainWindow()
    , KOrg::MainWindow()
    , mCalendarView(new CalendarView(this))
{
    KOCore::self()->addXMLGUIClient(this, this);
    //  setMinimumSize(600,400);  // make sure we don't get resized too small...

    auto mainWidget = new QWidget(this);
    auto mainWidgetLayout = new QVBoxLayout(mainWidget);
    mainWidgetLayout->setContentsMargins({});
    mainWidgetLayout->setSpacing(0);
    if (PimCommon::NeedUpdateVersionUtils::checkVersion()) {
        const auto status = PimCommon::NeedUpdateVersionUtils::obsoleteVersionStatus(QLatin1String(KORGANIZER_RELEASE_VERSION_DATE), QDate::currentDate());
        if (status != PimCommon::NeedUpdateVersionUtils::ObsoleteVersion::NotObsoleteYet) {
            auto needUpdateVersionWidget = new PimCommon::NeedUpdateVersionWidget(this);
            mainWidgetLayout->addWidget(needUpdateVersionWidget);
            needUpdateVersionWidget->setObsoleteVersion(status);
        }
    }
    mainWidgetLayout->addWidget(mCalendarView);

    mCalendarView->setObjectName(QLatin1StringView("KOrganizer::CalendarView"));
    setCentralWidget(mainWidget);

    mActionManager = new ActionManager(this, mCalendarView, this, this, false, menuBar(), toolBar());
    (void)new KOrganizerIfaceImpl(mActionManager, this, QStringLiteral("IfaceImpl"));

#ifdef WITH_KUSERFEEDBACK
    auto userFeedBackNotificationPopup = new KUserFeedback::NotificationPopup(this);
    userFeedBackNotificationPopup->setFeedbackProvider(UserFeedBackManager::self()->userFeedbackProvider());
#endif
}

KOrganizer::~KOrganizer()
{
    delete mActionManager;

    KOCore::self()->removeXMLGUIClient(this);
}

void KOrganizer::init(bool document)
{
    setHasDocument(document);

    // Create calendar object, which manages all calendar information associated
    // with this calendar view window.
    mActionManager->createCalendarAkonadi();

    mActionManager->init();

    KOrganizerPluginInterface::self()->setActionCollection(actionCollection());
    KOrganizerPluginInterface::self()->initializePlugins();
    initActions();
    readSettings();

    QStatusBar *bar = statusBar();

    bar->addWidget(new QLabel(this));

    auto progressBar = new KPIM::ProgressStatusBarWidget(statusBar(), this);

    bar->addPermanentWidget(progressBar->littleProgress());

    connect(mActionManager->view(), &CalendarView::statusMessage, this, &KOrganizer::showStatusMessage);

    setStandardToolBarMenuEnabled(true);
    setTitle();
}

#if 0
void KOrganizer::initializePluginActions()
{
#if 0
    if (mXmlGuiClient->factory()) {
        QHashIterator<PimCommon::ActionType::Type, QList<QAction *> > localActionsType(
            mPluginInterface->actionsType());
        while (localActionsType.hasNext()) {
            localActionsType.next();
            QList<QAction *> lst = localActionsType.value();
            if (!lst.isEmpty()) {
                const QString actionlistname = QStringLiteral("korganizer")
                                               + PimCommon::PluginInterface::actionXmlExtension(
                    localActionsType.key());
                mXmlGuiClient->unplugActionList(actionlistname);
                mXmlGuiClient->plugActionList(actionlistname, lst);
            }
        }
    }
#else
    qCDebug(KORGANIZER_LOG) << " Plugins not implemented yet";
#endif
}

#endif

void KOrganizer::newMainWindow(const QUrl &url)
{
    auto korg = new KOrganizer();
    if (url.isValid() || url.isEmpty()) {
        korg->init(true);
        if (mActionManager->importURL(url, false) || url.isEmpty()) {
            korg->show();
        } else {
            delete korg;
        }
    } else {
        korg->init(false);
        korg->show();
    }
}

void KOrganizer::readSettings()
{
    // read settings from the KConfig, supplying reasonable
    // defaults where none are to be found

    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    mActionManager->readSettings();
    config->sync();
}

void KOrganizer::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    mActionManager->writeSettings();
    config->sync();
}

void KOrganizer::initActions()
{
    setStandardToolBarMenuEnabled(true);
    createStandardStatusBarAction();

    KStandardActions::keyBindings(this, &KOrganizer::slotEditKeys, actionCollection());
    KStandardActions::configureToolbars(this, &KOrganizer::configureToolbars, actionCollection());
    KStandardActions::quit(this, &KOrganizer::close, actionCollection());

    setXMLFile(QStringLiteral("korganizerui.rc"), true);
    createGUI(nullptr);

    setAutoSaveSettings();
}

void KOrganizer::slotEditKeys()
{
    KShortcutsDialog::showDialog(actionCollection(), KShortcutsEditor::LetterShortcutsAllowed);
}

bool KOrganizer::queryClose()
{
    bool close = mActionManager->queryClose();

    // Write configuration. I don't know if it really makes sense doing it this
    // way, when having opened multiple calendars in different CalendarViews.
    if (close) {
        writeSettings();
    }

    return close;
}

void KOrganizer::showStatusMessage(const QString &message)
{
    statusBar()->showMessage(message, 2000);
}

bool KOrganizer::openURL(const QUrl &url, bool merge)
{
    return mActionManager->importURL(url, merge);
}

bool KOrganizer::saveURL()
{
    return mActionManager->saveURL();
}

bool KOrganizer::saveAsURL(const QUrl &kurl)
{
    return mActionManager->saveAsURL(kurl);
}

QUrl KOrganizer::getCurrentURL() const
{
    return mActionManager->url();
}

KXMLGUIFactory *KOrganizer::mainGuiFactory()
{
    return factory();
}

KXMLGUIClient *KOrganizer::mainGuiClient()
{
    return this;
}

QWidget *KOrganizer::topLevelWidget()
{
    return this;
}

void KOrganizer::saveProperties(KConfigGroup &config)
{
    mActionManager->saveProperties(config);
}

void KOrganizer::readProperties(const KConfigGroup &config)
{
    mActionManager->readProperties(config);
}

KOrg::CalendarViewBase *KOrganizer::view() const
{
    return mActionManager->view();
}

ActionManager *KOrganizer::actionManager()
{
    return mActionManager;
}

KActionCollection *KOrganizer::getActionCollection() const
{
    return actionCollection();
}

void KOrganizer::setTitle()
{
    QString title;
    if (hasDocument()) {
        const QUrl url = mActionManager->url();

        if (!url.isEmpty()) {
            if (url.isLocalFile()) {
                title = url.fileName();
            } else {
                title = url.toDisplayString();
            }
        } else {
            title = i18n("New Calendar");
        }

        if (mCalendarView->isReadOnly()) {
            title += QLatin1StringView(" [") + i18nc("the calendar is read-only", "read-only") + QLatin1Char(']');
        }
    } else {
        title = i18n("Calendar");
    }
    if (mCalendarView->isFiltered()) {
        title += QLatin1StringView(" - <") + mCalendarView->currentFilterName() + QLatin1StringView("> ");
    }

    setCaption(title, false);
}

#include "moc_korganizer.cpp"
