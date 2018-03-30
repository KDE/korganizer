/*
  This file is part of the KDE reminder agent.

  Copyright (c) 2000,2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (c) 2008-2009 Allen Winter <winter@kde.org>
  Copyright (c) 2009-2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

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

#include <config-korganizer.h>
#include "alarmdialog.h"
#include "korganizer_interface.h"
#include "mailclient.h"
#include "koalarmclient_debug.h"

#include <CalendarSupport/IncidenceViewer>
#include <CalendarSupport/KCalPrefs>
#include <CalendarSupport/IdentityManager>
#include <CalendarSupport/Utils>

#include <IncidenceEditor/IncidenceDialog>
#include <IncidenceEditor/IncidenceDialogFactory>

#include <KCalCore/Event>
#include <KCalCore/Todo>
#include <KCalUtils/IncidenceFormatter>

#include <KIdentityManagement/Identity>

#include <AkonadiCore/Item>

#include <MailTransport/TransportManager>
#include <QUrl>

#include <KComboBox>
#include <QHBoxLayout>
#include <KLocalizedString>
#include <KMessageBox>
#include <KNotification>
#include <KSharedConfig>
#include <KToolInvocation>
#include <KWindowSystem>
#include <KIconLoader>
#include <QIcon>
#include <phonon/mediaobject.h>
#include <QLabel>
#include <QKeyEvent>
#include <QSpinBox>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>

using namespace KIdentityManagement;
using namespace KCalCore;
using namespace KCalUtils;

// fallback defaults
static int defSuspendVal = 5;
static int defSuspendUnit = AlarmDialog::SuspendInMinutes;

class ReminderTreeItem : public QTreeWidgetItem
{
public:
    ReminderTreeItem(const Akonadi::Item &incidence, QTreeWidget *parent)
        : QTreeWidgetItem(parent)
        , mIncidence(incidence)
        , mNotified(false)
    {
    }

    bool operator<(const QTreeWidgetItem &other) const override;

    QString mDisplayText;

    const Akonadi::Item mIncidence;
    QDateTime mRemindAt;
    QDateTime mTrigger;
    QDateTime mHappening;
    bool mNotified;
};

struct ConfItem {
    QString uid;
    QUrl akonadiUrl;
    QDateTime remindAt;
};

bool ReminderTreeItem::operator<(const QTreeWidgetItem &other) const
{
    switch (treeWidget()->sortColumn()) {
    case 1:
    {         // happening datetime
        const ReminderTreeItem *item = static_cast<const ReminderTreeItem *>(&other);
        return item->mHappening < mHappening;
    }
    case 2:
    {         // trigger datetime
        const ReminderTreeItem *item = static_cast<const ReminderTreeItem *>(&other);
        return item->mTrigger < mTrigger;
    }
    default:
        return QTreeWidgetItem::operator <(other);
    }
}

AlarmDialog::AlarmDialog(const Akonadi::ETMCalendar::Ptr &calendar, QWidget *parent)
    : QDialog(parent, Qt::WindowStaysOnTopHint)
    , mCalendar(calendar)
    , mSuspendTimer(this)
{
    // User1 => Edit...
    // User2 => Dismiss All
    // User3 => Dismiss Selected
    //    Ok => Suspend

    if (calendar) {
        connect(
            calendar.data(), &Akonadi::ETMCalendar::calendarChanged, this,
            &AlarmDialog::slotCalendarChanged);
    }

    KIconLoader::global()->addAppDir(QStringLiteral("korgac"));

    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup generalConfig(config, "General");
    QPoint pos = generalConfig.readEntry("Position", QPoint(0, 0));

    int defSuspendVal = generalConfig.readEntry("DefaultSuspendValue", defSuspendVal);
    int suspendVal = generalConfig.readEntry("SuspendValue", defSuspendVal);

    int defSuspendUnit = generalConfig.readEntry("SuspendUnit", defSuspendUnit);
    SuspendUnit suspendUnit = static_cast<SuspendUnit>(generalConfig.readEntry("SuspendUnit",
                                                                               defSuspendUnit));

    QWidget *topBox = new QWidget(this);
    if (!pos.isNull()) {
        mPos = pos;
        topBox->move(mPos);
    }
    setWindowTitle(i18nc("@title:window", "Reminders"));
    setWindowIcon(QIcon::fromTheme(QStringLiteral("korgac")));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, this);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(topBox);
    mOkButton = buttonBox->button(QDialogButtonBox::Ok);
    mOkButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    mUser1Button = new QPushButton;
    buttonBox->addButton(mUser1Button, QDialogButtonBox::ActionRole);
    mUser2Button = new QPushButton;
    buttonBox->addButton(mUser2Button, QDialogButtonBox::ActionRole);
    mUser3Button = new QPushButton;
    buttonBox->addButton(mUser3Button, QDialogButtonBox::ActionRole);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &AlarmDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &AlarmDialog::reject);
    mainLayout->addWidget(buttonBox);

    mUser3Button->setText(i18nc("@action:button", "Dismiss Reminder"));
    mUser3Button->setToolTip(i18nc("@info:tooltip",
                                   "Dismiss the reminders for the selected incidences"));
    mUser2Button->setText(i18nc("@action:button", "Dismiss All"));
    mUser2Button->setToolTip(i18nc("@info:tooltip",
                                   "Dismiss the reminders for all listed incidences"));
    mUser1Button->setText(i18nc("@action:button", "Edit..."));
    mUser1Button->setToolTip(i18nc("@info:tooltip",
                                   "Edit the selected incidence"));
    mOkButton->setText(i18nc("@action:button", "Suspend"));
    mOkButton->setToolTip(i18nc("@info:tooltip",
                                "Suspend the reminders for the selected incidences "
                                "by the specified interval"));

    // Try to keep the dialog small and non-obtrusive.
    setMinimumWidth(575);
    setMinimumHeight(300);

    QVBoxLayout *mTopLayout = new QVBoxLayout(topBox);
    mTopLayout->setMargin(0);

    QLabel *label = new QLabel(
        i18nc("@label",
              "Reminders: "
              "Click on a title to toggle the details viewer for that item"),
        topBox);
    mTopLayout->addWidget(label);

    mIncidenceTree = new QTreeWidget(topBox);
    mIncidenceTree->setColumnCount(3);
    mIncidenceTree->setSortingEnabled(true);
    const QStringList headerLabels
        = (QStringList(i18nc("@title:column reminder title", "Title"))
           << i18nc("@title:column happens at date/time", "Date Time")
           << i18nc("@title:column trigger date/time", "Trigger Time"));
    mIncidenceTree->setHeaderLabels(headerLabels);
    mIncidenceTree->headerItem()->setToolTip(
        0,
        i18nc("@info:tooltip", "The event or to-do title"));
    mIncidenceTree->headerItem()->setToolTip(
        1,
        i18nc("@info:tooltip", "The reminder is set for this date/time"));
    mIncidenceTree->headerItem()->setToolTip(
        2,
        i18nc("@info:tooltip", "The date/time the reminder was triggered"));

    mIncidenceTree->setWordWrap(true);
    mIncidenceTree->setAllColumnsShowFocus(true);
    mIncidenceTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    mIncidenceTree->setRootIsDecorated(false);

    mTopLayout->addWidget(mIncidenceTree);

    connect(mIncidenceTree, &QTreeWidget::itemClicked, this, &AlarmDialog::update);
    connect(mIncidenceTree, &QTreeWidget::itemDoubleClicked, this, &AlarmDialog::edit);
    connect(mIncidenceTree, &QTreeWidget::itemSelectionChanged, this, &AlarmDialog::updateButtons);

    mDetailView = new CalendarSupport::IncidenceViewer(mCalendar.data(), topBox);
    const QString s = xi18nc("@info default incidence details string",
                             "<emphasis>Select an event or to-do from the list above "
                             "to view its details here.</emphasis>");
    mDetailView->setDefaultMessage(s);
    mTopLayout->addWidget(mDetailView);
    mDetailView->hide();
    mLastItem = 0;

    QWidget *suspendBox = new QWidget(topBox);
    QHBoxLayout *suspendBoxHBoxLayout = new QHBoxLayout(suspendBox);
    suspendBoxHBoxLayout->setMargin(0);
    mTopLayout->addWidget(suspendBox);

    QLabel *l = new QLabel(i18nc("@label:spinbox", "Suspend &duration:"), suspendBox);
    suspendBoxHBoxLayout->addWidget(l);

    mSuspendSpin = new QSpinBox(suspendBox);
    suspendBoxHBoxLayout->addWidget(mSuspendSpin);
    mSuspendSpin->setRange(1, 9999);
    mSuspendSpin->setValue(suspendVal);    // default suspend duration
    mSuspendSpin->setToolTip(
        i18nc("@info:tooltip",
              "Suspend the reminders by this amount of time"));
    mSuspendSpin->setWhatsThis(
        i18nc("@info:whatsthis",
              "Each reminder for the selected incidences will be suspended "
              "by this number of time units. You can choose the time units "
              "(typically minutes) in the adjacent selector."));

    l->setBuddy(mSuspendSpin);

    mSuspendUnit = new KComboBox(suspendBox);
    suspendBoxHBoxLayout->addWidget(mSuspendUnit);
    mSuspendUnit->addItem(i18nc("@item:inlistbox suspend in terms of minutes", "minute(s)"));
    mSuspendUnit->addItem(i18nc("@item:inlistbox suspend in terms of hours", "hour(s)"));
    mSuspendUnit->addItem(i18nc("@item:inlistbox suspend in terms of days", "day(s)"));
    mSuspendUnit->addItem(i18nc("@item:inlistbox suspend in terms of weeks", "week(s)"));
    mSuspendUnit->setToolTip(
        i18nc("@info:tooltip",
              "Suspend the reminders using this time unit"));
    mSuspendUnit->setWhatsThis(
        i18nc("@info:whatsthis",
              "Each reminder for the selected incidences will be suspended "
              "using this time unit. You can set the number of time units "
              "in the adjacent number entry input."));
    mSuspendUnit->setCurrentIndex(static_cast<int>(suspendUnit));

    mSuspendMenu = new QMenu();
    mSuspendMenu->setToolTipsVisible(true);
    mOkButton->setMenu(mSuspendMenu);

    QAction *mResetSuspendAction = new QAction(i18nc("@action:inmenu", "Reset"));
    connect(mResetSuspendAction, &QAction::triggered, this, &AlarmDialog::resetSuspend);
    mResetSuspendAction->setToolTip(i18nc("@info:tooltip",
                                          "Reset the suspend time to the default value"));
    mResetSuspendAction->setWhatsThis(i18nc("@info:whatsthis",
                                            "Reset the suspend time to the default value"));
    mSuspendMenu->addAction(mResetSuspendAction);

    QAction *mSetDefaultSuspendAction = new QAction(i18nc("@action:inmenu", "Set as Default"));
    connect(mSetDefaultSuspendAction, &QAction::triggered, this, &AlarmDialog::setDefaultSuspend);
    mSetDefaultSuspendAction->setToolTip(i18nc("@info:tooltip",
                                               "Set the current suspend time as the new default"));
    mSetDefaultSuspendAction->setWhatsThis(i18nc("@info:whatsthis",
                                                 "Press this button to set the current suspend "
                                                 "time as the new default value"));
    mSuspendMenu->addAction(mSetDefaultSuspendAction);

    connect(&mSuspendTimer, &QTimer::timeout, this, &AlarmDialog::wakeUp);

    connect(mOkButton, &QPushButton::clicked, this, &AlarmDialog::slotOk);
    connect(mUser1Button, &QPushButton::clicked, this, &AlarmDialog::slotUser1);
    connect(mUser2Button, &QPushButton::clicked, this, &AlarmDialog::slotUser2);
    connect(mUser3Button, &QPushButton::clicked, this, &AlarmDialog::slotUser3);

    mIdentityManager = new CalendarSupport::IdentityManager;
}

AlarmDialog::~AlarmDialog()
{
    mIncidenceTree->clear();
    delete mIdentityManager;
}

ReminderTreeItem *AlarmDialog::searchByItem(const Akonadi::Item &incidence)
{
    ReminderTreeItem *found = nullptr;
    QTreeWidgetItemIterator it(mIncidenceTree);
    while (*it) {
        ReminderTreeItem *item = static_cast<ReminderTreeItem *>(*it);
        if (item->mIncidence == incidence) {
            found = item;
            break;
        }
        ++it;
    }
    return found;
}

static QString cleanSummary(const QString &summary)
{
    static QString etc = i18nc("@label an elipsis", "...");
    int maxLen = 30;
    QString retStr = summary;
    retStr.replace(QLatin1Char('\n'), QLatin1Char(' '));
    if (retStr.length() > maxLen) {
        maxLen -= etc.length();
        retStr = retStr.left(maxLen);
        retStr += etc;
    }
    return retStr;
}

void AlarmDialog::addIncidence(const Akonadi::Item &incidenceitem, const QDateTime &reminderAt,
                               const QString &displayText)
{
    Incidence::Ptr incidence = CalendarSupport::incidence(incidenceitem);
    ReminderTreeItem *item = searchByItem(incidenceitem);
    if (!item) {
        item = new ReminderTreeItem(incidenceitem, mIncidenceTree);
    }
    item->mNotified = false;
    item->mHappening = QDateTime();
    item->mRemindAt = reminderAt;
    item->mTrigger = QDateTime::currentDateTime();
    item->mDisplayText = displayText;
    item->setText(0, cleanSummary(incidence->summary()));

    QString displayStr;
    const auto dateTime = triggerDateForIncidence(incidence, reminderAt, displayStr);

    if (incidence->type() == Incidence::TypeEvent) {
        item->setIcon(0, QIcon::fromTheme(QStringLiteral("view-calendar-day")));
    } else if (incidence->type() == Incidence::TypeTodo) {
        item->setIcon(0, QIcon::fromTheme(QStringLiteral("view-calendar-tasks")));
    }

    item->mHappening = dateTime;
    item->setText(1, displayStr);

    item->setText(2, QLocale().toString(item->mTrigger, QLocale::ShortFormat));
    QString tip
        = IncidenceFormatter::toolTipStr(
        CalendarSupport::displayName(mCalendar.data(), incidenceitem.parentCollection()),
        incidence, item->mRemindAt.date(), true);
    if (!item->mDisplayText.isEmpty()) {
        tip += QLatin1String("<br>") + item->mDisplayText;
    }
    item->setToolTip(0, tip);
    item->setToolTip(1, tip);
    item->setToolTip(2, tip);
    item->setData(0, QTreeWidgetItem::UserType, false);

    mIncidenceTree->setCurrentItem(item);
    showDetails(item);
    slotSave();
}

void AlarmDialog::resetSuspend()
{
    mSuspendSpin->setValue(defSuspendVal);    // default suspend duration
    mSuspendUnit->setCurrentIndex(defSuspendUnit);
}

void AlarmDialog::setDefaultSuspend()
{
    defSuspendVal = mSuspendSpin->value();
    defSuspendUnit = mSuspendUnit->currentIndex();
}

void AlarmDialog::slotOk()
{
    suspend();
}

void AlarmDialog::slotUser1()
{
    const ReminderList selection = selectedItems();
    if (!selection.isEmpty()) {
        ReminderTreeItem *item = selection.first();
        if (mCalendar->hasRight(item->mIncidence, Akonadi::Collection::CanChangeItem)) {
            edit();
        }
    }
}

void AlarmDialog::slotUser2()
{
    dismissAll();
}

void AlarmDialog::slotUser3()
{
    dismissCurrent();
}

void AlarmDialog::dismissCurrent()
{
    dismiss(selectedItems());

    const int activeCountNumber = activeCount();
    if (activeCountNumber == 0) {
        accept();
    } else {
        update();
    }
    Q_EMIT reminderCount(activeCountNumber);
}

void AlarmDialog::dismissAll()
{
    ReminderList selections;

    QTreeWidgetItemIterator it(mIncidenceTree);
    while (*it) {
        if (!(*it)->isDisabled()) {   //do not disable suspended reminders
            selections.append(static_cast<ReminderTreeItem *>(*it));
        }
        ++it;
    }
    dismiss(selections);

    setTimer();
    accept();
    Q_EMIT reminderCount(activeCount());
}

void AlarmDialog::dismiss(const ReminderList &selections)
{
    QList<Akonadi::Item::Id> ids;
    ids.reserve(selections.count());
    for (ReminderList::const_iterator it = selections.constBegin(); it != selections.constEnd();
         ++it) {
        qCDebug(KOALARMCLIENT_LOG) << "removing "
                                   << CalendarSupport::incidence((*it)->mIncidence)->summary();
        if (mIncidenceTree->itemBelow(*it)) {
            mIncidenceTree->setCurrentItem(mIncidenceTree->itemBelow(*it));
        } else if (mIncidenceTree->itemAbove(*it)) {
            mIncidenceTree->setCurrentItem(mIncidenceTree->itemAbove(*it));
        }
        mIncidenceTree->removeItemWidget(*it, 0);
        ids.append((*it)->mIncidence.id());
        delete *it;
    }

    removeFromConfig(ids);
}

void AlarmDialog::edit()
{
    ReminderList selection = selectedItems();
    if (selection.count() == 1) {
        Incidence::Ptr incidence = CalendarSupport::incidence(selection.first()->mIncidence);
        if (!mCalendar->hasRight(selection.first()->mIncidence,
                                 Akonadi::Collection::CanChangeItem)) {
            KMessageBox::sorry(
                this,
                i18nc("@info",
                      "\"%1\" is a read-only item so modifications are not possible.",
                      cleanSummary(incidence->summary())));
            return;
        }

        openIncidenceEditorNG(selection.first()->mIncidence);
    }
}

void AlarmDialog::suspend()
{
    if (!isVisible()) {   //do nothing if the dialog is hidden
        return;
    }

    int unit = 1;
    switch (mSuspendUnit->currentIndex()) {
    case SuspendInWeeks:
        unit *= 7;
        Q_FALLTHROUGH();
    case SuspendInDays:
        unit *= 24;
        Q_FALLTHROUGH();
    case SuspendInHours:
        unit *= 60;
        Q_FALLTHROUGH();
    case SuspendInMinutes:
        unit *= 60;
        Q_FALLTHROUGH();
    default:
        break;
    }

    ReminderTreeItem *selitem = nullptr;
    QTreeWidgetItemIterator it(mIncidenceTree);
    while (*it) {
        if ((*it)->isSelected() && !(*it)->isDisabled()) {   //suspend selected, non-suspended reminders
            (*it)->setSelected(false);
            (*it)->setDisabled(true);
            ReminderTreeItem *item = static_cast<ReminderTreeItem *>(*it);
            item->mRemindAt = QDateTime::currentDateTime().addSecs(unit * mSuspendSpin->value());
            item->mHappening = item->mRemindAt;
            item->mNotified = false;
            (*it)->setText(1, QLocale().toString(item->mHappening, QLocale::ShortFormat));
            selitem = item;
        }
        ++it;
    }

    if (selitem) {
        if (mIncidenceTree->itemBelow(selitem)) {
            mIncidenceTree->setCurrentItem(mIncidenceTree->itemBelow(selitem));
        } else if (mIncidenceTree->itemAbove(selitem)) {
            mIncidenceTree->setCurrentItem(mIncidenceTree->itemAbove(selitem));
        }
    }

    // save suspended alarms too so they can be restored on restart
    // kolab/issue4108
    slotSave();

    setTimer();
    if (activeCount() == 0) {
        accept();
    } else {
        update();
    }
    Q_EMIT reminderCount(activeCount());
}

void AlarmDialog::setTimer()
{
    int nextReminderAt = -1;

    QTreeWidgetItemIterator it(mIncidenceTree);
    while (*it) {
        ReminderTreeItem *item = static_cast<ReminderTreeItem *>(*it);
        if (item->mRemindAt > QDateTime::currentDateTime()) {
            const int secs = QDateTime::currentDateTime().secsTo(item->mRemindAt);
            nextReminderAt = nextReminderAt <= 0 ? secs : qMin(nextReminderAt, secs);
        }
        ++it;
    }

    if (nextReminderAt >= 0) {
        mSuspendTimer.stop();
        mSuspendTimer.start(1000 * (nextReminderAt + 1));
        mSuspendTimer.setSingleShot(true);
    }
}

void AlarmDialog::show()
{
    mIncidenceTree->resizeColumnToContents(0);
    mIncidenceTree->resizeColumnToContents(1);
    mIncidenceTree->resizeColumnToContents(2);
    mIncidenceTree->sortItems(1, Qt::AscendingOrder);

    // select the first item that hasn't already been notified
    QTreeWidgetItemIterator it(mIncidenceTree);
    while (*it) {
        ReminderTreeItem *item = static_cast<ReminderTreeItem *>(*it);
        if (!item->mNotified) {
            (*it)->setSelected(true);
            break;
        }
        ++it;
    }

    // reset the default suspend time
// Allen: commented-out the following lines on 17 Sept 2013
//  mSuspendSpin->setValue( defSuspendVal );
//  mSuspendUnit->setCurrentIndex( defSuspendUnit );

    QDialog::show();
    if (!mPos.isNull()) {
        QDialog::move(mPos);
    }
    KWindowSystem::unminimizeWindow(winId(), false);
    KWindowSystem::setState(winId(), NET::KeepAbove | NET::DemandsAttention);
    KWindowSystem::setOnAllDesktops(winId(), true);
    KWindowSystem::activateWindow(winId());

    // Audio, Procedure, and EMail alarms
    eventNotification();
}

void AlarmDialog::suspendAll()
{
    mIncidenceTree->clearSelection();
    QTreeWidgetItemIterator it(mIncidenceTree);

    // first, select all non-suspended reminders
    while (*it) {
        if (!(*it)->isDisabled()) {   //do not suspend suspended reminders
            (*it)->setSelected(true);
        }
        ++it;
    }

    //suspend all selected reminders
    suspend();
}

void AlarmDialog::eventNotification()
{
    bool beeped = false;
    bool found = false;

    QTreeWidgetItemIterator it(mIncidenceTree);
    while (*it) {
        ReminderTreeItem *item = static_cast<ReminderTreeItem *>(*it);
        ++it;
        if (item->isDisabled() || item->mNotified) {
            //skip suspended reminders or reminders that have been notified
            continue;
        }
        found = true;
        item->mNotified = true;
        Incidence::Ptr incidence = CalendarSupport::incidence(item->mIncidence);
        Alarm::List alarms = incidence->alarms();
        Alarm::List::ConstIterator ait;
        for (ait = alarms.constBegin(); ait != alarms.constEnd(); ++ait) {
            Alarm::Ptr alarm = *ait;
            // FIXME: Check whether this should be done for all multiple alarms
            if (alarm->type() == Alarm::Procedure) {
                // FIXME: Add a message box asking whether the procedure should really be executed
                qCDebug(KOALARMCLIENT_LOG) << "Starting program: '" << alarm->programFile() << "'";

                QString program = alarm->programFile();

                // if the program name contains spaces escape it
                if (program.contains(QLatin1Char(' '))
                    && !(program.startsWith(QLatin1Char('\"'))
                         && program.endsWith(QLatin1Char('\"')))) {
                    program = QLatin1Char('\"') + program + QLatin1Char('\"');
                }

                QProcess::startDetached(program + QLatin1Char(' ') + alarm->programArguments());
            } else if (alarm->type() == Alarm::Audio) {
                beeped = true;
                Phonon::MediaObject *player
                    = Phonon::createPlayer(Phonon::NotificationCategory,
                                           QUrl::fromLocalFile(alarm->audioFile()));
                player->setParent(this);
                connect(player, &Phonon::MediaObject::finished, player,
                        &Phonon::MediaObject::deleteLater);
                player->play();
            } else if (alarm->type() == Alarm::Email) {
                QString from = CalendarSupport::KCalPrefs::instance()->email();
                Identity id = mIdentityManager->identityForAddress(from);
                QString to;
                if (alarm->mailAddresses().isEmpty()) {
                    to = from;
                } else {
                    const Person::List addresses = alarm->mailAddresses();
                    QStringList add;
                    add.reserve(addresses.count());
                    Person::List::ConstIterator end(addresses.constEnd());
                    for (Person::List::ConstIterator it = addresses.constBegin();
                         it != end; ++it) {
                        add << (*it)->fullName();
                    }
                    to = add.join(QStringLiteral(", "));
                }

                QString subject;

                Akonadi::Item parentItem = mCalendar->item(alarm->parentUid());
                Incidence::Ptr parent = CalendarSupport::incidence(parentItem);

                if (alarm->mailSubject().isEmpty()) {
                    if (parent->summary().isEmpty()) {
                        subject = i18nc("@title", "Reminder");
                    } else {
                        subject = i18nc("@title", "Reminder: %1", cleanSummary(parent->summary()));
                    }
                } else {
                    subject = i18nc("@title", "Reminder: %1", alarm->mailSubject());
                }

                QString body = IncidenceFormatter::mailBodyStr(parent.staticCast<IncidenceBase>());
                if (!alarm->mailText().isEmpty()) {
                    body += QLatin1Char('\n') + alarm->mailText();
                }
                //TODO: support attachments
                KOrg::MailClient mailer;
                mailer.send(id, from, to, QString(), subject, body, true, false, QString(),
                            MailTransport::TransportManager::self()->defaultTransportName());
            }
        }
    }

    if (!beeped && found) {
        KNotification::beep();
    }
}

void AlarmDialog::wakeUp()
{
    bool activeReminders = false;
    QTreeWidgetItemIterator it(mIncidenceTree);
    QTreeWidgetItem *firstItem = nullptr;
    while (*it) {
        if (!firstItem) {
            firstItem = *it;
        }
        ReminderTreeItem *item = static_cast<ReminderTreeItem *>(*it);
        Incidence::Ptr incidence = CalendarSupport::incidence(item->mIncidence);

        if (item->mRemindAt <= QDateTime::currentDateTime()) {
            if (item->isDisabled()) {   //do not wakeup non-suspended reminders
                item->setDisabled(false);
                item->setSelected(false);
            }
            activeReminders = true;
        } else {
            item->setDisabled(true);
        }

        ++it;
    }

    if (activeReminders) {
        show();
    }
    setTimer();
    showDetails(firstItem);
    Q_EMIT reminderCount(activeCount());
}

void AlarmDialog::slotSave()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup generalConfig(config, "General");
    int numReminders = 0;

    QTreeWidgetItemIterator it(mIncidenceTree);
    while (*it) {
        ReminderTreeItem *item = static_cast<ReminderTreeItem *>(*it);
        KConfigGroup incidenceConfig(config,
                                     QStringLiteral("Incidence-%1").arg(numReminders + 1));

        Incidence::Ptr incidence = CalendarSupport::incidence(item->mIncidence);
        incidenceConfig.writeEntry("AkonadiUrl", item->mIncidence.url());
        incidenceConfig.writeEntry("RemindAt", item->mRemindAt);
        ++numReminders;
        ++it;
    }

    generalConfig.writeEntry("Reminders", numReminders);
    generalConfig.writeEntry("Position", pos());
    generalConfig.writeEntry("DefaultSuspendValue", defSuspendVal);
    generalConfig.writeEntry("DefaultSuspendUnit", defSuspendUnit);
    generalConfig.writeEntry("SuspendValue", mSuspendSpin->value());
    generalConfig.writeEntry("SuspendUnit", mSuspendUnit->currentIndex());

    config->sync();
}

AlarmDialog::ReminderList AlarmDialog::selectedItems() const
{
    ReminderList list;

    QTreeWidgetItemIterator it(mIncidenceTree);
    while (*it) {
        if ((*it)->isSelected()) {
            list.append(static_cast<ReminderTreeItem *>(*it));
        }
        ++it;
    }
    return list;
}

int AlarmDialog::activeCount()
{
    int count = 0;
    QTreeWidgetItemIterator it(mIncidenceTree);
    while (*it) {
        if (!(*it)->isDisabled()) {   //suspended reminders are non-active
            ++count;
        }
        ++it;
    }
    qCDebug(KOALARMCLIENT_LOG) << "computed " << count << " active reminders";
    return count;
}

void AlarmDialog::closeEvent(QCloseEvent *)
{
    slotSave();
    accept();
}

void AlarmDialog::updateButtons()
{
    const ReminderList selection = selectedItems();
    const int count = selection.count();
    const bool enabled = (count > 0);
    qCDebug(KOALARMCLIENT_LOG) << "selected items=" << count;
    mUser3Button->setEnabled(enabled);
    mOkButton->setEnabled(enabled);
    if (count == 1) {
        ReminderTreeItem *item = selection.first();
        if (mCalendar) {
            mUser1Button->setEnabled(mCalendar->hasRight(item->mIncidence,
                                                         Akonadi::Collection::CanChangeItem));
        }
    } else {
        mUser1Button->setEnabled(false);
    }
}

void AlarmDialog::toggleDetails(QTreeWidgetItem *item)
{
    if (!item) {
        return;
    }

    if (!mDetailView->isHidden()) {
        if (mLastItem == item) {
            resize(size().width(), size().height() - mDetailView->height() - 50);
            mDetailView->hide();
        } else {
            showDetails(item);
        }
    } else {
        resize(size().width(), size().height() + mDetailView->height() + 50);
        showDetails(item);
        mDetailView->show();
    }
    mLastItem = item;
}

void AlarmDialog::showDetails(QTreeWidgetItem *item)
{
    if (!item) {
        return;
    }

    ReminderTreeItem *reminderItem = dynamic_cast<ReminderTreeItem *>(item);

    if (!reminderItem) {
        mDetailView->setIncidence(Akonadi::Item());
    } else {
        if (!reminderItem->mDisplayText.isEmpty()) {
            const QString txt = QLatin1String("<qt><p><b>") + reminderItem->mDisplayText
                                + QLatin1String("</b></p></qt>");
            mDetailView->setHeaderText(txt);
        } else {
            mDetailView->setHeaderText(QString());
        }
        mDetailView->setIncidence(reminderItem->mIncidence, reminderItem->mRemindAt.date());
    }
}

void AlarmDialog::update()
{
    updateButtons();

    const ReminderList selection = selectedItems();
    if (!selection.isEmpty()) {
        ReminderTreeItem *item = selection.first();
        mUser1Button->setEnabled((mCalendar->hasRight(item->mIncidence,
                                                      Akonadi::Collection::CanChangeItem))
                                 && (selection.count() == 1));
        toggleDetails(item);
    }
}

void AlarmDialog::accept()
{
    if (activeCount() == 0) {
        mPos = pos();
        hide();
    }
}

/** static */
QDateTime AlarmDialog::triggerDateForIncidence(const Incidence::Ptr &incidence,
                                               const QDateTime &reminderAt, QString &displayStr)
{
    QDateTime result;

    if (incidence->alarms().isEmpty()) {
        return result;
    }

    if (incidence->recurs()) {
        result = incidence->recurrence()->getNextDateTime(reminderAt).toLocalTime();
    }

    if (!result.isValid()) {
        result = incidence->dateTime(Incidence::RoleAlarm).toLocalTime();
    }

    if (result.isValid()) {
        displayStr = QLocale().toString(result, QLocale::ShortFormat);
    }
    return result;
}

void AlarmDialog::slotCalendarChanged()
{
    KCalCore::Incidence::List incidences = mCalendar->incidences();
    const Akonadi::Item::List items = mCalendar->itemList(incidences);
    Akonadi::Item::List::ConstIterator end(items.constEnd());
    for (Akonadi::Item::List::ConstIterator it = items.constBegin();
         it != end; ++it) {
        ReminderTreeItem *item = searchByItem(*it);

        if (item) {
            Incidence::Ptr incidence = CalendarSupport::incidence(*it);
            QString displayStr;

            // Yes, alarms can be empty, if someone edited the incidence and removed all alarms
            if (!incidence->alarms().isEmpty()) {
                const auto dateTime
                    = triggerDateForIncidence(incidence, item->mRemindAt, displayStr);

                const QString summary = cleanSummary(incidence->summary());

                if (displayStr != item->text(1) || summary != item->text(0)
                    || item->mHappening != dateTime) {
                    item->setText(1, displayStr);
                    item->setText(0, summary);
                    item->mHappening = dateTime;
                }
            }
        }
    }
}

void AlarmDialog::keyPressEvent(QKeyEvent *e)
{
    const int key = e->key() | e->modifiers();

    if (key == Qt::Key_Enter || key == Qt::Key_Return) {
        e->ignore();
        return;
    }

    QDialog::keyPressEvent(e);
}

bool AlarmDialog::openIncidenceEditorThroughKOrganizer(const Incidence::Ptr &incidence)
{
    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered(QStringLiteral(
                                                                            "org.kde.korganizer")))
    {
        if (KToolInvocation::startServiceByDesktopName(QStringLiteral("org.kde.korganizer"),
                                                       QString())) {
            KMessageBox::error(
                this,
                i18nc("@info",
                      "Could not start KOrganizer so editing is not possible."));
            return false;
        }
    }
    org::kde::korganizer::Korganizer korganizer(
        QStringLiteral("org.kde.korganizer"), QStringLiteral(
            "/Korganizer"), QDBusConnection::sessionBus());

    qCDebug(KOALARMCLIENT_LOG) << "editing incidence " << incidence->summary();
    if (!korganizer.editIncidence(incidence->uid())) {
        KMessageBox::error(
            this,
            i18nc("@info",
                  "An internal KOrganizer error occurred attempting to modify \"%1\"",
                  cleanSummary(incidence->summary())));
    }

    // get desktop # where korganizer (or kontact) runs
    QString object
        = QDBusConnection::sessionBus().interface()->isServiceRegistered(QStringLiteral(
                                                                             "org.kde.kontact"))
          ? QStringLiteral("kontact/MainWindow_1") : QStringLiteral("korganizer/MainWindow_1");
    QDBusInterface korganizerObj(QStringLiteral("org.kde.korganizer"), QLatin1Char('/') + object);
#if KDEPIM_HAVE_X11
    QDBusReply<int> reply = korganizerObj.call(QStringLiteral("winId"));
    if (reply.isValid()) {
        int window = reply;
        auto winInfo = KWindowInfo(window, NET::WMDesktop);
        int desktop = winInfo.desktop();
        if (KWindowSystem::currentDesktop() == desktop) {
            KWindowSystem::minimizeWindow(winId(), false);
        } else {
            KWindowSystem::setCurrentDesktop(desktop);
        }
        KWindowSystem::activateWindow(winInfo.transientFor());
    }
#elif defined(Q_OS_WIN)
    // WId is a typedef to a void* on windows
    QDBusReply<int> reply = korganizerObj.call(QStringLiteral("winId"));
    if (reply.isValid()) {
        int window = reply;
        KWindowSystem::minimizeWindow(winId(), false);
        KWindowSystem::allowExternalProcessWindowActivation();
        KWindowSystem::activateWindow(reinterpret_cast<WId>(window));
    }
#else
    // TODO (mac)
#endif
    return true;
}

bool AlarmDialog::openIncidenceEditorNG(const Akonadi::Item &item)
{
    Incidence::Ptr incidence = CalendarSupport::incidence(item);
    IncidenceEditorNG::IncidenceDialog *dialog
        = IncidenceEditorNG::IncidenceDialogFactory::create(
        false,     /*doesn't need initial saving*/
        incidence->type(), nullptr, this);
    dialog->load(item);
    return true;
}

void AlarmDialog::removeFromConfig(const QList<Akonadi::Item::Id> &ids)
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup genGroup(config, "General");

    const int oldNumReminders = genGroup.readEntry("Reminders", 0);

    QVector<ConfItem> newReminders;
    // Delete everything
    for (int i = 1; i <= oldNumReminders; ++i) {
        const QString group(QStringLiteral("Incidence-%1").arg(i));
        KConfigGroup incGroup(config, group);
        const QString uid = incGroup.readEntry("UID");
        const QDateTime remindAtDate = incGroup.readEntry("RemindAt", QDateTime());
        const QUrl akonadiUrl(incGroup.readEntry("AkonadiUrl"));
        const Akonadi::Item::Id id = Akonadi::Item::fromUrl(akonadiUrl).id();
        if (!ids.contains(id)) {
            ConfItem ci;
            ci.akonadiUrl = akonadiUrl;
            ci.remindAt = remindAtDate;
            ci.uid = uid;
            newReminders.append(ci);
        }
        config->deleteGroup(group);
    }

    const int newRemindersCount(newReminders.count());
    genGroup.writeEntry("Reminders", newRemindersCount);

    //Write everything except those which have an uid we don't want
    for (int i = 0; i < newRemindersCount; ++i) {
        const QString group(QStringLiteral("Incidence-%1").arg(i + 1));
        KConfigGroup incGroup(config, group);
        const ConfItem conf = newReminders.at(i);
        incGroup.writeEntry("UID", conf.uid);
        incGroup.writeEntry("RemindAt", conf.remindAt);
        incGroup.writeEntry("AkonadiUrl", conf.akonadiUrl);
        incGroup.sync();
    }
    genGroup.sync();
}
