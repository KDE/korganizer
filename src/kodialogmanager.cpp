/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "kodialogmanager.h"
#include "calendarview.h"
#include "dialog/filtereditdialog.h"
#include "dialog/searchdialog.h"
#include "prefs/koprefs.h"

#include <CalendarSupport/ArchiveDialog>
#include <CalendarSupport/Utils>
#include <PimCommon/PimUtil>

#include <IncidenceEditor/IncidenceDialog>
#include <IncidenceEditor/IncidenceDialogFactory>

#include <Akonadi/Item>
#include <Akonadi/TagManagementDialog>

#include <KCalendarCore/Visitor>

#include <KCMultiDialog>
#include <KPluginMetaData>
#include <QPushButton>

using namespace KOrg;

// FIXME: Handle KOEventViewerDialogs in dialog manager.

class KODialogManager::DialogManagerVisitor : public KCalendarCore::Visitor
{
public:
    DialogManagerVisitor() = default;

    bool act(KCalendarCore::IncidenceBase::Ptr &incidence, KODialogManager *manager)
    {
        mDialogManager = manager;
        return incidence->accept(*this, incidence);
    }

protected:
    KODialogManager *mDialogManager = nullptr;
};

KODialogManager::KODialogManager(CalendarView *mainView)
    : QObject()
    , mMainView(mainView)
{
}

KODialogManager::~KODialogManager()
{
    delete mOptionsDialog;
    delete mSearchDialog;
    delete mArchiveDialog;
    delete mFilterEditDialog;
    delete mCategoryEditDialog;
}

void KODialogManager::showOptionsDialog()
{
    if (!mOptionsDialog) {
        mOptionsDialog = new KCMultiDialog(mMainView);
        connect(mOptionsDialog, qOverload<>(&KCMultiDialog::configCommitted), mMainView, &CalendarView::updateConfig);
        const QVector<KPluginMetaData> availablePlugins = KPluginMetaData::findPlugins(QStringLiteral("pim/kcms/korganizer"));
        for (const KPluginMetaData &metaData : availablePlugins) {
            mOptionsDialog->addModule(metaData);
        }
    }

    mOptionsDialog->show();
    mOptionsDialog->raise();
}

void KODialogManager::showCategoryEditDialog()
{
    createCategoryEditor();
    mCategoryEditDialog->exec();
}

void KODialogManager::showSearchDialog()
{
    if (!mSearchDialog) {
        mSearchDialog = new SearchDialog(mMainView);
        connect(mMainView->calendar().data(), &Akonadi::ETMCalendar::calendarChanged, mSearchDialog, &SearchDialog::updateView);
        connect(mSearchDialog, &SearchDialog::showIncidenceSignal, mMainView, qOverload<const Akonadi::Item &>(&CalendarView::showIncidence));
        connect(mSearchDialog, &SearchDialog::editIncidenceSignal, mMainView, [=](const Akonadi::Item &i) {
            mMainView->editIncidence(i);
        });
        connect(mSearchDialog, &SearchDialog::deleteIncidenceSignal, mMainView, [=](const Akonadi::Item &i) {
            mMainView->deleteIncidence(i, false);
        });
    }
    // make sure the widget is on top again
    mSearchDialog->show();
    mSearchDialog->raise();
}

void KODialogManager::showArchiveDialog()
{
    if (!mArchiveDialog) {
        mArchiveDialog = new CalendarSupport::ArchiveDialog(mMainView->calendar(), mMainView->incidenceChanger());
        connect(mArchiveDialog, &CalendarSupport::ArchiveDialog::eventsDeleted, mMainView, qOverload<>(&CalendarView::updateView));
        connect(mArchiveDialog, &CalendarSupport::ArchiveDialog::autoArchivingSettingsModified, mMainView, &CalendarView::slotAutoArchivingSettingsModified);
    }
    mArchiveDialog->show();
    mArchiveDialog->raise();

    // Workaround.
    QApplication::restoreOverrideCursor();
}

void KODialogManager::showFilterEditDialog(QList<KCalendarCore::CalFilter *> *filters)
{
    createCategoryEditor();
    if (!mFilterEditDialog) {
        mFilterEditDialog = new FilterEditDialog(filters, mMainView);
        connect(mFilterEditDialog, &FilterEditDialog::filterChanged, mMainView, &CalendarView::updateFilter);
        connect(mFilterEditDialog, &FilterEditDialog::editCategories, mCategoryEditDialog.data(), &QWidget::show);
    }
    mFilterEditDialog->show();
    mFilterEditDialog->raise();
}

IncidenceEditorNG::IncidenceDialog *KODialogManager::createDialog(const Akonadi::Item &item)
{
    const KCalendarCore::Incidence::Ptr incidence = CalendarSupport::incidence(item);
    if (!incidence) {
        return nullptr;
    }

    IncidenceEditorNG::IncidenceDialog *dialog = IncidenceEditorNG::IncidenceDialogFactory::create(
        /*needs initial saving=*/false,
        incidence->type(),
        mMainView->incidenceChanger(),
        mMainView);

    return dialog;
}

void KODialogManager::connectTypeAhead(IncidenceEditorNG::IncidenceDialog *dialog, KOEventView *view)
{
    if (dialog && view) {
        view->setTypeAheadReceiver(dialog->typeAheadReceiver());
    }
}

void KODialogManager::updateSearchDialog()
{
    if (mSearchDialog) {
        mSearchDialog->updateView();
    }
}

void KODialogManager::createCategoryEditor()
{
    if (!mCategoryEditDialog) {
        mCategoryEditDialog = new Akonadi::TagManagementDialog(mMainView);
        QDialogButtonBox *buttons = mCategoryEditDialog->buttons();
        if (buttons) {
            buttons->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help);
            connect(buttons->button(QDialogButtonBox::Help), &QPushButton::clicked, this, &KODialogManager::slotHelp);
        }
        mCategoryEditDialog->setModal(true);
    }
}

void KODialogManager::slotHelp()
{
    PimCommon::Util::invokeHelp(QStringLiteral("korganizer/categories-view.html"));
}
