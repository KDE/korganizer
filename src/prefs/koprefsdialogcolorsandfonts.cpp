/*
  This file is part of KOrganizer.

  Copyright (c) 2000,2001,2002,2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include "koprefsdialogcolorsandfonts.h"
#include "koprefs.h"
#include <KLocalizedString>

#include <CalendarSupport/KCalPrefs>
#include <CollectionComboBox>
#include <KColorButton>
#include <KComboBox>
#include <QCheckBox>
#include <QFontDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTabWidget>
#include <TagSelectionComboBox>
#include <AkonadiCore/EntityTreeModel>

FontPreviewButton::FontPreviewButton(const QString &labelStr, QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    QLabel *label = new QLabel(labelStr, this);
    mainLayout->addWidget(label);

    mPreview = new QLabel(parent);
    mPreview->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    mainLayout->addWidget(mPreview);

    QPushButton *button = new QPushButton(i18n("Choose..."), this);
    mainLayout->addWidget(button);
    connect(button, &QPushButton::clicked, this, &FontPreviewButton::selectFont);
}

void FontPreviewButton::setFont(const QFont &font)
{
    mPreview->setFont(font);
}

QFont FontPreviewButton::font() const
{
    return mPreview->font();
}

void FontPreviewButton::setPreviewText(const QString &str)
{
    mPreview->setText(str);
}

void FontPreviewButton::selectFont()
{
    bool ok;
    const QFont myFont = QFontDialog::getFont(&ok, mPreview->font());
    if (ok) {
        mPreview->setFont(myFont);
        Q_EMIT changed();
    }
}

KOPrefsDialogColorsAndFonts::KOPrefsDialogColorsAndFonts(QWidget *parent)
    : KCModule(parent)
{
    QBoxLayout *topTopLayout = new QVBoxLayout(this);
    QTabWidget *tabWidget = new QTabWidget(this);
    topTopLayout->addWidget(tabWidget);

    QWidget *colorFrame = new QWidget(this);
    topTopLayout->addWidget(colorFrame);
    QGridLayout *colorLayout = new QGridLayout(colorFrame);
    tabWidget->addTab(colorFrame, QIcon::fromTheme(QStringLiteral("preferences-desktop-color")),
                      i18nc("@title:tab", "Colors"));

    // Use System color
    mUseSystemColorCheckBox = new QCheckBox(KOPrefs::instance()->useSystemColorItem()->label(), colorFrame);
    connect(mUseSystemColorCheckBox, &QCheckBox::toggled,
            this, &KOPrefsDialogColorsAndFonts::useSystemColorToggle);
    colorLayout->addWidget(mUseSystemColorCheckBox, 1, 0, 1, 2);

    // agenda view background color
    mAgendaBgColorButton = new KColorButton(this);
    mButtonsDisable.push_back(mAgendaBgColorButton);
    colorLayout->addWidget(new QLabel(KOPrefs::instance()->agendaGridBackgroundColorItem()->label(), this), 2, 0);
    connect(mAgendaBgColorButton, &KColorButton::changed, this, &KOPrefsDialogColorsAndFonts::slotConfigChanged);
    colorLayout->addWidget(mAgendaBgColorButton, 2, 1);

    mViewBgBusyColorButton = new KColorButton(this);
    connect(mViewBgBusyColorButton, &KColorButton::changed, this, &KOPrefsDialogColorsAndFonts::slotConfigChanged);
    mButtonsDisable.push_back(mViewBgBusyColorButton);
    colorLayout->addWidget(new QLabel(KOPrefs::instance()->viewBgBusyColorItem()->label(), this), 3, 0);
    colorLayout->addWidget(mViewBgBusyColorButton, 3, 1);

    // working hours color
    mAgendaGridWorkHoursBackgroundColorButton = new KColorButton(this);
    connect(mAgendaGridWorkHoursBackgroundColorButton, &KColorButton::changed, this, &KOPrefsDialogColorsAndFonts::slotConfigChanged);
    mButtonsDisable.push_back(mAgendaGridWorkHoursBackgroundColorButton);
    colorLayout->addWidget(new QLabel(KOPrefs::instance()->workingHoursColorItem()->label(), this), 4, 0);
    colorLayout->addWidget(mAgendaGridWorkHoursBackgroundColorButton, 4, 1);

    // agenda view Marcus Bains line color
    mAgendaMarcusBainsLineLineColorButton = new KColorButton(this);
    connect(mAgendaMarcusBainsLineLineColorButton, &KColorButton::changed, this, &KOPrefsDialogColorsAndFonts::slotConfigChanged);
    colorLayout->addWidget(new QLabel(KOPrefs::instance()->agendaMarcusBainsLineLineColorItem()->label(), this), 5, 0);
    colorLayout->addWidget(mAgendaMarcusBainsLineLineColorButton, 5, 1);

    // Holiday Color
    mAgendaHolidaysBackgroundColorButton = new KColorButton(this);
    connect(mAgendaHolidaysBackgroundColorButton, &KColorButton::changed, this, &KOPrefsDialogColorsAndFonts::slotConfigChanged);
    colorLayout->addWidget(new QLabel(KOPrefs::instance()->agendaHolidaysBackgroundColorItem()->label(), this), 6, 0);
    colorLayout->addWidget(mAgendaHolidaysBackgroundColorButton, 6, 1);

    // Todo due today color
    mTodoDueTodayColorButton = new KColorButton(this);
    connect(mTodoDueTodayColorButton, &KColorButton::changed, this, &KOPrefsDialogColorsAndFonts::slotConfigChanged);
    colorLayout->addWidget(new QLabel(KOPrefs::instance()->todoDueTodayColorItem()->label(), this), 7, 0);
    colorLayout->addWidget(mTodoDueTodayColorButton, 7, 1);

    // Todo overdue color
    mTodoOverdueColorButton = new KColorButton(this);
    connect(mTodoOverdueColorButton, &KColorButton::changed, this, &KOPrefsDialogColorsAndFonts::slotConfigChanged);
    colorLayout->addWidget(new QLabel(KOPrefs::instance()->todoOverdueColorItem()->label(), this), 8, 0);
    colorLayout->addWidget(mTodoOverdueColorButton, 8, 1);

    // categories colors
    QGroupBox *categoryGroup = new QGroupBox(i18nc("@title:group", "Categories"), colorFrame);
    colorLayout->addWidget(categoryGroup, 9, 0, 1, 2);

    QGridLayout *categoryLayout = new QGridLayout;
    categoryGroup->setLayout(categoryLayout);

    mUnsetCategoryColorButton = new KColorButton(this);
    connect(mUnsetCategoryColorButton, &KColorButton::changed, this, &KOPrefsDialogColorsAndFonts::slotConfigChanged);
    categoryLayout->addWidget(new QLabel(CalendarSupport::KCalPrefs::instance()->unsetCategoryColorItem()->label(), this), 0, 0);
    categoryLayout->addWidget(mUnsetCategoryColorButton, 0, 1);

    mCategoryCombo = new Akonadi::TagSelectionComboBox(categoryGroup);
    mCategoryCombo->setWhatsThis(
        i18nc("@info:whatsthis",
              "Select here the event category you want to modify. "
              "You can change the selected category color using "
              "the button below."));
    connect(mCategoryCombo, qOverload<int>(&KComboBox::activated),
            this, &KOPrefsDialogColorsAndFonts::updateCategoryColor);
    categoryLayout->addWidget(mCategoryCombo, 1, 0);

    mCategoryButton = new KColorButton(categoryGroup);
    mCategoryButton->setWhatsThis(
        i18nc("@info:whatsthis",
              "Choose here the color of the event category selected "
              "using the combo box above."));
    connect(mCategoryButton, &KColorButton::changed, this,
            &KOPrefsDialogColorsAndFonts::setCategoryColor);
    categoryLayout->addWidget(mCategoryButton, 1, 1);

    updateCategoryColor();

    // resources colors
    QGroupBox *resourceGroup = new QGroupBox(i18nc("@title:group", "Resources"), colorFrame);
    colorLayout->addWidget(resourceGroup, 10, 0, 1, 2);

    QBoxLayout *resourceLayout = new QHBoxLayout;
    resourceGroup->setLayout(resourceLayout);

    mResourceCombo = new Akonadi::CollectionComboBox(resourceGroup);
    //mResourceCombo->addExcludedSpecialResources(Akonadi::Collection::SearchResource);
    QStringList mimetypes;
    mimetypes << KCalendarCore::Todo::todoMimeType();
    mimetypes << KCalendarCore::Journal::journalMimeType();
    mimetypes << KCalendarCore::Event::eventMimeType();

    mResourceCombo->setMimeTypeFilter(mimetypes);
    mResourceCombo->setWhatsThis(
        i18nc("@info:whatsthis",
              "Select the calendar you want to modify. "
              "You can change the selected calendar color using "
              "the button below."));
    connect(mResourceCombo, qOverload<int>(&Akonadi::CollectionComboBox::activated),
            this, &KOPrefsDialogColorsAndFonts::updateResourceColor);
    resourceLayout->addWidget(mResourceCombo);

    mResourceButton = new KColorButton(resourceGroup);
    mResourceButton->setWhatsThis(
        i18nc("@info:whatsthis",
              "Choose here the color of the calendar selected "
              "using the combo box above."));
    connect(mResourceButton, &KColorButton::changed, this,
            &KOPrefsDialogColorsAndFonts::setResourceColor);
    resourceLayout->addWidget(mResourceButton);

    colorLayout->setRowStretch(11, 1);

    QWidget *fontFrame = new QWidget(this);
    tabWidget->addTab(fontFrame, QIcon::fromTheme(QStringLiteral("preferences-desktop-font")),
                      i18nc("@title:tab", "Fonts"));

    QVBoxLayout *fontLayout = new QVBoxLayout(fontFrame);

    mTimeBarFontButton = new FontPreviewButton(KOPrefs::instance()->agendaTimeLabelsFontItem()->label(), this);
    mTimeBarFontButton->setPreviewText(QLocale().toString(QTime(12, 34), QLocale::ShortFormat));
    fontLayout->addWidget(mTimeBarFontButton);
    connect(mTimeBarFontButton, &FontPreviewButton::changed, this, &KOPrefsDialogColorsAndFonts::slotConfigChanged);

    mMonthViewFont = new FontPreviewButton(KOPrefs::instance()->monthViewFontItem()->label(), this);
    mMonthViewFont->setPreviewText(QLocale().toString(QTime(12, 34), QLocale::ShortFormat) + QLatin1Char(' ')
                                   +i18nc("@label", "Event text"));
    fontLayout->addWidget(mMonthViewFont);
    connect(mMonthViewFont, &FontPreviewButton::changed, this, &KOPrefsDialogColorsAndFonts::slotConfigChanged);

    mAgendaViewFont = new FontPreviewButton(KOPrefs::instance()->agendaViewFontItem()->label(), this);
    mAgendaViewFont->setPreviewText(i18nc("@label", "Event text"));
    fontLayout->addWidget(mAgendaViewFont);
    connect(mAgendaViewFont, &FontPreviewButton::changed, this, &KOPrefsDialogColorsAndFonts::slotConfigChanged);

    mMarcusBainsFont = new FontPreviewButton(KOPrefs::instance()->agendaMarcusBainsLineFontItem()->label(), this);
    mMarcusBainsFont->setPreviewText(QLocale().toString(QTime(12, 34, 23), QLocale::ShortFormat));
    fontLayout->addWidget(mMarcusBainsFont);
    connect(mMarcusBainsFont, &FontPreviewButton::changed, this, &KOPrefsDialogColorsAndFonts::slotConfigChanged);

    fontLayout->addStretch(1);
    load();
}

void KOPrefsDialogColorsAndFonts::save()
{
    QHash<QString, QColor>::const_iterator i = mCategoryDict.constBegin();
    while (i != mCategoryDict.constEnd()) {
        CalendarSupport::KCalPrefs::instance()->setCategoryColor(i.key(), i.value());
        ++i;
    }

    i = mResourceDict.constBegin();
    while (i != mResourceDict.constEnd()) {
        KOPrefs::instance()->setResourceColor(i.key(), i.value());
        ++i;
    }

    KOPrefs::instance()->setUseSystemColor(mUseSystemColorCheckBox->isChecked());

    KOPrefs::instance()->setAgendaGridBackgroundColor(mAgendaBgColorButton->color());
    KOPrefs::instance()->setViewBgBusyColor(mViewBgBusyColorButton->color());
    KOPrefs::instance()->setWorkingHoursColor(mAgendaGridWorkHoursBackgroundColorButton->color());
    KOPrefs::instance()->setAgendaMarcusBainsLineLineColor(mAgendaMarcusBainsLineLineColorButton->color());
    KOPrefs::instance()->setAgendaHolidaysBackgroundColor(mAgendaHolidaysBackgroundColorButton->color());
    KOPrefs::instance()->setTodoDueTodayColor(mTodoDueTodayColorButton->color());
    KOPrefs::instance()->setTodoOverdueColor(mTodoOverdueColorButton->color());
    CalendarSupport::KCalPrefs::instance()->setUnsetCategoryColor(mUnsetCategoryColorButton->color());
    KOPrefs::instance()->setAgendaTimeLabelsFont(mTimeBarFontButton->font());
    KOPrefs::instance()->setMonthViewFont(mMonthViewFont->font());
    KOPrefs::instance()->setAgendaViewFont(mAgendaViewFont->font());
    KOPrefs::instance()->setAgendaMarcusBainsLineFont(mMarcusBainsFont->font());
}

void KOPrefsDialogColorsAndFonts::load()
{
    updateCategories();
    updateResources();
    mUseSystemColorCheckBox->setChecked(KOPrefs::instance()->useSystemColor());
    mAgendaBgColorButton->setColor(KOPrefs::instance()->agendaGridBackgroundColor());
    mViewBgBusyColorButton->setColor(KOPrefs::instance()->viewBgBusyColor());
    mAgendaGridWorkHoursBackgroundColorButton->setColor(KOPrefs::instance()->workingHoursColor());
    mAgendaMarcusBainsLineLineColorButton->setColor(KOPrefs::instance()->agendaMarcusBainsLineLineColor());
    mAgendaHolidaysBackgroundColorButton->setColor(KOPrefs::instance()->agendaHolidaysBackgroundColor());
    mTodoDueTodayColorButton->setColor(KOPrefs::instance()->todoDueTodayColor());
    mTodoOverdueColorButton->setColor(KOPrefs::instance()->todoOverdueColor());
    mUnsetCategoryColorButton->setColor(CalendarSupport::KCalPrefs::instance()->unsetCategoryColor());
    mTimeBarFontButton->setFont(KOPrefs::instance()->agendaTimeLabelsFont());
    mMonthViewFont->setFont(KOPrefs::instance()->monthViewFont());
    mAgendaViewFont->setFont(KOPrefs::instance()->agendaViewFont());
    mMarcusBainsFont->setFont(KOPrefs::instance()->agendaMarcusBainsLineFont());
}

void KOPrefsDialogColorsAndFonts::useSystemColorToggle(bool useSystemColor)
{
    for (KColorButton *colorButton : qAsConst(mButtonsDisable)) {
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
    slotConfigChanged();
}

void KOPrefsDialogColorsAndFonts::updateCategoryColor()
{
    const QString cat = mCategoryCombo->currentText();
    QColor color = mCategoryDict.value(cat);
    if (!color.isValid()) {
        //TODO get this from the tag
        color = CalendarSupport::KCalPrefs::instance()->categoryColor(cat);
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
    const QString id
        = QString::number(mResourceCombo->itemData(
                              mResourceCombo->currentIndex(),
                              Akonadi::EntityTreeModel::CollectionIdRole).toLongLong(&ok));
    if (!ok) {
        return;
    }
    mResourceDict.insert(id, mResourceButton->color());
    slotConfigChanged();
}

void KOPrefsDialogColorsAndFonts::updateResourceColor()
{
    bool ok;
    const QString id
        = QString::number(mResourceCombo->itemData(
                              mResourceCombo->currentIndex(),
                              Akonadi::EntityTreeModel::CollectionIdRole).toLongLong(&ok));
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

void KOPrefsDialogColorsAndFonts::slotConfigChanged()
{
    Q_EMIT markAsChanged();
}

extern "C"
{
Q_DECL_EXPORT KCModule *create_korganizerconfigcolorsandfonts(QWidget *parent, const char *)
{
    return new KOPrefsDialogColorsAndFonts(parent);
}
}
