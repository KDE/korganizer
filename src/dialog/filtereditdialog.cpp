/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  SPDX-FileCopyrightText: 2005 Thomas Zander <zander@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "filtereditdialog.h"
#include "korganizer_debug.h"
#include <KStatefulBrush>

#include <KCalendarCore/CalFilter>

#include <KLineEditEventHandler>

#include <Akonadi/TagSelectionDialog>

#include <PimCommon/PimUtil>

#include <KColorScheme>
#include <KMessageBox>

#include <QDialogButtonBox>

FilterEditDialog::FilterEditDialog(QList<KCalendarCore::CalFilter *> *filters, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18nc("@title::window", "Edit Calendar Filters"));
    auto mainLayout = new QVBoxLayout(this);

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply, this);
    mOkButton = buttonBox->button(QDialogButtonBox::Ok);
    mApplyButton = buttonBox->button(QDialogButtonBox::Apply);
    mOkButton->setDefault(true);
    mOkButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &FilterEditDialog::slotOk);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &FilterEditDialog::reject);
    mainLayout->addWidget(mFilterEdit = new FilterEdit(filters, this));
    mainLayout->addWidget(buttonBox);

    connect(mFilterEdit, &FilterEdit::dataConsistent, this, &FilterEditDialog::setDialogConsistent);
    updateFilterList();
    connect(mFilterEdit, &FilterEdit::editCategories, this, &FilterEditDialog::editCategories);
    connect(mFilterEdit, &FilterEdit::filterChanged, this, &FilterEditDialog::filterChanged);
    connect(buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &FilterEditDialog::slotApply);
}

FilterEditDialog::~FilterEditDialog()
{
    delete mFilterEdit;
    mFilterEdit = nullptr;
}

void FilterEditDialog::updateFilterList()
{
    mFilterEdit->updateFilterList();
}

void FilterEditDialog::slotApply()
{
    mFilterEdit->saveChanges();
}

void FilterEditDialog::slotOk()
{
    slotApply();
    accept();
}

void FilterEditDialog::setDialogConsistent(bool consistent)
{
    mOkButton->setEnabled(consistent);
    mApplyButton->setEnabled(consistent);
}

FilterEdit::FilterEdit(QList<KCalendarCore::CalFilter *> *filters, QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
    searchline->setListWidget(mRulesList);
    KLineEditEventHandler::catchReturnKey(mNameLineEdit);
    mDetailsFrame->setEnabled(false);
    mFilters = filters;
    mNewButton->setWhatsThis(i18nc("@info:whatsthis", "Press this button to define a new filter."));
    mDeleteButton->setWhatsThis(i18nc("@info:whatsthis", "Press this button to remove the currently active filter."));

    connect(mRulesList, &QListWidget::itemSelectionChanged, this, qOverload<>(&FilterEdit::filterSelected));

    connect(mNewButton, &QPushButton::clicked, this, &FilterEdit::bNewPressed);
    connect(mDeleteButton, &QPushButton::clicked, this, &FilterEdit::bDeletePressed);
    connect(mNameLineEdit, &QLineEdit::textChanged, this, &FilterEdit::updateSelectedName);
    connect(mCatEditButton, &QPushButton::clicked, this, &FilterEdit::editCategorySelection);
    connect(mCompletedCheck, &QCheckBox::toggled, mCompletedTimeSpanLabel, &QLabel::setEnabled);
    connect(mCompletedCheck, &QCheckBox::toggled, mCompletedTimeSpan, &QSpinBox::setEnabled);
}

FilterEdit::~FilterEdit() = default;

void FilterEdit::updateFilterList()
{
    mRulesList->clear();
    if (!mFilters || mFilters->empty()) {
        mDetailsFrame->setEnabled(false);
        Q_EMIT dataConsistent(false);
    } else {
        QList<KCalendarCore::CalFilter *>::iterator i;
        QList<KCalendarCore::CalFilter *>::iterator const end(mFilters->end());
        for (i = mFilters->begin(); i != end; ++i) {
            if (*i) {
                mRulesList->addItem((*i)->name());
            }
        }
        if (mRulesList->currentRow() != -1) {
            KCalendarCore::CalFilter *f = mFilters->at(mRulesList->currentRow());
            if (f) {
                filterSelected(f);
            }
        }
        Q_EMIT dataConsistent(true);
    }
    if (mFilters && !mFilters->isEmpty() && !mCurrent) {
        filterSelected(mFilters->at(0));
    }
    if (mFilters) {
        mDeleteButton->setEnabled(!mFilters->isEmpty());
    }
}

void FilterEdit::saveChanges()
{
    if (!mCurrent) {
        return;
    }

    mCurrent->setName(mNameLineEdit->text());
    int criteria = 0;
    if (mCompletedCheck->isChecked()) {
        criteria |= KCalendarCore::CalFilter::HideCompletedTodos;
    }
    if (mRecurringCheck->isChecked()) {
        criteria |= KCalendarCore::CalFilter::HideRecurring;
    }
    if (mCatShowCheck->isChecked()) {
        criteria |= KCalendarCore::CalFilter::ShowCategories;
    }
    if (mHideInactiveTodosCheck->isChecked()) {
        criteria |= KCalendarCore::CalFilter::HideInactiveTodos;
    }
    if (mHideTodosNotAssignedToMeCheck->isChecked()) {
        criteria |= KCalendarCore::CalFilter::HideNoMatchingAttendeeTodos;
    }
    mCurrent->setCriteria(criteria);
    mCurrent->setCompletedTimeSpan(mCompletedTimeSpan->value());

    QStringList categoryList;
    const int numberOfCat(mCatList->count());
    categoryList.reserve(numberOfCat);
    for (int i = 0; i < numberOfCat; ++i) {
        QListWidgetItem *item = mCatList->item(i);
        if (item) {
            categoryList.append(item->text());
        }
    }
    mCurrent->setCategoryList(categoryList);
    Q_EMIT filterChanged();
}

void FilterEdit::filterSelected()
{
    if (mRulesList->currentRow() < mFilters->count()) {
        mDetailsFrame->setEnabled(true);
        filterSelected(mFilters->at(mRulesList->currentRow()));
    }
}

void FilterEdit::filterSelected(KCalendarCore::CalFilter *filter)
{
    if (!filter || filter == mCurrent) {
        return;
    }
    qCDebug(KORGANIZER_LOG) << "Selected filter" << filter->name();
    saveChanges();

    mCurrent = filter;
    mNameLineEdit->blockSignals(true);
    mNameLineEdit->setText(mCurrent->name());
    mNameLineEdit->blockSignals(false);
    mDetailsFrame->setEnabled(true);
    mCompletedCheck->setChecked(mCurrent->criteria() & KCalendarCore::CalFilter::HideCompletedTodos);
    mCompletedTimeSpan->setValue(mCurrent->completedTimeSpan());
    mRecurringCheck->setChecked(mCurrent->criteria() & KCalendarCore::CalFilter::HideRecurring);
    mHideInactiveTodosCheck->setChecked(mCurrent->criteria() & KCalendarCore::CalFilter::HideInactiveTodos);
    mHideTodosNotAssignedToMeCheck->setChecked(mCurrent->criteria() & KCalendarCore::CalFilter::HideNoMatchingAttendeeTodos);

    if (mCurrent->criteria() & KCalendarCore::CalFilter::ShowCategories) {
        mCatShowCheck->setChecked(true);
    } else {
        mCatHideCheck->setChecked(true);
    }
    mCatList->clear();
    mCatList->addItems(mCurrent->categoryList());
}

void FilterEdit::bNewPressed()
{
    mDetailsFrame->setEnabled(true);
    saveChanges();
    QStringList filterNames;
    const int numRules = mRulesList->count();
    filterNames.reserve(numRules);
    for (int i = 0; i < numRules; ++i) {
        filterNames << mRulesList->item(i)->text();
    }
    QString newFilterName;
    for (int i = 1;; ++i) {
        newFilterName = i18nc("@label default filter name", "New Filter %1", i);
        if (!filterNames.contains(newFilterName)) {
            break;
        }
    }

    auto newFilter = new KCalendarCore::CalFilter(newFilterName);
    mFilters->append(newFilter);
    updateFilterList();
    mRulesList->setCurrentRow(mRulesList->count() - 1);
    Q_EMIT filterChanged();
}

void FilterEdit::bDeletePressed()
{
    if (!mRulesList->currentItem()) { // nothing selected
        return;
    }
    if (mFilters->isEmpty()) { // We need at least a default filter object.
        return;
    }

    if (KMessageBox::warningContinueCancel(this,
                                           i18nc("@info", "Do you really want to permanently remove the filter \"%1\"?", mCurrent->name()),
                                           i18nc("@title:window", "Delete Filter?"),
                                           KStandardGuiItem::del())
        == KMessageBox::Cancel) {
        return;
    }

    int const selected = mRulesList->currentRow();
    KCalendarCore::CalFilter *filter = mFilters->at(selected);
    mFilters->removeAll(filter);
    delete filter;
    mCurrent = nullptr;
    updateFilterList();
    mRulesList->setCurrentRow(qMin(mRulesList->count() - 1, selected));
    Q_EMIT filterChanged();
}

void FilterEdit::updateSelectedName(const QString &newText)
{
    mRulesList->blockSignals(true);
    QListWidgetItem *item = mRulesList->currentItem();
    if (item) {
        item->setText(newText);
    }
    mRulesList->blockSignals(false);
    if (correctName(newText)) {
        Q_EMIT dataConsistent(false);
        return;
    }
    bool allOk = true;

    for (KCalendarCore::CalFilter *i : std::as_const(*mFilters)) {
        if (i && i->name().isEmpty()) {
            allOk = false;
        }
    }

    Q_EMIT dataConsistent(allOk);
}

bool FilterEdit::correctName(const QString &newText)
{
    bool negative = false;
#ifndef QT_NO_STYLE_STYLESHEET
    QString styleSheet;
    if (mNegativeBackground.isEmpty()) {
        const KStatefulBrush bgBrush = KStatefulBrush(KColorScheme::View, KColorScheme::NegativeBackground);
        mNegativeBackground = QStringLiteral("QLineEdit{ background-color:%1 }").arg(bgBrush.brush(mNameLineEdit->palette()).color().name());
    }
    if (!newText.isEmpty()) {
        const int val = mRulesList->count();
        for (int i = 0; i < val; ++i) {
            QListWidgetItem *item = mRulesList->item(i);
            if (item && (mRulesList->currentItem() != item)) {
                if (newText == item->text()) {
                    negative = true;
                    break;
                }
            }
        }
    } else {
        negative = true;
    }
    if (negative) {
        styleSheet = mNegativeBackground;
    }
    mNameLineEdit->setStyleSheet(styleSheet);
#endif
    return negative;
}

void FilterEdit::editCategorySelection()
{
    if (!mCurrent) {
        return;
    }

    if (!mCategorySelectDialog) {
        mCategorySelectDialog = new Akonadi::TagSelectionDialog(this);
        QDialogButtonBox *buttons = mCategorySelectDialog->buttons();
        if (buttons) {
            buttons->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help);
            connect(mCategorySelectDialog, &Akonadi::TagSelectionDialog::accepted, this, &FilterEdit::updateCategorySelection);
            connect(buttons->button(QDialogButtonBox::Help), &QPushButton::clicked, this, &FilterEdit::slotHelp);
        }
    }
    Akonadi::Tag::List tags;
    const auto names = mCurrent->categoryList();
    tags.resize(names.size());
    // NOLINTBEGIN(modernize-use-ranges) to avoid detaching
    std::transform(names.cbegin(), names.cend(), std::back_inserter(tags), [](const QString &name) {
        return Akonadi::Tag{name};
    });
    // NOLINTEND(modernize-use-ranges)
    mCategorySelectDialog->setSelection(tags);

    mCategorySelectDialog->show();
}

void FilterEdit::slotHelp()
{
    PimCommon::Util::invokeHelp(QStringLiteral("korganizer/filters-view.html"));
}

void FilterEdit::updateCategorySelection()
{
    const auto tags = mCategorySelectDialog->selection();
    QStringList categories;
    categories.reserve(tags.size());
    // NOLINTBEGIN(modernize-use-ranges) to avoid detaching
    std::transform(tags.cbegin(), tags.cend(), std::back_inserter(categories), std::bind(&Akonadi::Tag::name, std::placeholders::_1));
    // NOLINTEND(modernize-use-ranges)
    mCatList->clear();
    mCatList->addItems(categories);
    mCurrent->setCategoryList(categories);
}

#include "moc_filtereditdialog.cpp"
