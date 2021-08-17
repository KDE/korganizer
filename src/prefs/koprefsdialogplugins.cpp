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
#include <KGuiItem>
#include <KLocalizedString>
#include <KMessageBox>
#include <KPluginFactory>
#include <KService>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QTreeWidget>
#include <QVBoxLayout>

class PluginItem : public QTreeWidgetItem
{
public:
    PluginItem(QTreeWidget *parent, const KService::Ptr &service)
        : QTreeWidgetItem(parent, {service->name()})
        , mService(service)
    {
    }

    PluginItem(QTreeWidgetItem *parent, const KService::Ptr &service)
        : QTreeWidgetItem(parent, {service->name()})
        , mService(service)
    {
    }

    KService::Ptr service()
    {
        return mService;
    }

private:
    const KService::Ptr mService;
};

K_PLUGIN_CLASS_WITH_JSON(KOPrefsDialogPlugins, "korganizer_configplugins.json")

/**
  Dialog for selecting and configuring KOrganizer plugins
*/
KOPrefsDialogPlugins::KOPrefsDialogPlugins(QWidget *parent, const QVariantList &args)
    : KPrefsModule(KOPrefs::instance(), parent, args)
{
    QBoxLayout *topTopLayout = new QVBoxLayout(this);
    mTreeWidget = new QTreeWidget(this);
    mTreeWidget->setColumnCount(1);
    mTreeWidget->setHeaderLabel(i18nc("@title:column plugin name", "Name"));
    topTopLayout->addWidget(mTreeWidget);

    mDescription = new QLabel(this);
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

    auto buttonRow = new QWidget(this);
    QBoxLayout *buttonRowLayout = new QHBoxLayout(buttonRow);
    buttonRowLayout->setContentsMargins({});
    mConfigureButton = new QPushButton(buttonRow);
    KGuiItem::assign(mConfigureButton,
                     KGuiItem(i18nc("@action:button", "Configure &Plugin..."),
                              QStringLiteral("configure"),
                              QString(),
                              i18nc("@info:whatsthis",
                                    "This button allows you to configure"
                                    " the plugin that you have selected in the list above")));
    buttonRowLayout->addWidget(mConfigureButton);
    buttonRowLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding));
    topTopLayout->addWidget(buttonRow);

    mPositioningGroupBox = new QGroupBox(i18nc("@title:group", "Position"), this);
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

    connect(mConfigureButton, &QPushButton::clicked, this, &KOPrefsDialogPlugins::configure);

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
    KService::List plugins = KOCore::self()->availablePlugins();
    plugins += KOCore::self()->availableParts();

    EventViews::PrefsPtr viewPrefs = KOPrefs::instance()->eventViewsPreferences();

    QStringList selectedPlugins = viewPrefs->selectedPlugins();

    mDecorations = new QTreeWidgetItem(mTreeWidget, QStringList(i18nc("@title:group", "Calendar Decorations")));

    KService::List::ConstIterator it;
    KService::List::ConstIterator end(plugins.constEnd());

    for (it = plugins.constBegin(); it != end; ++it) {
        QTreeWidgetItem *item = nullptr;
        if ((*it)->hasServiceType(EventViews::CalendarDecoration::Decoration::serviceType())) {
            item = new PluginItem(mDecorations, *it);
        } else {
            continue;
        }
        if (selectedPlugins.contains((*it)->desktopEntryName())) {
            item->setCheckState(0, Qt::Checked);
        } else {
            item->setCheckState(0, Qt::Unchecked);
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
                selectedPlugins.append(item->service()->desktopEntryName());
            }
        }
    }
    EventViews::PrefsPtr viewPrefs = KOPrefs::instance()->eventViewsPreferences();
    viewPrefs->setSelectedPlugins(selectedPlugins);

    KOPrefs::instance()->setDecorationsAtMonthViewTop(mDecorationsAtMonthViewTop.values());
    viewPrefs->setDecorationsAtAgendaViewTop(mDecorationsAtAgendaViewTop.values());
    viewPrefs->setDecorationsAtAgendaViewBottom(mDecorationsAtAgendaViewBottom.values());
}

void KOPrefsDialogPlugins::configure()
{
    if (mTreeWidget->selectedItems().count() != 1) {
        return;
    }

    PluginItem *item = static_cast<PluginItem *>(mTreeWidget->selectedItems().last());
    if (!item) {
        return;
    }

    CalendarSupport::Plugin *plugin = KOCore::self()->loadPlugin(item->service());

    if (plugin) {
        plugin->configure(this);
        delete plugin;

        slotWidChanged();
    } else {
        KMessageBox::sorry(this, i18nc("@info", "Unable to configure this plugin"), QStringLiteral("PluginConfigUnable"));
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

    QString decoration = item->service()->desktopEntryName();

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
        mConfigureButton->setEnabled(false);
        mDescription->setText(QString());
        return;
    }

    PluginItem *item = dynamic_cast<PluginItem *>(mTreeWidget->selectedItems().last());
    if (!item) {
        mConfigureButton->setEnabled(false);
        mConfigureButton->hide();
        mDescription->setText(QString());
        return;
    }

    QVariant variant = item->service()->property(QStringLiteral("X-KDE-KOrganizer-HasSettings"));

    bool hasSettings = false;
    if (variant.isValid()) {
        hasSettings = variant.toBool();
    }

    mDescription->setText(item->service()->comment());
    if (!hasSettings) {
        mConfigureButton->hide();
    } else {
        mConfigureButton->show();
        mConfigureButton->setEnabled(item->checkState(0) == Qt::Checked);
    }

    if (item->service()->hasServiceType(EventViews::CalendarDecoration::Decoration::serviceType())) {
        bool hasPosition = false;
        QString decoration = item->service()->desktopEntryName();
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
    }

    slotWidChanged();
}

#include "koprefsdialogplugins.moc"
