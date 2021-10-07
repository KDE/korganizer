/*
  This file is part of the KDE reminder agent.

  SPDX-FileCopyrightText: 2000, 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2008-2020 Allen Winter <winter@kde.org>
  SPDX-FileCopyrightText: 2009-2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "alarmdialog.h"
#include "config-korganizer.h"
#include "koalarmclient_debug.h"
#include "korganizer_interface.h"

#include "dbusproperties.h" // DBUS-generated
#include "notifications_interface.h" // DBUS-generated

#include <CalendarSupport/IncidenceViewer>
#include <CalendarSupport/Utils>

#include <KCalUtils/IncidenceFormatter>

#include <IncidenceEditor/IncidenceDialog>
#include <IncidenceEditor/IncidenceDialogFactory>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>
#include <KNotification>
#include <KSharedConfig>
#include <KWindowSystem>

#include <QComboBox>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QTreeWidget>
#include <QVBoxLayout>

#include <phonon/mediaobject.h>

using namespace KCalendarCore;

// fallback defaults
static int defSuspendVal = 5;
static int defSuspendUnit = AlarmDialog::SuspendInMinutes;

static const char s_fdo_notifications_service[] = "org.freedesktop.Notifications";
static const char s_fdo_notifications_path[] = "/org/freedesktop/Notifications";

class ReminderTreeItem : public QTreeWidgetItem
{
public:
    ReminderTreeItem(const Akonadi::Item &incidence, QTreeWidget *parent)
        : QTreeWidgetItem(parent)
        , mIncidence(incidence)
    {
    }

    ~ReminderTreeItem() override
    {
    }

    bool operator<(const QTreeWidgetItem &other) const override;

    QString mDisplayText;

    const Akonadi::Item mIncidence;
    QDateTime mRemindAt;
    QDateTime mTrigger;
    QDateTime mHappening;
    bool mNotified = false;
};

struct ConfItem {
    QString uid;
    QUrl akonadiUrl;
    QDateTime remindAt;
};

bool ReminderTreeItem::operator<(const QTreeWidgetItem &other) const
{
    switch (treeWidget()->sortColumn()) {
    case 1: { // happening datetime
        const auto item = static_cast<const ReminderTreeItem *>(&other);
        return item->mHappening < mHappening;
    }
    case 2: { // trigger datetime
        const auto item = static_cast<const ReminderTreeItem *>(&other);
        return item->mTrigger < mTrigger;
    }
    default:
        return QTreeWidgetItem::operator<(other);
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
        connect(calendar.data(), &Akonadi::ETMCalendar::calendarChanged, this, &AlarmDialog::slotCalendarChanged);
        Akonadi::IncidenceChanger *changer = calendar->incidenceChanger();
        changer->setShowDialogsOnError(false);
    }

    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup generalConfig(config, "General");

    const QSize initialSize(424, 187);
    // split up position and size to be compatible with previous version
    // that only stored the position.
    const QPoint pos = generalConfig.readEntry("Position", QPoint());
    if (!pos.isNull()) {
        const QSize size = generalConfig.readEntry("Size", initialSize);
        mRect = QRect(pos, size);
        setGeometry(mRect);
    }

    defSuspendVal = generalConfig.readEntry("DefaultSuspendValue", defSuspendVal);
    defSuspendUnit = generalConfig.readEntry("DefaultSuspendUnit", defSuspendUnit);

    int suspendVal = defSuspendVal;
    int suspendUnit = defSuspendUnit;

    auto topBox = new QWidget(this);
    setWindowTitle(i18nc("@title:window", "Reminders"));
    setWindowIcon(QIcon::fromTheme(QStringLiteral("korgac")));
    auto buttonBox = new QDialogButtonBox(this);
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(topBox);
    mOkButton = new QPushButton;
    mOkButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    mUser1Button = new QPushButton;
    mUser1Button->setDefault(false);
    buttonBox->addButton(mUser1Button, QDialogButtonBox::ActionRole);
    mUser2Button = new QPushButton;
    mUser2Button->setDefault(false);
    buttonBox->addButton(mUser2Button, QDialogButtonBox::ActionRole);
    mUser3Button = new QPushButton;
    mUser3Button->setDefault(false);
    buttonBox->addButton(mUser3Button, QDialogButtonBox::ActionRole);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &AlarmDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &AlarmDialog::reject);
    mainLayout->addWidget(buttonBox);
    buttonBox->addButton(mOkButton, QDialogButtonBox::ActionRole);

    mUser3Button->setText(i18nc("@action:button", "Dismiss Reminder"));
    mUser3Button->setToolTip(i18nc("@info:tooltip", "Dismiss the reminders for the selected incidences"));
    mUser3Button->setWhatsThis(i18nc("@info:whatsthis",
                                     "Press this button to dismiss the currently selected incidence.  "
                                     "All non-selected non-incidences will be unaffected."));

    mUser2Button->setText(i18nc("@action:button", "Dismiss All"));
    mUser2Button->setToolTip(i18nc("@info:tooltip", "Dismiss the reminders for all listed incidences"));
    mUser2Button->setWhatsThis(i18nc("@info:whatsthis", "Press this button to dismiss all the listed incidences."));
    mUser1Button->setText(i18nc("@action:button", "Edit..."));
    mUser1Button->setToolTip(i18nc("@info:tooltip", "Edit the selected incidence"));
    mUser1Button->setWhatsThis(i18nc("@info::whatsthis",
                                     "Press this button if you want to edit the selected incidence.  "
                                     "A dialog will popup allowing you to edit the incidence."));
    mOkButton->setText(i18nc("@action:button", "Suspend"));
    mOkButton->setToolTip(i18nc("@info:tooltip",
                                "Suspend the reminders for the selected incidences "
                                "by the specified interval"));
    mOkButton->setWhatsThis(i18nc("@info:whatsthis",
                                  "Press this button to suspend the currently selected incidences.  "
                                  "The suspend interval is configurable by the Suspend duration settings."));

    auto topLayout = new QVBoxLayout(topBox);
    // Try to keep the dialog small and non-obtrusive.
    // the user can resize down to the minimum
    setMinimumSize(280, 160);
    // take out some padding which makes it larger
    topLayout->setSpacing(2);
    topLayout->setContentsMargins({});
    setContentsMargins({});

    auto label = new QLabel(i18nc("@label",
                                  "Reminders: "
                                  "Clicking on the title toggles details for item"),
                            topBox);
    topLayout->addWidget(label);

    mIncidenceTree = new QTreeWidget(topBox);
    mIncidenceTree->setColumnCount(3);
    mIncidenceTree->setSortingEnabled(true);
    const QStringList headerLabels = (QStringList(i18nc("@title:column reminder summary", "Summary"))
                                      << i18nc("@title:column happens at date/time", "Date Time") << i18nc("@title:column trigger date/time", "Trigger Time"));
    mIncidenceTree->setHeaderLabels(headerLabels);

    QHeaderView *header = mIncidenceTree->header();
    header->setSectionResizeMode(0, QHeaderView::Stretch);
    mIncidenceTree->headerItem()->setToolTip(0, i18nc("@info:tooltip", "The event or to-do summary"));

    header->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    mIncidenceTree->headerItem()->setToolTip(1, i18nc("@info:tooltip", "The reminder is set for this date/time"));

    header->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    header->setStretchLastSection(false);
    mIncidenceTree->headerItem()->setToolTip(2, i18nc("@info:tooltip", "The date/time the reminder was triggered"));

    mIncidenceTree->setWordWrap(true);
    mIncidenceTree->setAllColumnsShowFocus(true);
    mIncidenceTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    mIncidenceTree->setRootIsDecorated(false);

    topLayout->addWidget(mIncidenceTree);

    connect(mIncidenceTree, &QTreeWidget::itemClicked, this, &AlarmDialog::update);
    connect(mIncidenceTree, &QTreeWidget::itemDoubleClicked, this, &AlarmDialog::edit);
    connect(mIncidenceTree, &QTreeWidget::itemSelectionChanged, this, &AlarmDialog::updateButtons);

    mDetailView = new CalendarSupport::IncidenceViewer(mCalendar.data(), topBox);
    const QString s = xi18nc("@info default incidence details string",
                             "<emphasis>Select an event or to-do from the list above "
                             "to view its details here.</emphasis>");
    mDetailView->setDefaultMessage(s);
    topLayout->addWidget(mDetailView);
    mDetailView->hide();
    mLastItem = nullptr;

    auto suspendBox = new QWidget(topBox);
    auto suspendBoxHBoxLayout = new QHBoxLayout(suspendBox);
    suspendBoxHBoxLayout->setContentsMargins({});
    topLayout->addWidget(suspendBox);

    auto l = new QLabel(i18nc("@label:spinbox", "Suspend &duration:"), suspendBox);
    suspendBoxHBoxLayout->addWidget(l);

    mSuspendSpin = new QSpinBox(suspendBox);
    suspendBoxHBoxLayout->addWidget(mSuspendSpin);
    mSuspendSpin->setRange(1, 9999);
    mSuspendSpin->setValue(suspendVal); // default suspend duration
    mSuspendSpin->setToolTip(i18nc("@info:tooltip", "Suspend the reminders by this amount of time"));
    mSuspendSpin->setWhatsThis(i18nc("@info:whatsthis",
                                     "Each reminder for the selected incidences will be suspended "
                                     "by this number of time units. You can choose the time units "
                                     "(typically minutes) in the adjacent selector."));

    l->setBuddy(mSuspendSpin);

    mSuspendUnit = new QComboBox(suspendBox);
    suspendBoxHBoxLayout->addWidget(mSuspendUnit);
    mSuspendUnit->addItem(i18nc("@item:inlistbox suspend in terms of minutes", "minute(s)"));
    mSuspendUnit->addItem(i18nc("@item:inlistbox suspend in terms of hours", "hour(s)"));
    mSuspendUnit->addItem(i18nc("@item:inlistbox suspend in terms of days", "day(s)"));
    mSuspendUnit->addItem(i18nc("@item:inlistbox suspend in terms of weeks", "week(s)"));
    mSuspendUnit->setToolTip(i18nc("@info:tooltip", "Suspend the reminders using this time unit"));
    mSuspendUnit->setWhatsThis(i18nc("@info:whatsthis",
                                     "Each reminder for the selected incidences will be suspended "
                                     "using this time unit. You can set the number of time units "
                                     "in the adjacent number entry input."));
    mSuspendUnit->setCurrentIndex(suspendUnit);

    connect(&mSuspendTimer, &QTimer::timeout, this, &AlarmDialog::wakeUp);

    connect(mOkButton, &QPushButton::clicked, this, &AlarmDialog::slotOk);
    connect(mUser1Button, &QPushButton::clicked, this, &AlarmDialog::slotUser1);
    connect(mUser2Button, &QPushButton::clicked, this, &AlarmDialog::slotUser2);
    connect(mUser3Button, &QPushButton::clicked, this, &AlarmDialog::slotUser3);

    QDBusConnection dbusConn = QDBusConnection::sessionBus();
    if (dbusConn.interface()->isServiceRegistered(QString::fromLatin1(s_fdo_notifications_service))) {
        auto propsIface = new OrgFreedesktopDBusPropertiesInterface(QString::fromLatin1(s_fdo_notifications_service),
                                                                    QString::fromLatin1(s_fdo_notifications_path),
                                                                    dbusConn,
                                                                    this);
        connect(propsIface, &OrgFreedesktopDBusPropertiesInterface::PropertiesChanged, this, &AlarmDialog::slotDBusNotificationsPropertiesChanged);
    }
}

AlarmDialog::~AlarmDialog()
{
    mIncidenceTree->clear();
}

ReminderTreeItem *AlarmDialog::searchByItem(const Akonadi::Item &incidence)
{
    ReminderTreeItem *found = nullptr;
    QTreeWidgetItemIterator it(mIncidenceTree);
    while (*it) {
        auto item = static_cast<ReminderTreeItem *>(*it);
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
    QString retStr = summary;
    return retStr.replace(QLatin1Char('\n'), QLatin1Char(' '));
}

void AlarmDialog::addIncidence(const Akonadi::Item &incidenceitem, const QDateTime &reminderAt, const QString &displayText)
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
    QString tip = KCalUtils::IncidenceFormatter::toolTipStr(CalendarSupport::displayName(mCalendar.data(), incidenceitem.parentCollection()),
                                                            incidence,
                                                            item->mRemindAt.date(),
                                                            true);
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
    mSuspendSpin->setValue(defSuspendVal); // default suspend duration
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
        if (!(*it)->isDisabled()) { // do not disable suspended reminders
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
    for (ReminderList::const_iterator it = selections.constBegin(); it != selections.constEnd(); ++it) {
        qCDebug(KOALARMCLIENT_LOG) << "removing " << CalendarSupport::incidence((*it)->mIncidence)->summary();
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
    const ReminderList selection = selectedItems();
    if (selection.count() == 1) {
        Incidence::Ptr incidence = CalendarSupport::incidence(selection.first()->mIncidence);
        if (!mCalendar->hasRight(selection.first()->mIncidence, Akonadi::Collection::CanChangeItem)) {
            KMessageBox::sorry(this, i18nc("@info", "\"%1\" is a read-only incidence so modifications are not possible.", cleanSummary(incidence->summary())));
            return;
        }

        if (!openIncidenceEditorNG(selection.first()->mIncidence)) {
            KMessageBox::error(this,
                               i18nc("@info", "An internal error occurred attempting to modify \"%1\". Unsupported type.", cleanSummary(incidence->summary())));
            qCWarning(KOALARMCLIENT_LOG) << "Attempting to edit an unsupported incidence type.";
        }
    }
}

void AlarmDialog::suspend()
{
    if (!isVisible()) { // do nothing if the dialog is hidden
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
        if ((*it)->isSelected() && !(*it)->isDisabled()) { // suspend selected, non-suspended reminders
            (*it)->setSelected(false);
            (*it)->setDisabled(true);
            auto item = static_cast<ReminderTreeItem *>(*it);
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
        auto item = static_cast<ReminderTreeItem *>(*it);
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

void AlarmDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    // Moving to showEvent from show() because with QDialog and Qt 5.7
    // setOnAllDesktops seemed to work only if the dialog was already showing,
    // which happens when a notification appears and the dialog was already
    // up.
    KWindowSystem::setOnAllDesktops(winId(), true);
    KWindowSystem::unminimizeWindow(winId());
    KWindowSystem::setState(winId(), NET::KeepAbove | NET::DemandsAttention);
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
        auto item = static_cast<ReminderTreeItem *>(*it);
        if (!item->mNotified && !item->isDisabled()) {
            (*it)->setSelected(true);
            break;
        }
        ++it;
    }

    mUser2Button->setVisible(mIncidenceTree->topLevelItemCount() > 1);

    // reset the default suspend time
    // Allen: commented-out the following lines on 17 Sept 2013
    //  mSuspendSpin->setValue( defSuspendVal );
    //  mSuspendUnit->setCurrentIndex( defSuspendUnit );

    // move then show, avoids a visible jump if it is show then move
    if (!mRect.isNull()) {
        setGeometry(mRect);
    }
    QDialog::show();
    if (grabFocus()) {
        KWindowSystem::activateWindow(winId());
    } else {
        KWindowSystem::raiseWindow(winId());
    }

    // Audio, Procedure, and EMail alarms
    eventNotification();
}

void AlarmDialog::suspendAll()
{
    mIncidenceTree->clearSelection();
    QTreeWidgetItemIterator it(mIncidenceTree);

    // first, select all non-suspended reminders
    while (*it) {
        if (!(*it)->isDisabled()) { // do not suspend suspended reminders
            (*it)->setSelected(true);
        }
        ++it;
    }

    // suspend all selected reminders
    suspend();
}

void AlarmDialog::eventNotification()
{
    bool beeped = false;
    bool found = false;

    QTreeWidgetItemIterator it(mIncidenceTree);
    while (*it) {
        auto item = static_cast<ReminderTreeItem *>(*it);
        ++it;
        if (item->isDisabled() || item->mNotified) {
            // skip suspended reminders or reminders that have been notified
            continue;
        }
        found = true;
        item->mNotified = true;
        Incidence::Ptr incidence = CalendarSupport::incidence(item->mIncidence);
        Alarm::List alarms = incidence->alarms();
        Alarm::List::ConstIterator ait;
        for (ait = alarms.constBegin(); ait != alarms.constEnd(); ++ait) {
            Alarm::Ptr alarm = *ait;
            // we intentionally ignore Alarm::Procedure and Alarm::Email here, as that is insecure in the presence of shared calendars
            // FIXME: Check whether this should be done for all multiple alarms
            if (alarm->type() == Alarm::Audio) {
                beeped = true;
                Phonon::MediaObject *player = Phonon::createPlayer(Phonon::NotificationCategory, QUrl::fromLocalFile(alarm->audioFile()));
                player->setParent(this);
                connect(player, &Phonon::MediaObject::finished, player, &Phonon::MediaObject::deleteLater);
                player->play();
            }
        }
    }

    if (!beeped && found) {
        KNotification::beep();
    }
}

void AlarmDialog::wakeUp()
{
    // Check if notifications are inhibited (e.x. plasma "do not disturb" mode.
    // In that case, we'll wait until they are allowed again (see slotDBusNotificationsPropertiesChanged)
    QDBusConnection dbusConn = QDBusConnection::sessionBus();
    if (dbusConn.interface()->isServiceRegistered(QString::fromLatin1(s_fdo_notifications_service))) {
        OrgFreedesktopNotificationsInterface iface(QString::fromLatin1(s_fdo_notifications_service), QString::fromLatin1(s_fdo_notifications_path), dbusConn);
        if (iface.inhibited()) {
            return;
        }
    }

    bool activeReminders = false;
    QTreeWidgetItemIterator it(mIncidenceTree);
    QTreeWidgetItem *firstItem = nullptr;
    while (*it) {
        if (!firstItem) {
            firstItem = *it;
        }
        auto item = static_cast<ReminderTreeItem *>(*it);
        Incidence::Ptr incidence = CalendarSupport::incidence(item->mIncidence);

        if (item->mRemindAt <= QDateTime::currentDateTime()) {
            if (item->isDisabled()) { // do not wakeup non-suspended reminders
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

void AlarmDialog::slotDBusNotificationsPropertiesChanged(const QString &interface,
                                                         const QVariantMap &changedProperties,
                                                         const QStringList &invalidatedProperties)
{
    Q_UNUSED(interface) // always "org.freedesktop.Notifications"
    Q_UNUSED(invalidatedProperties)
    const auto it = changedProperties.find(QStringLiteral("Inhibited"));
    if (it != changedProperties.end()) {
        const bool inhibited = it.value().toBool();
        qCDebug(KOALARMCLIENT_LOG) << "Notifications inhibited:" << inhibited;
        if (!inhibited) {
            wakeUp();
        }
    }
}

void AlarmDialog::slotSave()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup generalConfig(config, "General");
    int numReminders = 0;

    QTreeWidgetItemIterator it(mIncidenceTree);
    while (*it) {
        auto item = static_cast<ReminderTreeItem *>(*it);
        KConfigGroup incidenceConfig(config, QStringLiteral("Incidence-%1").arg(numReminders + 1));

        Incidence::Ptr incidence = CalendarSupport::incidence(item->mIncidence);
        incidenceConfig.writeEntry("AkonadiUrl", item->mIncidence.url());
        incidenceConfig.writeEntry("RemindAt", item->mRemindAt);
        ++numReminders;
        ++it;
    }

    generalConfig.writeEntry("Reminders", numReminders);
    mRect = geometry();
    generalConfig.writeEntry("Position", mRect.topLeft());
    generalConfig.writeEntry("Size", mRect.size());
    generalConfig.writeEntry("DefaultSuspendValue", defSuspendVal);
    generalConfig.writeEntry("DefaultSuspendUnit", defSuspendUnit);

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
        if (!(*it)->isDisabled()) { // suspended reminders are non-active
            ++count;
        }
        ++it;
    }
    qCDebug(KOALARMCLIENT_LOG) << "computed " << count << " active reminders";
    return count;
}

void AlarmDialog::closeEvent(QCloseEvent *)
{
    // note, this is on application close (not window hide)
    slotSave();
    accept();
}

void AlarmDialog::reject()
{
    slotSave();
    QDialog::reject();
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
            mUser1Button->setEnabled(mCalendar->hasRight(item->mIncidence, Akonadi::Collection::CanChangeItem));
        }
    } else {
        mUser1Button->setEnabled(false);
    }
    if (enabled) {
        mIncidenceTree->setFocus();
        mIncidenceTree->setCurrentItem(selection.first());
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

    auto reminderItem = dynamic_cast<ReminderTreeItem *>(item);

    if (!reminderItem) {
        mDetailView->setIncidence(Akonadi::Item());
    } else {
        if (!reminderItem->mDisplayText.isEmpty()) {
            const QString txt = QLatin1String("<qt><p><b>") + reminderItem->mDisplayText + QLatin1String("</b></p></qt>");
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
        mUser1Button->setEnabled((mCalendar->hasRight(item->mIncidence, Akonadi::Collection::CanChangeItem)) && (selection.count() == 1));
        toggleDetails(item);
    }
}

void AlarmDialog::accept()
{
    if (activeCount() == 0) {
        slotSave();
        hide();
    }
}

/** static */
QDateTime AlarmDialog::triggerDateForIncidence(const Incidence::Ptr &incidence, const QDateTime &reminderAt, QString &displayStr)
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
    KCalendarCore::Incidence::List incidences = mCalendar->incidences();
    const Akonadi::Item::List items = mCalendar->itemList(incidences);
    Akonadi::Item::List::ConstIterator end(items.constEnd());
    for (Akonadi::Item::List::ConstIterator it = items.constBegin(); it != end; ++it) {
        ReminderTreeItem *item = searchByItem(*it);

        if (item) {
            Incidence::Ptr incidence = CalendarSupport::incidence(*it);
            QString displayStr;

            // Yes, alarms can be empty, if someone edited the incidence and removed all alarms
            if (!incidence->alarms().isEmpty()) {
                const auto dateTime = triggerDateForIncidence(incidence, item->mRemindAt, displayStr);
                const QString summary = cleanSummary(incidence->summary());

                if (displayStr != item->text(1) || summary != item->text(0) || item->mHappening != dateTime) {
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
    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered(QStringLiteral("org.kde.korganizer"))) {
        if (!QDBusConnection::sessionBus().interface()->startService(QStringLiteral("org.kde.korganizer")).isValid()) {
            KMessageBox::error(this, i18nc("@info", "Could not start KOrganizer so editing is not possible."));
            return false;
        }
    }
    org::kde::korganizer::Korganizer korganizer(QStringLiteral("org.kde.korganizer"), QStringLiteral("/Korganizer"), QDBusConnection::sessionBus());

    qCDebug(KOALARMCLIENT_LOG) << "editing incidence " << incidence->summary();
    if (!korganizer.editIncidence(incidence->uid())) {
        KMessageBox::error(this, i18nc("@info", "An internal KOrganizer error occurred attempting to modify \"%1\"", cleanSummary(incidence->summary())));
    }

    // get desktop # where korganizer (or kontact) runs
    QString object = QDBusConnection::sessionBus().interface()->isServiceRegistered(QStringLiteral("org.kde.kontact"))
        ? QStringLiteral("kontact/MainWindow_1")
        : QStringLiteral("korganizer/MainWindow_1");
    QDBusInterface korganizerObj(QStringLiteral("org.kde.korganizer"), QLatin1Char('/') + object);
#if KDEPIM_HAVE_X11
    QDBusReply<int> reply = korganizerObj.call(QStringLiteral("winId"));
    if (reply.isValid()) {
        int window = reply;
        auto winInfo = KWindowInfo(window, NET::WMDesktop);
        int desktop = winInfo.desktop();
        if (KWindowSystem::currentDesktop() == desktop) {
            KWindowSystem::minimizeWindow(winId());
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
        KWindowSystem::minimizeWindow(winId());
        KWindowSystem::allowExternalProcessWindowActivation();
        KWindowSystem::activateWindow(static_cast<WId>(window));
    }
#else
    // TODO (mac)
#endif
    return true;
}

bool AlarmDialog::openIncidenceEditorNG(const Akonadi::Item &item)
{
    Incidence::Ptr incidence = CalendarSupport::incidence(item);
    IncidenceEditorNG::IncidenceDialog *dialog = IncidenceEditorNG::IncidenceDialogFactory::create(false, /*doesn't need initial saving*/
                                                                                                   incidence->type(),
                                                                                                   nullptr,
                                                                                                   this);
    if (!dialog) {
        return false;
    } else {
        dialog->load(item);
        return true;
    }
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

    // Write everything except those which have an uid we don't want
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

bool AlarmDialog::grabFocus()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup generalConfig(config, "General");
    return generalConfig.readEntry("GrabFocus", false);
}
