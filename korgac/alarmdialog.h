/*
  This file is part of the KDE reminder agent.

  Copyright (c) 2000 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KORGAC_ALARMDIALOG_H
#define KORGAC_ALARMDIALOG_H

#include <Akonadi/Calendar/ETMCalendar>

#include <KCalendarCore/Incidence>

#include <QDialog>
#include <QMenu>
#include <QTimer>

namespace Akonadi {
class Item;
}

namespace KIdentityManagement {
class IdentityManager;
}

namespace CalendarSupport {
class IncidenceViewer;
}

class ReminderTreeItem;

class KComboBox;

class QDateTime;
class QTreeWidget;
class QTreeWidgetItem;
class QSpinBox;
class QToolButton;
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
    void setRemindAt(const QDateTime &dt);
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
    void slotDBusNotificationsPropertiesChanged(
            const QString& interface,
            const QVariantMap& changedProperties,
            const QStringList& invalidatedProperties);

private:
    void update();
    void updateButtons();
    typedef QList<ReminderTreeItem *> ReminderList;

    static QDateTime triggerDateForIncidence(const KCalendarCore::Incidence::Ptr &inc, const QDateTime &reminderAt, QString &displayStr);

    // Removes each Incidence-X group that has one of the specified uids
    void removeFromConfig(const QList<Akonadi::Item::Id> &);

    // Opens through dbus, @deprecated
    Q_REQUIRED_RESULT bool openIncidenceEditorThroughKOrganizer(const KCalendarCore::Incidence::Ptr &incidence);

    // opens directly
    Q_REQUIRED_RESULT bool openIncidenceEditorNG(const Akonadi::Item &incidence);

    Q_REQUIRED_RESULT bool startKOrganizer();
    ReminderTreeItem *searchByItem(const Akonadi::Item &incidence);
    void setTimer();
    void dismiss(const ReminderList &selections);
    Q_REQUIRED_RESULT int activeCount();
    Q_REQUIRED_RESULT ReminderList selectedItems() const;
    void toggleDetails(QTreeWidgetItem *item);
    void showDetails(QTreeWidgetItem *item);
    static bool grabFocus();

    Akonadi::ETMCalendar::Ptr mCalendar;
    QTreeWidget *mIncidenceTree = nullptr;
    CalendarSupport::IncidenceViewer *mDetailView = nullptr;
    KIdentityManagement::IdentityManager *mIdentityManager = nullptr;

    QRect mRect;
    QSpinBox *mSuspendSpin = nullptr;
    KComboBox *mSuspendUnit = nullptr;
    QMenu *mSuspendMenu = nullptr;
    QAction *mResetSuspendAction = nullptr;
    QAction *mSetDefaultSuspendAction = nullptr;
    QTimer mSuspendTimer;
    QTreeWidgetItem *mLastItem = nullptr;
    QPushButton *mUser1Button = nullptr;
    QPushButton *mUser2Button = nullptr;
    QPushButton *mUser3Button = nullptr;
    QToolButton *mOkButton = nullptr;
};

#endif
