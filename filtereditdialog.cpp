/*
  This file is part of KOrganizer.

  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (C) 2005 Thomas Zander <zander@kde.org>

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

#include "filtereditdialog.h"
#include "koprefs.h"

#include <calendarsupport/categoryconfig.h>
#include <libkdepim/widgets/tagwidgets.h>

#include <KCalCore/CalFilter>

#include <KMessageBox>
#include <QDebug>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <kcolorscheme.h>
#include <KHelpClient>

FilterEditDialog::FilterEditDialog(QList<KCalCore::CalFilter *> *filters, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18nc("@title::window", "Edit Calendar Filters"));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
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
    mFilterEdit = 0;
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

FilterEdit::FilterEdit(QList<KCalCore::CalFilter *> *filters, QWidget *parent)
    : QWidget(parent), mCurrent(0), mCategorySelectDialog(0)
{
    setupUi(this);
    searchline->setListWidget(mRulesList);
    mDetailsFrame->setEnabled(false);
    mFilters = filters;
    mNewButton->setWhatsThis(
        i18nc("@info:whatsthis",
              "Press this button to define a new filter."));
    mDeleteButton->setWhatsThis(
        i18nc("@info:whatsthis",
              "Press this button to remove the currently active filter."));

    connect(mRulesList, SIGNAL(itemSelectionChanged()),
            this, SLOT(filterSelected()));

    connect(mNewButton, &QPushButton::clicked, this, &FilterEdit::bNewPressed);
    connect(mDeleteButton, &QPushButton::clicked, this, &FilterEdit::bDeletePressed);
    connect(mNameLineEdit, &QLineEdit::textChanged, this, &FilterEdit::updateSelectedName);
    connect(mCatEditButton, &QPushButton::clicked, this, &FilterEdit::editCategorySelection);
    connect(mCompletedCheck, &QCheckBox::toggled, mCompletedTimeSpanLabel, &QLabel::setEnabled);
    connect(mCompletedCheck, &QCheckBox::toggled, mCompletedTimeSpan, &QSpinBox::setEnabled);

}

FilterEdit::~FilterEdit()
{
}

void FilterEdit::updateFilterList()
{
    mRulesList->clear();
    qDebug() << "  FilterEdit::updateFilterList() :" << mFilters;
    if (mFilters) {
        qDebug() << " mFilters->empty() :" << mFilters->empty();
    }
    if (!mFilters || mFilters->empty()) {
        mDetailsFrame->setEnabled(false);
        emit(dataConsistent(false));
    } else {
        QList<KCalCore::CalFilter *>::iterator i;
        for (i = mFilters->begin(); i != mFilters->end(); ++i) {
            if (*i) {
                mRulesList->addItem((*i)->name());
            }
        }
        if (mRulesList->currentRow() != -1) {
            KCalCore::CalFilter *f = mFilters->at(mRulesList->currentRow());
            if (f) {
                filterSelected(f);
            }
        }
        emit(dataConsistent(true));
    }
    if (mFilters && mFilters->count() > 0 && !mCurrent) {
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
        criteria |= KCalCore::CalFilter::HideCompletedTodos;
    }
    if (mRecurringCheck->isChecked()) {
        criteria |= KCalCore::CalFilter::HideRecurring;
    }
    if (mCatShowCheck->isChecked()) {
        criteria |= KCalCore::CalFilter::ShowCategories;
    }
    if (mHideInactiveTodosCheck->isChecked()) {
        criteria |= KCalCore::CalFilter::HideInactiveTodos;
    }
    if (mHideTodosNotAssignedToMeCheck->isChecked()) {
        criteria |= KCalCore::CalFilter::HideNoMatchingAttendeeTodos;
    }
    mCurrent->setCriteria(criteria);
    mCurrent->setCompletedTimeSpan(mCompletedTimeSpan->value());

    QStringList categoryList;
    for (int i = 0; i < mCatList->count(); ++i) {
        QListWidgetItem *item = mCatList->item(i);
        if (item) {
            categoryList.append(item->text());
        }
    }
    mCurrent->setCategoryList(categoryList);
    emit filterChanged();
}

void FilterEdit::filterSelected()
{
    if (mRulesList->currentRow() < mFilters->count()) {
        mDetailsFrame->setEnabled(true);
        filterSelected(mFilters->at(mRulesList->currentRow()));
    }
}

void FilterEdit::filterSelected(KCalCore::CalFilter *filter)
{
    if (!filter || filter == mCurrent) {
        return;
    }
    qDebug() << "Selected filter" << filter->name();
    saveChanges();

    mCurrent = filter;
    mNameLineEdit->blockSignals(true);
    mNameLineEdit->setText(mCurrent->name());
    mNameLineEdit->blockSignals(false);
    mDetailsFrame->setEnabled(true);
    mCompletedCheck->setChecked(mCurrent->criteria() & KCalCore::CalFilter::HideCompletedTodos);
    mCompletedTimeSpan->setValue(mCurrent->completedTimeSpan());
    mRecurringCheck->setChecked(mCurrent->criteria() & KCalCore::CalFilter::HideRecurring);
    mHideInactiveTodosCheck->setChecked(
        mCurrent->criteria() & KCalCore::CalFilter::HideInactiveTodos);
    mHideTodosNotAssignedToMeCheck->setChecked(
        mCurrent->criteria() & KCalCore::CalFilter::HideNoMatchingAttendeeTodos);

    if (mCurrent->criteria() & KCalCore::CalFilter::ShowCategories) {
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
    KCalCore::CalFilter *newFilter =
        new KCalCore::CalFilter(i18nc("@label default filter name",
                                      "New Filter %1", mFilters->count()));
    mFilters->append(newFilter);
    updateFilterList();
    mRulesList->setCurrentRow(mRulesList->count() - 1);
    emit filterChanged();
}

void FilterEdit::bDeletePressed()
{
    if (!mRulesList->currentItem()) {   // nothing selected
        return;
    }
    if (mFilters->isEmpty()) {   // We need at least a default filter object.
        return;
    }

    if (KMessageBox::warningContinueCancel(
                this,
                i18nc("@info",
                      "Do you really want to permanently remove the filter \"%1\"?", mCurrent->name()),
                i18nc("@title:window", "Delete Filter?"),
                KStandardGuiItem::del()) == KMessageBox::Cancel) {
        return;
    }

    int selected = mRulesList->currentRow();
    KCalCore::CalFilter *filter = mFilters->at(selected);
    mFilters->removeAll(filter);
    delete filter;
    mCurrent = 0;
    updateFilterList();
    mRulesList->setCurrentRow(qMin(mRulesList->count() - 1, selected));
    emit filterChanged();
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
        emit dataConsistent(false);
        return;
    }
    bool allOk = true;

    foreach (KCalCore::CalFilter *i, *mFilters) {
        if (i && i->name().isEmpty()) {
            allOk = false;
        }
    }

    emit dataConsistent(allOk);
}

bool FilterEdit::correctName(const QString &newText)
{
#ifndef QT_NO_STYLE_STYLESHEET
    QString styleSheet;
    if (mNegativeBackground.isEmpty()) {
        KStatefulBrush bgBrush = KStatefulBrush(KColorScheme::View, KColorScheme::NegativeBackground);
        mNegativeBackground = QStringLiteral("QLineEdit{ background-color:%1 }").arg(bgBrush.brush(mNameLineEdit).color().name());
    }
    bool negative = false;
    if (!newText.isEmpty()) {
        for (int i = 0; i < mRulesList->count(); ++i) {
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
        mCategorySelectDialog = new KPIM::TagSelectionDialog(this);
        mCategorySelectDialog->buttons()->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help);
        connect(mCategorySelectDialog, &KPIM::TagSelectionDialog::accepted, this, &FilterEdit::updateCategorySelection);
        connect(mCategorySelectDialog->buttons()->button(QDialogButtonBox::Help), &QPushButton::clicked, this, &FilterEdit::slotHelp);
    }
    mCategorySelectDialog->setSelection(mCurrent->categoryList());

    mCategorySelectDialog->show();
}

void FilterEdit::slotHelp()
{
    KHelpClient::invokeHelp(QLatin1String("categories-view"), QLatin1String("korganizer"));
}

void FilterEdit::updateCategorySelection()
{
    const QStringList &categories = mCategorySelectDialog->selection();
    mCatList->clear();
    mCatList->addItems(categories);
    mCurrent->setCategoryList(categories);
}
