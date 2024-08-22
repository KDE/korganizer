/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2007 Volker Krause <vkrause@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "multiagendaview.h"
#include "akonadicollectionview.h"
#include "calendarview.h"
#include "koeventpopupmenu.h"
#include "prefs/koprefs.h"
#include "ui_multiagendaviewconfigwidget.h"

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
    return i18n("Agenda %1", c + 1);
}

class CalendarViewCalendarFactory : public EventViews::MultiAgendaView::CalendarFactory
{
public:
    using Ptr = QSharedPointer<CalendarViewCalendarFactory>;

    CalendarViewCalendarFactory(CalendarViewBase *calendarView)
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

class KOrg::MultiAgendaViewPrivate
{
public:
    MultiAgendaViewPrivate(CalendarViewBase *calendarView, MultiAgendaView *qq)
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
    MultiAgendaView *const q;
};

MultiAgendaView::MultiAgendaView(CalendarViewBase *calendarView, QWidget *parent)
    : KOEventView(parent)
    , d(new MultiAgendaViewPrivate(calendarView, this))
{
    connect(d->mMultiAgendaView, &EventViews::EventView::datesSelected, this, &KOEventView::datesSelected);

    connect(d->mMultiAgendaView, &EventViews::EventView::shiftedEvent, this, &KOEventView::shiftedEvent);

    connect(d->mMultiAgendaView, &EventViews::MultiAgendaView::showIncidencePopupSignal, d->mPopup, &KOEventPopupMenu::showIncidencePopup);

    connect(d->mMultiAgendaView, &EventViews::MultiAgendaView::showNewEventPopupSignal, this, &MultiAgendaView::showNewEventPopup);

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

    connect(d->mMultiAgendaView, qOverload<>(&EventViews::MultiAgendaView::newEventSignal), this, qOverload<>(&KOrg::MultiAgendaView::newEventSignal));

    connect(d->mMultiAgendaView,
            qOverload<const QDate &>(&EventViews::MultiAgendaView::newEventSignal),
            this,
            qOverload<const QDate &>(&KOrg::MultiAgendaView::newEventSignal));

    connect(d->mMultiAgendaView,
            qOverload<const QDateTime &>(&EventViews::MultiAgendaView::newEventSignal),
            this,
            qOverload<const QDateTime &>(&KOrg::MultiAgendaView::newEventSignal));

    connect(d->mMultiAgendaView,
            qOverload<const QDateTime &, const QDateTime &>(&EventViews::MultiAgendaView::newEventSignal),
            this,
            qOverload<const QDateTime &, const QDateTime &>(&KOrg::MultiAgendaView::newEventSignal));

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

MultiAgendaView::~MultiAgendaView() = default;

void MultiAgendaView::setModel(QAbstractItemModel *model)
{
    KOEventView::setModel(model);
    d->mMultiAgendaView->setModel(model);
}

Akonadi::Item::List MultiAgendaView::selectedIncidences()
{
    return d->mMultiAgendaView->selectedIncidences();
}

KCalendarCore::DateList MultiAgendaView::selectedIncidenceDates()
{
    return d->mMultiAgendaView->selectedIncidenceDates();
}

int MultiAgendaView::currentDateCount() const
{
    return d->mMultiAgendaView->currentDateCount();
}

void MultiAgendaView::showDates(const QDate &start, const QDate &end, const QDate &)
{
    d->mMultiAgendaView->showDates(start, end);
}

void MultiAgendaView::showIncidences(const Akonadi::Item::List &incidenceList, const QDate &date)
{
    d->mMultiAgendaView->showIncidences(incidenceList, date);
}

void MultiAgendaView::updateView()
{
    d->mMultiAgendaView->updateView();
}

Akonadi::Collection::Id MultiAgendaView::collectionId() const
{
    return d->mCollectionId;
}

void MultiAgendaView::changeIncidenceDisplay(const Akonadi::Item &, Akonadi::IncidenceChanger::ChangeType)
{
}

int MultiAgendaView::maxDatesHint() const
{
    return EventViews::AgendaView::MAX_DAY_COUNT;
}

void MultiAgendaView::setDateRange(const QDateTime &start, const QDateTime &end, const QDate &)
{
    d->mMultiAgendaView->setDateRange(start, end);
}

bool MultiAgendaView::eventDurationHint(QDateTime &startDt, QDateTime &endDt, bool &allDay)
{
    return d->mMultiAgendaView->eventDurationHint(startDt, endDt, allDay);
}

void MultiAgendaView::setIncidenceChanger(Akonadi::IncidenceChanger *changer)
{
    d->mMultiAgendaView->setIncidenceChanger(changer);
}

void MultiAgendaView::updateConfig()
{
    d->mMultiAgendaView->updateConfig();
}

void MultiAgendaView::setChanges(EventViews::EventView::Changes changes)
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

bool MultiAgendaView::hasConfigurationDialog() const
{
    // It has. And it's implemented in korg, not libeventviews.
    return true;
}

void MultiAgendaView::showConfigurationDialog(QWidget *parent)
{
    QPointer<MultiAgendaViewConfigDialog> dlg(new MultiAgendaViewConfigDialog(d->mCollectionSelectionModel, parent));

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

KCheckableProxyModel *MultiAgendaView::takeCustomCollectionSelectionProxyModel()
{
    return d->mMultiAgendaView->takeCustomCollectionSelectionProxyModel();
}

void MultiAgendaView::setCustomCollectionSelectionProxyModel(KCheckableProxyModel *model)
{
    d->mMultiAgendaView->setCustomCollectionSelectionProxyModel(model);
}

void MultiAgendaView::setCollectionSelectionProxyModel(KCheckableProxyModel *model)
{
    d->mCollectionSelectionModel = model;
}

class KOrg::MultiAgendaViewConfigDialogPrivate
{
public:
    MultiAgendaViewConfigDialog *const q;
    explicit MultiAgendaViewConfigDialogPrivate(QAbstractItemModel *base, MultiAgendaViewConfigDialog *qq)
        : q(qq)
        , baseModel(base)
        , currentColumn(0)
    {
    }

    ~MultiAgendaViewConfigDialogPrivate()
    {
        qDeleteAll(newlyCreated);
    }

    void setUpColumns(int n);
    AkonadiCollectionView *createView(KCheckableProxyModel *model);
    [[nodiscard]] AkonadiCollectionView *view(int index) const;
    QList<KCheckableProxyModel *> newlyCreated;
    QList<KCheckableProxyModel *> selections;
    QList<QString> titles;
    Ui::MultiAgendaViewConfigWidget ui;
    QStandardItemModel listModel;
    QAbstractItemModel *baseModel = nullptr;
    int currentColumn;
};

void MultiAgendaView::restoreConfig(const KConfigGroup &configGroup)
{
    d->mMultiAgendaView->restoreConfig(configGroup);
}

void MultiAgendaView::saveConfig(KConfigGroup &configGroup)
{
    d->mMultiAgendaView->saveConfig(configGroup);
}

void MultiAgendaView::calendarAdded(const Akonadi::CollectionCalendar::Ptr &calendar)
{
    d->mMultiAgendaView->addCalendar(calendar);
}

void MultiAgendaView::calendarRemoved(const Akonadi::CollectionCalendar::Ptr &calendar)
{
    d->mMultiAgendaView->removeCalendar(calendar);
}

MultiAgendaViewConfigDialog::MultiAgendaViewConfigDialog(QAbstractItemModel *baseModel, QWidget *parent)
    : QDialog(parent)
    , d(new MultiAgendaViewConfigDialogPrivate(baseModel, this))
{
    setWindowTitle(i18nc("@title:window", "Configure Side-By-Side View"));
    auto mainLayout = new QVBoxLayout(this);
    auto widget = new QWidget;
    d->ui.setupUi(widget);
    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &MultiAgendaViewConfigDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &MultiAgendaViewConfigDialog::reject);
    mainLayout->addWidget(buttonBox);
    mainLayout->addWidget(widget);

    d->ui.columnList->setModel(&d->listModel);
    connect(d->ui.columnList->selectionModel(), &QItemSelectionModel::currentChanged, this, &MultiAgendaViewConfigDialog::currentChanged);
    connect(d->ui.useCustomRB, &QAbstractButton::toggled, this, &MultiAgendaViewConfigDialog::useCustomToggled);
    connect(d->ui.columnNumberSB, &QSpinBox::valueChanged, this, &MultiAgendaViewConfigDialog::numberOfColumnsChanged);
    connect(d->ui.titleLE, &QLineEdit::textEdited, this, &MultiAgendaViewConfigDialog::titleEdited);
    d->setUpColumns(numberOfColumns());
    useCustomToggled(false);
}

void MultiAgendaViewConfigDialog::currentChanged(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }

    const int idx = index.data(Qt::UserRole).toInt();
    d->ui.titleLE->setText(index.data(Qt::DisplayRole).toString());
    d->ui.selectionStack->setCurrentIndex(idx);
    d->currentColumn = idx;
}

void MultiAgendaViewConfigDialog::useCustomToggled(bool on)
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

AkonadiCollectionView *MultiAgendaViewConfigDialogPrivate::createView(KCheckableProxyModel *model)
{
    auto cview = new AkonadiCollectionView(nullptr, false, q);
    cview->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    cview->setCollectionSelectionProxyModel(model);
    return cview;
}

void MultiAgendaViewConfigDialogPrivate::setUpColumns(int n)
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
            KCheckableProxyModel *const m = selections[i];
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
            sortProxy->setObjectName(QStringLiteral("MultiAgendaColumnSetupProxyModel-%1").arg(i));

            auto columnFilterProxy = new KRearrangeColumnsProxyModel(sortProxy);
            columnFilterProxy->setSourceColumns(QList<int>() << Akonadi::ETMCalendar::CollectionTitle);
            columnFilterProxy->setSourceModel(sortProxy);

            auto qsm = new QItemSelectionModel(columnFilterProxy, columnFilterProxy);

            auto selection = new KCheckableProxyModel;
            selection->setObjectName(QStringLiteral("MultiAgendaColumnCheckableProxy-%1").arg(i));
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

bool MultiAgendaViewConfigDialog::useCustomColumns() const
{
    return d->ui.useCustomRB->isChecked();
}

void MultiAgendaViewConfigDialog::setUseCustomColumns(bool custom)
{
    if (custom) {
        d->ui.useCustomRB->setChecked(true);
    } else {
        d->ui.useDefaultRB->setChecked(true);
    }
}

int MultiAgendaViewConfigDialog::numberOfColumns() const
{
    return d->ui.columnNumberSB->value();
}

void MultiAgendaViewConfigDialog::setNumberOfColumns(int n)
{
    d->ui.columnNumberSB->setValue(n);
    d->setUpColumns(n);
}

KCheckableProxyModel *MultiAgendaViewConfigDialog::takeSelectionModel(int column)
{
    if (column < 0 || column >= d->selections.size()) {
        return nullptr;
    }

    KCheckableProxyModel *const m = d->selections[column];
    d->newlyCreated.erase(std::remove(d->newlyCreated.begin(), d->newlyCreated.end(), m), d->newlyCreated.end());
    return m;
}

AkonadiCollectionView *MultiAgendaViewConfigDialogPrivate::view(int index) const
{
    return qobject_cast<AkonadiCollectionView *>(ui.selectionStack->widget(index));
}

void MultiAgendaViewConfigDialog::setSelectionModel(int column, KCheckableProxyModel *model)
{
    Q_ASSERT(column >= 0 && column < d->selections.size());

    KCheckableProxyModel *const m = d->selections[column];
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

void MultiAgendaViewConfigDialog::titleEdited(const QString &text)
{
    d->titles[d->currentColumn] = text;
    d->listModel.item(d->currentColumn)->setText(text);
}

void MultiAgendaViewConfigDialog::numberOfColumnsChanged(int number)
{
    d->setUpColumns(number);
}

QString MultiAgendaViewConfigDialog::columnTitle(int column) const
{
    Q_ASSERT(column >= 0);
    return column >= d->titles.count() ? QString() : d->titles[column];
}

void MultiAgendaViewConfigDialog::setColumnTitle(int column, const QString &title)
{
    Q_ASSERT(column >= 0);
    d->titles.resize(qMax(d->titles.size(), column + 1));
    d->titles[column] = title;
    if (QStandardItem *const item = d->listModel.item(column)) {
        item->setText(title);
    }
    // TODO update LE if item is selected
}

void MultiAgendaViewConfigDialog::accept()
{
    d->newlyCreated.clear();
    QDialog::accept();
}

MultiAgendaViewConfigDialog::~MultiAgendaViewConfigDialog() = default;

#include "moc_multiagendaview.cpp"
