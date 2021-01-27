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
#include <CalendarSupport/CategoryConfig>
#include <CalendarSupport/Utils>
#include <PimCommon/PimUtil>

#include <IncidenceEditor/IncidenceDialog>
#include <IncidenceEditor/IncidenceDialogFactory>

#include <AkonadiCore/Item>
#include <AkonadiWidgets/TagManagementDialog>

#include <KCalendarCore/Visitor>

#include <KCMultiDialog>
#include <QPushButton>

using namespace KOrg;

// FIXME: Handle KOEventViewerDialogs in dialog manager.

class KODialogManager::DialogManagerVisitor : public KCalendarCore::Visitor
{
public:
    DialogManagerVisitor()
    {
    }

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
        connect(mOptionsDialog,
                qOverload<const QByteArray &>(&KCMultiDialog::configCommitted),
                mMainView,
                qOverload<const QByteArray &>(&CalendarView::updateConfig));
        QStringList modules;

        modules.append(QStringLiteral("korganizer_configmain.desktop"));
        modules.append(QStringLiteral("korganizer_configtime.desktop"));
        modules.append(QStringLiteral("korganizer_configviews.desktop"));
        modules.append(QStringLiteral("korganizer_configcolorsandfonts.desktop"));
        modules.append(QStringLiteral("korganizer_configgroupscheduling.desktop"));
        modules.append(QStringLiteral("korganizer_configfreebusy.desktop"));
        modules.append(QStringLiteral("korganizer_configplugins.desktop"));
        modules.append(QStringLiteral("korganizer_configdesignerfields.desktop"));
#ifdef WITH_KUSERFEEDBACK
        modules.append(QStringLiteral("korganizer_userfeedback.desktop"));
#endif
        // add them all
        const QStringList::iterator mitEnd(modules.end());
        for (QStringList::iterator mit = modules.begin(); mit != mitEnd; ++mit) {
            mOptionsDialog->addModule(*mit);
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
        // mSearchDialog->setCalendar( mMainView->calendar() );
        connect(mSearchDialog, SIGNAL(showIncidenceSignal(Akonadi::Item)), mMainView, SLOT(showIncidence(Akonadi::Item)));
        connect(mSearchDialog, SIGNAL(editIncidenceSignal(Akonadi::Item)), mMainView, SLOT(editIncidence(Akonadi::Item)));
        connect(mSearchDialog, SIGNAL(deleteIncidenceSignal(Akonadi::Item)), mMainView, SLOT(deleteIncidence(Akonadi::Item)));
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

void KODialogManager::connectEditor(IncidenceEditorNG::IncidenceDialog *editor)
{
    createCategoryEditor();
    connect(editor, SIGNAL(deleteIncidenceSignal(Akonadi::Item)), mMainView, SLOT(deleteIncidence(Akonadi::Item)));

    connect(editor, SIGNAL(dialogClose(Akonadi::Item)), mMainView, SLOT(dialogClosing(Akonadi::Item)));
    connect(editor, SIGNAL(deleteAttendee(Akonadi::Item)), mMainView, SIGNAL(cancelAttendees(Akonadi::Item)));
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
