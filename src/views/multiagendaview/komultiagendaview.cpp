/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2007 Volker Krause <vkrause@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "komultiagendaview.h"
#include "akonadicollectionview.h"
#include "calendarview.h"
#include "koeventpopupmenu.h"
#include "prefs/koprefs.h"

#include "ui_komultiagendaviewconfigwidget.h"

#include <EventViews/AgendaView>
#include <EventViews/MultiAgendaView>

#include <Akonadi/EntityTreeModel>
#include <Akonadi/EntityTreeView>

#include <KCheckableProxyModel>

#include <KRearrangeColumnsProxyModel>
#include <QDialogButtonBox>
#include <QPushButton>

#include <KConfigGroup>
#include <QHBoxLayout>
#include <QSortFilterProxyModel>
#include <QStandardItem>

using namespace KOrg;

static QString generateColumnLabel(int c)
{
    return i18nc("@title:column", "Agenda %1", c + 1);
}

namespace
{
class CalendarViewCalendarFactory : public EventViews::MultiAgendaView::CalendarFactory
{
public:
    using Ptr = QSharedPointer<CalendarViewCalendarFactory>;

    explicit CalendarViewCalendarFactory(CalendarViewBase *calendarView)
        : mView(calendarView)
    {
    }

    Akonadi::CollectionCalendar::Ptr calendarForCollection(const Akonadi::Collection &collection) override
    {
        return mView->calendarForCollection(collection);
    }

private:
    CalendarViewBase *const mView;
};
}

class KOMultiAgendaViewPrivate
{
public:
    KOMultiAgendaViewPrivate(CalendarViewBase *calendarView, KOMultiAgendaView *qq)
        : q(qq)
    {
        auto layout = new QHBoxLayout(q);
        mMultiAgendaView = new EventViews::MultiAgendaView(CalendarViewCalendarFactory::Ptr::create(calendarView), q);
        mMultiAgendaView->setPreferences(KOPrefs::instance()->eventViewsPreferences());
        layout->addWidget(mMultiAgendaView);

        mPopup = q->eventPopup();
    }

    EventViews::MultiAgendaView *mMultiAgendaView = nullptr;
    KOEventPopupMenu *mPopup = nullptr;
    KCheckableProxyModel *mCollectionSelectionModel = nullptr;
    Akonadi::Collection::Id mCollectionId = -1;

private:
    Q_DISABLE_COPY_MOVE(KOMultiAgendaViewPrivate)
    KOMultiAgendaView *const q;
};

KOMultiAgendaView::KOMultiAgendaView(CalendarViewBase *calendarView, QWidget *parent)
    : KOEventView(parent)
    , d(new KOMultiAgendaViewPrivate(calendarView, this))
{
    connect(d->mMultiAgendaView, &EventViews::EventView::datesSelected, this, &KOEventView::datesSelected);

    connect(d->mMultiAgendaView, &EventViews::EventView::shiftedEvent, this, &KOEventView::shiftedEvent);

    connect(d->mMultiAgendaView, &EventViews::MultiAgendaView::showIncidencePopupSignal, d->mPopup, &KOEventPopupMenu::showIncidencePopup);

    connect(d->mMultiAgendaView, &EventViews::MultiAgendaView::showNewEventPopupSignal, this, &KOMultiAgendaView::showNewEventPopup);

    connect(d->mMultiAgendaView, &EventViews::EventView::incidenceSelected, this, &BaseView::incidenceSelected);

    connect(d->mMultiAgendaView, &EventViews::EventView::showIncidenceSignal, this, &BaseView::showIncidenceSignal);

    connect(d->mMultiAgendaView, &EventViews::EventView::editIncidenceSignal, this, &BaseView::editIncidenceSignal);

    connect(d->mMultiAgendaView, &EventViews::EventView::deleteIncidenceSignal, this, &BaseView::deleteIncidenceSignal);

    connect(d->mMultiAgendaView, &EventViews::EventView::cutIncidenceSignal, this, &BaseView::cutIncidenceSignal);

    connect(d->mMultiAgendaView, &EventViews::EventView::copyIncidenceSignal, this, &BaseView::copyIncidenceSignal);

    connect(d->mMultiAgendaView, &EventViews::EventView::pasteIncidenceSignal, this, &BaseView::pasteIncidenceSignal);

    connect(d->mMultiAgendaView, &EventViews::EventView::toggleAlarmSignal, this, &BaseView::toggleAlarmSignal);

    connect(d->mMultiAgendaView, &EventViews::EventView::toggleTodoCompletedSignal, this, &BaseView::toggleTodoCompletedSignal);

    connect(d->mMultiAgendaView, &EventViews::EventView::copyIncidenceToResourceSignal, this, &BaseView::copyIncidenceToResourceSignal);

    connect(d->mMultiAgendaView, &EventViews::EventView::moveIncidenceToResourceSignal, this, &BaseView::moveIncidenceToResourceSignal);

    connect(d->mMultiAgendaView, &EventViews::EventView::dissociateOccurrencesSignal, this, &BaseView::dissociateOccurrencesSignal);

    connect(d->mMultiAgendaView, qOverload<>(&EventViews::MultiAgendaView::newEventSignal), this, qOverload<>(&KOMultiAgendaView::newEventSignal));

    connect(d->mMultiAgendaView,
            qOverload<const QDate &>(&EventViews::MultiAgendaView::newEventSignal),
            this,
            qOverload<const QDate &>(&KOMultiAgendaView::newEventSignal));

    connect(d->mMultiAgendaView,
            qOverload<const QDateTime &>(&EventViews::MultiAgendaView::newEventSignal),
            this,
            qOverload<const QDateTime &>(&KOMultiAgendaView::newEventSignal));

    connect(d->mMultiAgendaView,
            qOverload<const QDateTime &, const QDateTime &>(&EventViews::MultiAgendaView::newEventSignal),
            this,
            qOverload<const QDateTime &, const QDateTime &>(&KOMultiAgendaView::newEventSignal));

    connect(d->mMultiAgendaView, &EventViews::EventView::newTodoSignal, this, &BaseView::newTodoSignal);

    connect(d->mMultiAgendaView, &EventViews::EventView::newSubTodoSignal, this, &BaseView::newSubTodoSignal);

    connect(d->mMultiAgendaView, &EventViews::EventView::newJournalSignal, this, &BaseView::newJournalSignal);

    connect(d->mMultiAgendaView, &EventViews::MultiAgendaView::activeCalendarChanged, this, [this](const Akonadi::CollectionCalendar::Ptr &calendar) {
        if (calendar) {
            d->mCollectionId = calendar->collection().id();
        } else {
            d->mCollectionId = -1;
        }
    });
}

KOMultiAgendaView::~KOMultiAgendaView() = default;

void KOMultiAgendaView::setModel(QAbstractItemModel *model)
{
    KOEventView::setModel(model);
    d->mMultiAgendaView->setModel(model);
}

Akonadi::Item::List KOMultiAgendaView::selectedIncidences()
{
    return d->mMultiAgendaView->selectedIncidences();
}

KCalendarCore::DateList KOMultiAgendaView::selectedIncidenceDates()
{
    return d->mMultiAgendaView->selectedIncidenceDates();
}

int KOMultiAgendaView::currentDateCount() const
{
    return d->mMultiAgendaView->currentDateCount();
}

void KOMultiAgendaView::showDates(const QDate &start, const QDate &end, const QDate &)
{
    d->mMultiAgendaView->showDates(start, end);
}

void KOMultiAgendaView::showIncidences(const Akonadi::Item::List &incidenceList, const QDate &date)
{
    d->mMultiAgendaView->showIncidences(incidenceList, date);
}

void KOMultiAgendaView::updateView()
{
    d->mMultiAgendaView->updateView();
}

Akonadi::Collection::Id KOMultiAgendaView::collectionId() const
{
    return d->mCollectionId;
}

void KOMultiAgendaView::changeIncidenceDisplay(const Akonadi::Item &, Akonadi::IncidenceChanger::ChangeType)
{
}

int KOMultiAgendaView::maxDatesHint() const
{
    return EventViews::AgendaView::MAX_DAY_COUNT;
}

void KOMultiAgendaView::setDateRange(const QDateTime &start, const QDateTime &end, const QDate &)
{
    d->mMultiAgendaView->setDateRange(start, end);
}

bool KOMultiAgendaView::eventDurationHint(QDateTime &startDt, QDateTime &endDt, bool &allDay)
{
    return d->mMultiAgendaView->eventDurationHint(startDt, endDt, allDay);
}

void KOMultiAgendaView::setIncidenceChanger(Akonadi::IncidenceChanger *changer)
{
    d->mMultiAgendaView->setIncidenceChanger(changer);
}

void KOMultiAgendaView::updateConfig()
{
    d->mMultiAgendaView->updateConfig();
}

void KOMultiAgendaView::setChanges(EventViews::EventView::Changes changes)
{
    // Only ConfigChanged and FilterChanged should go from korg->AgendaView
    // All other values are already detected inside AgendaView.
    // We could just pass "changes", but korganizer does a very bad job at
    // determining what changed, for example if you move an incidence
    // the BaseView::setDateRange(...) is called causing DatesChanged
    // flag to be on, when no dates changed.
    EventViews::EventView::Changes c;
    if (changes.testFlag(EventViews::EventView::ConfigChanged)) {
        c = EventViews::EventView::ConfigChanged;
    }

    if (changes.testFlag(EventViews::EventView::FilterChanged)) {
        c |= EventViews::EventView::FilterChanged;
    }

    d->mMultiAgendaView->setChanges(c | d->mMultiAgendaView->changes());
}

bool KOMultiAgendaView::hasConfigurationDialog() const
{
    // It has. And it's implemented in korg, not libeventviews.
    return true;
}

void KOMultiAgendaView::showConfigurationDialog(QWidget *parent)
{
    const QPointer<KOMultiAgendaViewConfigDialog> dlg(new KOMultiAgendaViewConfigDialog(d->mCollectionSelectionModel, parent));

    dlg->setUseCustomColumns(d->mMultiAgendaView->customColumnSetupUsed());
    dlg->setNumberOfColumns(d->mMultiAgendaView->customNumberOfColumns());

    QList<KCheckableProxyModel *> models = d->mMultiAgendaView->collectionSelectionModels();
    for (int i = 0; i < models.size(); ++i) {
        dlg->setSelectionModel(i, models[i]);
    }

    QStringList customColumnTitles = d->mMultiAgendaView->customColumnTitles();
    const int numTitles = customColumnTitles.size();
    for (int i = 0; i < numTitles; ++i) {
        dlg->setColumnTitle(i, customColumnTitles[i]);
    }

    if (dlg->exec() == QDialog::Accepted) {
        d->mMultiAgendaView->customCollectionsChanged(dlg);
    }

    delete dlg;
}

KCheckableProxyModel *KOMultiAgendaView::takeCustomCollectionSelectionProxyModel()
{
    return d->mMultiAgendaView->takeCustomCollectionSelectionProxyModel();
}

void KOMultiAgendaView::setCustomCollectionSelectionProxyModel(KCheckableProxyModel *model)
{
    d->mMultiAgendaView->setCustomCollectionSelectionProxyModel(model);
}

void KOMultiAgendaView::setCollectionSelectionProxyModel(KCheckableProxyModel *model)
{
    d->mCollectionSelectionModel = model;
}

class KOMultiAgendaViewConfigDialogPrivate
{
public:
    explicit KOMultiAgendaViewConfigDialogPrivate(QAbstractItemModel *base, KOMultiAgendaViewConfigDialog *qq)
        : q(qq)
        , baseModel(base)
    {
    }

    ~KOMultiAgendaViewConfigDialogPrivate()
    {
        qDeleteAll(newlyCreated);
    }

    void setUpColumns(int n);
    [[nodiscard]] AkonadiCollectionView *view(int index) const;
    AkonadiCollectionView *createView(KCheckableProxyModel *model);
    KOMultiAgendaViewConfigDialog *const q;
    QList<KCheckableProxyModel *> newlyCreated;
    QList<KCheckableProxyModel *> selections;
    QList<QString> titles;
    Ui::KOMultiAgendaViewConfigWidget ui;
    QStandardItemModel listModel;
    QAbstractItemModel *baseModel = nullptr;
    int currentColumn = 0;
};

void KOMultiAgendaView::restoreConfig(const KConfigGroup &configGroup)
{
    d->mMultiAgendaView->restoreConfig(configGroup);
}

void KOMultiAgendaView::saveConfig(KConfigGroup &configGroup)
{
    d->mMultiAgendaView->saveConfig(configGroup);
}

void KOMultiAgendaView::calendarAdded(const Akonadi::CollectionCalendar::Ptr &calendar)
{
    d->mMultiAgendaView->addCalendar(calendar);
}

void KOMultiAgendaView::calendarRemoved(const Akonadi::CollectionCalendar::Ptr &calendar)
{
    d->mMultiAgendaView->removeCalendar(calendar);
}

bool KOMultiAgendaView::showSideBar()
{
    // follow agenda view settings
    return KOPrefs::instance()->agendaViewShowSidebar();
}

void KOMultiAgendaView::setShowSideBar(bool show)
{
    // follow agenda view settings
    KOPrefs::instance()->setAgendaViewShowSidebar(show);
}

KOMultiAgendaViewConfigDialog::KOMultiAgendaViewConfigDialog(QAbstractItemModel *baseModel, QWidget *parent)
    : QDialog(parent)
    , d(new KOMultiAgendaViewConfigDialogPrivate(baseModel, this))
{
    setWindowTitle(i18nc("@title:window", "Configure Side-By-Side View"));
    auto mainLayout = new QVBoxLayout(this);
    auto widget = new QWidget;
    d->ui.setupUi(widget);
    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &KOMultiAgendaViewConfigDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &KOMultiAgendaViewConfigDialog::reject);
    mainLayout->addWidget(buttonBox);
    mainLayout->addWidget(widget);

    d->ui.columnList->setModel(&d->listModel);
    connect(d->ui.columnList->selectionModel(), &QItemSelectionModel::currentChanged, this, &KOMultiAgendaViewConfigDialog::currentChanged);
    connect(d->ui.useCustomRB, &QAbstractButton::toggled, this, &KOMultiAgendaViewConfigDialog::useCustomToggled);
    connect(d->ui.columnNumberSB, &QSpinBox::valueChanged, this, &KOMultiAgendaViewConfigDialog::numberOfColumnsChanged);
    connect(d->ui.titleLE, &QLineEdit::textEdited, this, &KOMultiAgendaViewConfigDialog::titleEdited);
    d->setUpColumns(d->ui.columnNumberSB->value());
    useCustomToggled(false);
}

void KOMultiAgendaViewConfigDialog::currentChanged(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }

    const int idx = index.data(Qt::UserRole).toInt();
    d->ui.titleLE->setText(index.data(Qt::DisplayRole).toString());
    d->ui.selectionStack->setCurrentIndex(idx);
    d->currentColumn = idx;
}

void KOMultiAgendaViewConfigDialog::useCustomToggled(bool on)
{
    d->ui.columnList->setEnabled(on);
    d->ui.columnNumberLabel->setEnabled(on);
    d->ui.columnNumberSB->setEnabled(on);
    d->ui.selectedCalendarsLabel->setEnabled(on);
    d->ui.selectionStack->setEnabled(on);
    d->ui.titleLabel->setEnabled(on);
    d->ui.titleLE->setEnabled(on);
    // this explicit enabling/disabling of the ETV is necessary, as the stack
    // widget state is not propagated to the collection views. Probably because
    // the Akonadi error overlays enable/disable the ETV explicitly and thus
    // override the parent-child relationship?
    for (int i = 0; i < d->ui.selectionStack->count(); ++i) {
        d->view(i)->view()->setEnabled(on);
    }
}

AkonadiCollectionView *KOMultiAgendaViewConfigDialogPrivate::createView(KCheckableProxyModel *model)
{
    auto cview = new AkonadiCollectionView(nullptr, false, q);
    cview->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    cview->setCollectionSelectionProxyModel(model);
    return cview;
}

void KOMultiAgendaViewConfigDialogPrivate::setUpColumns(int n)
{
    Q_ASSERT(n > 0);
    const int oldN = selections.size();
    if (oldN == n) {
        return;
    }

    if (n < oldN) {
        for (int i = oldN - 1; i >= n; --i) {
            QWidget *w = ui.selectionStack->widget(i);
            ui.selectionStack->removeWidget(w);
            delete w;
            qDeleteAll(listModel.takeRow(i));
            const KCheckableProxyModel *const m = selections[i];
            selections.remove(i);
            const int pos = newlyCreated.indexOf(m);
            if (pos != -1) {
                delete m;
                newlyCreated.remove(pos);
            }
        }
    } else {
        selections.resize(n);
        for (int i = oldN; i < n; ++i) {
            auto item = new QStandardItem;
            item->setEditable(false);
            if (titles.count() <= i) {
                titles.resize(i + 1);
                titles[i] = generateColumnLabel(i);
            }
            item->setText(titles[i]);
            item->setData(i, Qt::UserRole);
            listModel.appendRow(item);

            auto sortProxy = new QSortFilterProxyModel;
            sortProxy->setSourceModel(baseModel);
            sortProxy->setObjectName(QStringLiteral("KOMultiAgendaColumnSetupProxyModel-%1").arg(i));

            auto columnFilterProxy = new KRearrangeColumnsProxyModel(sortProxy);
            columnFilterProxy->setSourceColumns(QList<int>() << Akonadi::ETMCalendar::CollectionTitle);
            columnFilterProxy->setSourceModel(sortProxy);

            auto qsm = new QItemSelectionModel(columnFilterProxy, columnFilterProxy);

            auto selection = new KCheckableProxyModel;
            selection->setObjectName(QStringLiteral("KOMultiAgendaColumnCheckableProxy-%1").arg(i));
            selection->setSourceModel(columnFilterProxy);
            selection->setSelectionModel(qsm);

            AkonadiCollectionView *cview = createView(selection);
            const int idx = ui.selectionStack->addWidget(cview);
            Q_ASSERT(i == idx);
            Q_UNUSED(idx)
            selections[i] = selection;
            newlyCreated.push_back(selection);
        }
    }
}

bool KOMultiAgendaViewConfigDialog::useCustomColumns() const
{
    return d->ui.useCustomRB->isChecked();
}

void KOMultiAgendaViewConfigDialog::setUseCustomColumns(bool custom)
{
    if (custom) {
        d->ui.useCustomRB->setChecked(true);
    } else {
        d->ui.useDefaultRB->setChecked(true);
    }
}

int KOMultiAgendaViewConfigDialog::numberOfColumns() const
{
    return d->ui.columnNumberSB->value();
}

void KOMultiAgendaViewConfigDialog::setNumberOfColumns(int n)
{
    d->ui.columnNumberSB->setValue(n);
    d->setUpColumns(n);
}

KCheckableProxyModel *KOMultiAgendaViewConfigDialog::takeSelectionModel(int column)
{
    if (column < 0 || column >= d->selections.size()) {
        return nullptr;
    }

    KCheckableProxyModel *const m = d->selections[column];
    d->newlyCreated.erase(std::remove(d->newlyCreated.begin(), d->newlyCreated.end(), m), d->newlyCreated.end());
    return m;
}

AkonadiCollectionView *KOMultiAgendaViewConfigDialogPrivate::view(int index) const
{
    return qobject_cast<AkonadiCollectionView *>(ui.selectionStack->widget(index));
}

void KOMultiAgendaViewConfigDialog::setSelectionModel(int column, KCheckableProxyModel *model)
{
    Q_ASSERT(column >= 0 && column < d->selections.size());

    const KCheckableProxyModel *const m = d->selections[column];
    if (m == model) {
        return;
    }

    AkonadiCollectionView *cview = d->view(column);
    Q_ASSERT(cview);
    cview->setCollectionSelectionProxyModel(model);

    if (d->newlyCreated.contains(m)) {
        d->newlyCreated.erase(std::remove(d->newlyCreated.begin(), d->newlyCreated.end(), m), d->newlyCreated.end());
        delete m;
    }

    d->selections[column] = model;
}

void KOMultiAgendaViewConfigDialog::titleEdited(const QString &text)
{
    d->titles[d->currentColumn] = text;
    d->listModel.item(d->currentColumn)->setText(text);
}

void KOMultiAgendaViewConfigDialog::numberOfColumnsChanged(int number)
{
    d->setUpColumns(number);
}

QString KOMultiAgendaViewConfigDialog::columnTitle(int column) const
{
    Q_ASSERT(column >= 0);
    return column >= d->titles.count() ? QString() : d->titles[column];
}

void KOMultiAgendaViewConfigDialog::setColumnTitle(int column, const QString &title)
{
    Q_ASSERT(column >= 0);
    d->titles.resize(qMax(d->titles.size(), column + 1));
    d->titles[column] = title;
    if (QStandardItem *const item = d->listModel.item(column)) {
        item->setText(title);
    }
    // TODO update LE if item is selected
}

void KOMultiAgendaViewConfigDialog::accept()
{
    d->newlyCreated.clear();
    QDialog::accept();
}

KOMultiAgendaViewConfigDialog::~KOMultiAgendaViewConfigDialog() = default;

#include "moc_komultiagendaview.cpp"
