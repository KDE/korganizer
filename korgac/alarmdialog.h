/*
  This file is part of the KDE reminder agent.

  SPDX-FileCopyrightText: 2000 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#ifndef KORGAC_ALARMDIALOG_H
#define KORGAC_ALARMDIALOG_H

#include <Akonadi/Calendar/ETMCalendar>

#include <QDialog>
#include <QTimer>

class ReminderTreeItem;

namespace KIdentityManagement {
class IdentityManager;
}

namespace Akonadi {
class Item;
}

namespace CalendarSupport {
class IncidenceViewer;
}

class QComboBox;

class QSpinBox;
class QTreeWidget;
class QTreeWidgetItem;

class AlarmDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * The suspend time unit.
     */
    enum SuspendUnit {
        SuspendInMinutes = 0,  ///< Suspend time is in minutes
        SuspendInHours = 1,    ///< Suspend time is in hours
        SuspendInDays = 2,     ///< Suspend time is in days
        SuspendInWeeks = 3     ///< Suspend time is in weeks
    };
    Q_ENUM(SuspendUnit)

    explicit AlarmDialog(const Akonadi::ETMCalendar::Ptr &calendar, QWidget *parent = nullptr);
    ~AlarmDialog();

    void addIncidence(const Akonadi::Item &incidence, const QDateTime &reminderAt, const QString &displayText);
    void eventNotification();

public Q_SLOTS:
    void slotOk();    // suspend
    void slotUser1(); // edit
    void slotUser2(); // dismiss all
    void slotUser3(); // dismiss selected
    void resetSuspend(); //reset the suspend value to the default
    void setDefaultSuspend(); //set current suspend value as the default
    void slotSave();
    void wakeUp();
    void show();
    void edit();
    void suspend();
    void suspendAll();
    void dismissAll();
    void dismissCurrent();
    /*reimp*/
    void accept() override;
    void reject() override;

    /**
       If an incidence changed, for example in korg, we must update
       the date and summary shown in the list view.
    */
    void slotCalendarChanged();

Q_SIGNALS:
    void reminderCount(int count);

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void closeEvent(QCloseEvent *) override;
    void showEvent(QShowEvent *event) override;

private Q_SLOTS:
    void slotDBusNotificationsPropertiesChanged(const QString &interface, const QVariantMap &changedProperties, const QStringList &invalidatedProperties);

private:
    void update();
    void updateButtons();
    using ReminderList = QList<ReminderTreeItem *>;

    static Q_REQUIRED_RESULT QDateTime triggerDateForIncidence(const KCalendarCore::Incidence::Ptr &inc, const QDateTime &reminderAt, QString &displayStr);

    // Removes each Incidence-X group that has one of the specified uids
    void removeFromConfig(const QList<Akonadi::Item::Id> &);

    // Opens through dbus, @deprecated
    Q_REQUIRED_RESULT bool openIncidenceEditorThroughKOrganizer(const KCalendarCore::Incidence::Ptr &incidence);

    // opens directly
    Q_REQUIRED_RESULT bool openIncidenceEditorNG(const Akonadi::Item &incidence);

    ReminderTreeItem *searchByItem(const Akonadi::Item &incidence);
    void setTimer();
    void dismiss(const ReminderList &selections);
    Q_REQUIRED_RESULT int activeCount();
    Q_REQUIRED_RESULT ReminderList selectedItems() const;
    void toggleDetails(QTreeWidgetItem *item);
    void showDetails(QTreeWidgetItem *item);
    static Q_REQUIRED_RESULT bool grabFocus();

    Akonadi::ETMCalendar::Ptr mCalendar;
    QTreeWidget *mIncidenceTree = nullptr;
    CalendarSupport::IncidenceViewer *mDetailView = nullptr;
    KIdentityManagement::IdentityManager *mIdentityManager = nullptr;

    QRect mRect;
    QSpinBox *mSuspendSpin = nullptr;
    QComboBox *mSuspendUnit = nullptr;
    QTimer mSuspendTimer;
    QTreeWidgetItem *mLastItem = nullptr;
    QPushButton *mUser1Button = nullptr;
    QPushButton *mUser2Button = nullptr;
    QPushButton *mUser3Button = nullptr;
    QPushButton *mOkButton = nullptr;
};

#endif
