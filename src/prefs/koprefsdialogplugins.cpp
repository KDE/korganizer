/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2000, 2001, 2002, 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "koprefsdialogplugins.h"
#include "kocore.h"
#include "koprefs.h"
#include <CalendarSupport/KCalPrefs>
#include <KLocalizedString>
#include <KMessageBox>
#include <KPluginFactory>
#include <QAction>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QRadioButton>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>

class PluginItem : public QTreeWidgetItem
{
public:
    PluginItem(QTreeWidget *parent, const KPluginMetaData &service)
        : QTreeWidgetItem(parent, {service.name()})
        , mService(service)
    {
    }

    PluginItem(QTreeWidgetItem *parent, const KPluginMetaData &service)
        : QTreeWidgetItem(parent, {service.name()})
        , mService(service)
    {
    }

    KPluginMetaData service()
    {
        return mService;
    }

private:
    const KPluginMetaData mService;
};

Q_DECLARE_METATYPE(PluginItem *)

K_PLUGIN_CLASS_WITH_JSON(KOPrefsDialogPlugins, "korganizer_configplugins.json")

/**
  Dialog for selecting and configuring KOrganizer plugins
*/
KOPrefsDialogPlugins::KOPrefsDialogPlugins(QObject *parent, const KPluginMetaData &data)
    : KPrefsModule(KOPrefs::instance(), parent, data)
    , mTreeWidget(new QTreeWidget(widget()))
    , mDescription(new QLabel(widget()))
    , mPositioningGroupBox(new QGroupBox(i18nc("@title:group", "Position"), widget()))
{
    auto topTopLayout = new QVBoxLayout(widget());
    mTreeWidget->setColumnCount(2);
    mTreeWidget->setHeaderHidden(true);
    mTreeWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    mTreeWidget->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    mTreeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    mTreeWidget->header()->setStretchLastSection(false);
    topTopLayout->addWidget(mTreeWidget);

    mDescription->setAlignment(Qt::AlignVCenter);
    mDescription->setWordWrap(true);
    mDescription->setFrameShape(QLabel::Panel);
    mDescription->setFrameShadow(QLabel::Sunken);
    mDescription->setMinimumSize(QSize(0, 55));
    QSizePolicy policy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    policy.setHorizontalStretch(0);
    policy.setVerticalStretch(0);
    policy.setHeightForWidth(mDescription->sizePolicy().hasHeightForWidth());
    mDescription->setSizePolicy(policy);
    topTopLayout->addWidget(mDescription);

    // mPositionMonthTop = new QCheckBox(
    // i18nc( "@option:check", "Show in the month view" ), mPositioningGroupBox );
    mPositionAgendaTop = new QRadioButton(i18nc("@option:check", "Show at the top of the agenda views"), mPositioningGroupBox);
    mPositionAgendaBottom = new QRadioButton(i18nc("@option:check", "Show at the bottom of the agenda views"), mPositioningGroupBox);
    auto positioningLayout = new QVBoxLayout(mPositioningGroupBox);
    // positioningLayout->addWidget( mPositionMonthTop );
    positioningLayout->addWidget(mPositionAgendaTop);
    positioningLayout->addWidget(mPositionAgendaBottom);
    positioningLayout->addStretch(1);
    topTopLayout->addWidget(mPositioningGroupBox);

    connect(mPositionAgendaTop, &QRadioButton::clicked, this, &KOPrefsDialogPlugins::positioningChanged);
    connect(mPositionAgendaBottom, &QRadioButton::clicked, this, &KOPrefsDialogPlugins::positioningChanged);

    connect(mTreeWidget, &QTreeWidget::itemSelectionChanged, this, &KOPrefsDialogPlugins::selectionChanged);
    connect(mTreeWidget, &QTreeWidget::itemChanged, this, &KOPrefsDialogPlugins::selectionChanged);
    connect(mTreeWidget, &QTreeWidget::itemClicked, this, &KOPrefsDialogPlugins::slotWidChanged);

    load();

    selectionChanged();
}

KOPrefsDialogPlugins::~KOPrefsDialogPlugins()
{
    delete mDecorations;
}

void KOPrefsDialogPlugins::usrReadConfig()
{
    mTreeWidget->clear();
    auto plugins = KOCore::self()->availableCalendarDecorations();

    EventViews::PrefsPtr viewPrefs = KOPrefs::instance()->eventViewsPreferences();

    QStringList selectedPlugins = viewPrefs->selectedPlugins();

    mDecorations = new QTreeWidgetItem(mTreeWidget, QStringList(i18nc("@title:group", "Calendar Decorations")));

    for (const KPluginMetaData &plugin : plugins) {
        PluginItem *item = new PluginItem(mDecorations, plugin);

        if (selectedPlugins.contains(plugin.pluginId())) {
            item->setCheckState(0, Qt::Checked);
        } else {
            item->setCheckState(0, Qt::Unchecked);
        }
        const QVariant variant = plugin.value(QStringLiteral("X-KDE-KOrganizer-HasSettings"), false);
        const bool hasSettings = (variant.isValid() && variant.toBool());
        if (hasSettings) {
            auto but = new QToolButton(mTreeWidget);
            auto act = new QAction(but);
            const QString decoration = plugin.pluginId();
            act->setData(QVariant::fromValue<PluginItem *>(item));
            but->setDefaultAction(act);
            but->setIcon(QIcon::fromTheme(QStringLiteral("configure")));
            but->setFixedWidth(28);
            but->setToolTip(i18nc("@action", "Configure"));
            but->setAutoFillBackground(true);
            but->setEnabled(true);
            mTreeWidget->setItemWidget(item, 1, but);
            connect(but, &QToolButton::triggered, this, &KOPrefsDialogPlugins::configureClicked);
        }
    }

    mDecorations->setExpanded(true);

    const auto monthViewTop = KOPrefs::instance()->decorationsAtMonthViewTop();
    mDecorationsAtMonthViewTop = QSet<QString>(monthViewTop.begin(), monthViewTop.end());
    const auto agendaViewTop = viewPrefs->decorationsAtAgendaViewTop();
    mDecorationsAtAgendaViewTop = QSet<QString>(agendaViewTop.begin(), agendaViewTop.end());
    const auto agendaViewBottom = viewPrefs->decorationsAtAgendaViewBottom();
    mDecorationsAtAgendaViewBottom = QSet<QString>(agendaViewBottom.begin(), agendaViewBottom.end());
}

void KOPrefsDialogPlugins::usrWriteConfig()
{
    QStringList selectedPlugins;

    for (int i = 0; i < mTreeWidget->topLevelItemCount(); ++i) {
        QTreeWidgetItem *serviceTypeGroup = mTreeWidget->topLevelItem(i);
        for (int j = 0; j < serviceTypeGroup->childCount(); ++j) {
            auto item = static_cast<PluginItem *>(serviceTypeGroup->child(j));
            if (item->checkState(0) == Qt::Checked) {
                selectedPlugins.append(item->service().pluginId());
            }
        }
    }
    EventViews::PrefsPtr viewPrefs = KOPrefs::instance()->eventViewsPreferences();
    viewPrefs->setSelectedPlugins(selectedPlugins);

    KOPrefs::instance()->setDecorationsAtMonthViewTop(mDecorationsAtMonthViewTop.values());
    viewPrefs->setDecorationsAtAgendaViewTop(mDecorationsAtAgendaViewTop.values());
    viewPrefs->setDecorationsAtAgendaViewBottom(mDecorationsAtAgendaViewBottom.values());
}

void KOPrefsDialogPlugins::configureClicked(QAction *action)
{
    if (!action) {
        return;
    }

    auto item = action->data().value<PluginItem *>();

    if (!item) {
        return;
    }

    EventViews::CalendarDecoration::Decoration *plugin = KOCore::self()->loadCalendarDecoration(item->service());

    if (plugin) {
        plugin->configure(widget());
        delete plugin;

        slotWidChanged();
    } else {
        KMessageBox::error(widget(), i18nc("@info", "Unable to configure this plugin"), QStringLiteral("PluginConfigUnable"));
    }
}

void KOPrefsDialogPlugins::positioningChanged()
{
    if (mTreeWidget->selectedItems().count() != 1) {
        return;
    }

    PluginItem *item = dynamic_cast<PluginItem *>(mTreeWidget->selectedItems().last());
    if (!item) {
        return;
    }

    QString decoration = item->service().pluginId();

    /*if ( mPositionMonthTop->checkState() == Qt::Checked ) {
      if ( !mDecorationsAtMonthViewTop.contains( decoration ) ) {
        mDecorationsAtMonthViewTop.insert( decoration );
      }
    } else {
      mDecorationsAtMonthViewTop.remove( decoration );
    }*/

    if (mPositionAgendaTop->isChecked()) {
        if (!mDecorationsAtAgendaViewTop.contains(decoration)) {
            mDecorationsAtAgendaViewTop.insert(decoration);
        }
    } else {
        mDecorationsAtAgendaViewTop.remove(decoration);
    }

    if (mPositionAgendaBottom->isChecked()) {
        if (!mDecorationsAtAgendaViewBottom.contains(decoration)) {
            mDecorationsAtAgendaViewBottom.insert(decoration);
        }
    } else {
        mDecorationsAtAgendaViewBottom.remove(decoration);
    }

    slotWidChanged();
}

void KOPrefsDialogPlugins::selectionChanged()
{
    mPositioningGroupBox->hide();
    // mPositionMonthTop->setChecked( false );
    mPositionAgendaTop->setChecked(false);
    mPositionAgendaBottom->setChecked(false);

    if (mTreeWidget->selectedItems().count() != 1) {
        mDescription->setText(QString());
        return;
    }

    PluginItem *item = dynamic_cast<PluginItem *>(mTreeWidget->selectedItems().last());
    if (!item) {
        mDescription->setText(QString());
        return;
    }

    mDescription->setText(item->service().description());

    bool hasPosition = false;
    QString decoration = item->service().pluginId();
    /*if ( mDecorationsAtMonthViewTop.contains( decoration ) ) {
      mPositionMonthTop->setChecked( true );
      hasPosition = true;
    }*/
    if (mDecorationsAtAgendaViewTop.contains(decoration)) {
        mPositionAgendaTop->setChecked(true);
        hasPosition = true;
    }
    if (mDecorationsAtAgendaViewBottom.contains(decoration)) {
        mPositionAgendaBottom->setChecked(true);
        hasPosition = true;
    }

    if (!hasPosition) {
        // no position has been selected, so default to Agenda Top
        mDecorationsAtAgendaViewTop << decoration;
        mPositionAgendaTop->setChecked(true);
    }

    mPositioningGroupBox->setEnabled(item->checkState(0) == Qt::Checked);
    mPositioningGroupBox->show();

    slotWidChanged();
}

#include "koprefsdialogplugins.moc"

#include "moc_koprefsdialogplugins.cpp"
