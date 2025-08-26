/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2000, 2001, 2002, 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "koprefsdialogcolorsandfonts.h"
#include "koprefs.h"
#include <Akonadi/CollectionComboBox>
#include <Akonadi/EntityTreeModel>
#include <Akonadi/TagCache>
#include <Akonadi/TagSelectionComboBox>
#include <CalendarSupport/KCalPrefs>
#include <KColorButton>
#include <KComboBox>
#include <KLocalizedString>
#include <KPluginFactory>
#include <QCheckBox>
#include <QFontDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTabWidget>

K_PLUGIN_CLASS_WITH_JSON(KOPrefsDialogColorsAndFonts, "korganizer_configcolorsandfonts.json")

KOPrefsDialogColorsAndFonts::KOPrefsDialogColorsAndFonts(QObject *parent, const KPluginMetaData &data)
    : Korganizer::KPrefsModule(KOPrefs::instance(), parent, data)
{
    auto topTopLayout = new QVBoxLayout(widget());
    topTopLayout->setContentsMargins({});
    auto tabWidget = new QTabWidget(widget());
    tabWidget->setDocumentMode(true);
    topTopLayout->addWidget(tabWidget);

    auto colorFrame = new QWidget(widget());
    topTopLayout->addWidget(colorFrame);
    auto colorLayout = new QGridLayout(colorFrame);
    tabWidget->addTab(colorFrame, QIcon::fromTheme(QStringLiteral("preferences-desktop-color")), i18nc("@title:tab", "Colors"));

    // Use System color
    Korganizer::KPrefsWidBool *useSystemColorBool = addWidBool(KOPrefs::instance()->useSystemColorItem(), colorFrame);

    const QCheckBox *useSystemColorButton = useSystemColorBool->checkBox();
    QObject::connect(useSystemColorButton, &QCheckBox::toggled, this, &KOPrefsDialogColorsAndFonts::useSystemColorToggle);
    colorLayout->addWidget(useSystemColorBool->checkBox(), 1, 0, 1, 2);

    // agenda view background color
    Korganizer::KPrefsWidColor *agendaBgColor = addWidColor(KOPrefs::instance()->agendaGridBackgroundColorItem(), colorFrame);
    KColorButton *agendaBgColorButton = agendaBgColor->button();
    mButtonsDisable.push_back(agendaBgColorButton);
    colorLayout->addWidget(agendaBgColor->label(), 2, 0);
    colorLayout->addWidget(agendaBgColorButton, 2, 1);

    Korganizer::KPrefsWidColor *viewBgBusyColor = addWidColor(KOPrefs::instance()->viewBgBusyColorItem(), colorFrame);
    KColorButton *viewBgBusyColorButton = viewBgBusyColor->button();
    mButtonsDisable.push_back(viewBgBusyColorButton);
    colorLayout->addWidget(viewBgBusyColor->label(), 3, 0);
    colorLayout->addWidget(viewBgBusyColorButton, 3, 1);

    // working hours color
    Korganizer::KPrefsWidColor *agendaGridWorkHoursBackgroundColor = addWidColor(KOPrefs::instance()->workingHoursColorItem(), colorFrame);
    KColorButton *agendaGridWorkHoursBackgroundColorButton = agendaGridWorkHoursBackgroundColor->button();
    mButtonsDisable.push_back(agendaGridWorkHoursBackgroundColorButton);
    colorLayout->addWidget(agendaGridWorkHoursBackgroundColor->label(), 4, 0);
    colorLayout->addWidget(agendaGridWorkHoursBackgroundColor->button(), 4, 1);

    // agenda view Marcus Bains line color
    Korganizer::KPrefsWidColor *mblColor = addWidColor(KOPrefs::instance()->agendaMarcusBainsLineLineColorItem(), colorFrame);
    colorLayout->addWidget(mblColor->label(), 5, 0);
    colorLayout->addWidget(mblColor->button(), 5, 1);

    // Holiday Color
    Korganizer::KPrefsWidColor *holidayColor = addWidColor(KOPrefs::instance()->agendaHolidaysBackgroundColorItem(), colorFrame);
    colorLayout->addWidget(holidayColor->label(), 6, 0);
    colorLayout->addWidget(holidayColor->button(), 6, 1);

    // Todo due today color
    Korganizer::KPrefsWidColor *todoDueTodayColor = addWidColor(KOPrefs::instance()->todoDueTodayColorItem(), colorFrame);
    colorLayout->addWidget(todoDueTodayColor->label(), 7, 0);
    colorLayout->addWidget(todoDueTodayColor->button(), 7, 1);

    // Todo overdue color
    Korganizer::KPrefsWidColor *todoOverdueColor = addWidColor(KOPrefs::instance()->todoOverdueColorItem(), colorFrame);
    colorLayout->addWidget(todoOverdueColor->label(), 8, 0);
    colorLayout->addWidget(todoOverdueColor->button(), 8, 1);

    // categories colors
    auto categoryGroup = new QGroupBox(i18nc("@title:group", "Tags"), colorFrame);
    colorLayout->addWidget(categoryGroup, 9, 0, 1, 2);

    auto categoryLayout = new QGridLayout;
    categoryGroup->setLayout(categoryLayout);

    Korganizer::KPrefsWidColor *unsetCategoryColor = addWidColor(CalendarSupport::KCalPrefs::instance()->unsetCategoryColorItem(), categoryGroup);
    categoryLayout->addWidget(unsetCategoryColor->label(), 0, 0);
    categoryLayout->addWidget(unsetCategoryColor->button(), 0, 1);
    unsetCategoryColor->label()->setWhatsThis(unsetCategoryColor->button()->whatsThis());
    unsetCategoryColor->label()->setToolTip(unsetCategoryColor->button()->toolTip());

    mCategoryCombo = new Akonadi::TagSelectionComboBox(categoryGroup);
    mCategoryCombo->setWhatsThis(i18nc("@info:whatsthis",
                                       "Select the tag you want to modify. "
                                       "You can change the selected tag's color using "
                                       "the adjacent color button."));
    connect(mCategoryCombo, &KComboBox::activated, this, &KOPrefsDialogColorsAndFonts::updateCategoryColor);
    categoryLayout->addWidget(mCategoryCombo, 1, 0);

    mCategoryButton = new KColorButton(categoryGroup);
    mCategoryButton->setWhatsThis(i18nc("@info:whatsthis",
                                        "Choose the color of the tag selected "
                                        "in the adjacent combo box."));
    connect(mCategoryButton, &KColorButton::changed, this, &KOPrefsDialogColorsAndFonts::setCategoryColor);
    categoryLayout->addWidget(mCategoryButton, 1, 1);

    updateCategoryColor();

    // resources colors
    auto resourceGroup = new QGroupBox(i18nc("@title:group", "Resources"), colorFrame);
    colorLayout->addWidget(resourceGroup, 10, 0, 1, 2);

    QBoxLayout *resourceLayout = new QHBoxLayout;
    resourceGroup->setLayout(resourceLayout);

    mResourceCombo = new Akonadi::CollectionComboBox(resourceGroup);
    // mResourceCombo->addExcludedSpecialResources(Akonadi::Collection::SearchResource);
    const QStringList mimetypes{KCalendarCore::Todo::todoMimeType(), KCalendarCore::Journal::journalMimeType(), KCalendarCore::Event::eventMimeType()};
    mResourceCombo->setMimeTypeFilter(mimetypes);
    mResourceCombo->setWhatsThis(i18nc("@info:whatsthis",
                                       "Select the calendar you want to modify. "
                                       "You can change the selected calendar color using "
                                       "the button below."));
    connect(mResourceCombo, &Akonadi::CollectionComboBox::activated, this, &KOPrefsDialogColorsAndFonts::updateResourceColor);
    resourceLayout->addWidget(mResourceCombo);

    mResourceButton = new KColorButton(resourceGroup);
    mResourceButton->setWhatsThis(i18nc("@info:whatsthis",
                                        "Choose here the color of the calendar selected "
                                        "using the combo box above."));
    connect(mResourceButton, &KColorButton::changed, this, &KOPrefsDialogColorsAndFonts::setResourceColor);
    resourceLayout->addWidget(mResourceButton);

    colorLayout->setRowStretch(11, 1);

    auto fontFrame = new QWidget(widget());
    tabWidget->addTab(fontFrame, QIcon::fromTheme(QStringLiteral("preferences-desktop-font")), i18nc("@title:tab", "Fonts"));

    auto fontLayout = new QGridLayout(fontFrame);

    Korganizer::KPrefsWidFont *timeBarFont =
        addWidFont(KOPrefs::instance()->agendaTimeLabelsFontItem(), fontFrame, QLocale().toString(QTime(12, 34), QLocale::ShortFormat));
    fontLayout->addWidget(timeBarFont->label(), 0, 0);
    fontLayout->addWidget(timeBarFont->preview(), 0, 1);
    fontLayout->addWidget(timeBarFont->button(), 0, 2);

    Korganizer::KPrefsWidFont *monthViewFont = addWidFont(KOPrefs::instance()->monthViewFontItem(),
                                                          fontFrame,
                                                          QLocale().toString(QTime(12, 34), QLocale::ShortFormat) + u' ' + i18nc("@label", "Event text"));

    fontLayout->addWidget(monthViewFont->label(), 1, 0);
    fontLayout->addWidget(monthViewFont->preview(), 1, 1);
    fontLayout->addWidget(monthViewFont->button(), 1, 2);

    Korganizer::KPrefsWidFont *agendaViewFont = addWidFont(KOPrefs::instance()->agendaViewFontItem(), fontFrame, i18nc("@label", "Event text"));
    fontLayout->addWidget(agendaViewFont->label(), 2, 0);
    fontLayout->addWidget(agendaViewFont->preview(), 2, 1);
    fontLayout->addWidget(agendaViewFont->button(), 2, 2);

    Korganizer::KPrefsWidFont *marcusBainsFont =
        addWidFont(KOPrefs::instance()->agendaMarcusBainsLineFontItem(), fontFrame, QLocale().toString(QTime(12, 34, 23), QLocale::ShortFormat));
    fontLayout->addWidget(marcusBainsFont->label(), 3, 0);
    fontLayout->addWidget(marcusBainsFont->preview(), 3, 1);
    fontLayout->addWidget(marcusBainsFont->button(), 3, 2);

    fontLayout->setColumnStretch(1, 1);
    fontLayout->setRowStretch(4, 1);

    load();
}

void KOPrefsDialogColorsAndFonts::usrWriteConfig()
{
    QHash<QString, QColor>::const_iterator i = mCategoryDict.constBegin();
    while (i != mCategoryDict.constEnd()) {
        Akonadi::TagCache::instance()->setTagColor(i.key(), i.value());
        ++i;
    }

    i = mResourceDict.constBegin();
    while (i != mResourceDict.constEnd()) {
        KOPrefs::instance()->setResourceColor(i.key(), i.value());
        ++i;
    }

    setNeedsSave(false);
    // mCalendarViewsPrefs->writeConfig();
}

void KOPrefsDialogColorsAndFonts::usrReadConfig()
{
    updateCategories();
    updateResources();
    // mCalendarViewsPrefs->readConfig();
}

void KOPrefsDialogColorsAndFonts::useSystemColorToggle(bool useSystemColor)
{
    for (KColorButton *colorButton : std::as_const(mButtonsDisable)) {
        if (useSystemColor) {
            colorButton->setEnabled(false);
        } else {
            colorButton->setEnabled(true);
        }
    }
}

void KOPrefsDialogColorsAndFonts::updateCategories()
{
    updateCategoryColor();
}

void KOPrefsDialogColorsAndFonts::setCategoryColor()
{
    mCategoryDict.insert(mCategoryCombo->currentText(), mCategoryButton->color());
    slotWidChanged();
}

void KOPrefsDialogColorsAndFonts::updateCategoryColor()
{
    const QString cat = mCategoryCombo->currentText();
    QColor color = mCategoryDict.value(cat);
    if (!color.isValid()) {
        color = Akonadi::TagCache::instance()->tagColor(cat);
    }
    if (color.isValid()) {
        mCategoryButton->setColor(color);
    }
}

void KOPrefsDialogColorsAndFonts::updateResources()
{
    updateResourceColor();
}

void KOPrefsDialogColorsAndFonts::setResourceColor()
{
    bool ok;
    const QString id = QString::number(mResourceCombo->itemData(mResourceCombo->currentIndex(), Akonadi::EntityTreeModel::CollectionIdRole).toLongLong(&ok));
    if (!ok) {
        return;
    }
    mResourceDict.insert(id, mResourceButton->color());
    slotWidChanged();
}

void KOPrefsDialogColorsAndFonts::updateResourceColor()
{
    bool ok;
    const QString id = QString::number(mResourceCombo->itemData(mResourceCombo->currentIndex(), Akonadi::EntityTreeModel::CollectionIdRole).toLongLong(&ok));
    if (!ok) {
        return;
    }
    qCDebug(KORGANIZER_LOG) << id << mResourceCombo->itemText(mResourceCombo->currentIndex());

    QColor color = mResourceDict.value(id);
    if (!color.isValid()) {
        color = KOPrefs::instance()->resourceColor(id);
    }
    mResourceButton->setColor(color);
}

#include "koprefsdialogcolorsandfonts.moc"

#include "moc_koprefsdialogcolorsandfonts.cpp"
