/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2001,2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  SPDX-FileCopyrightText: 2005,2008,2011 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/
// krazy:excludeall=tipsandthis

#include "kprefsdialog.h"

#include <KColorButton>
#include <KComboBox>
#include <KDateComboBox>
#include <KLocalizedString>
#include <KMessageBox>
#include <KTimeComboBox>
#include <KUrlRequester>
#include <QDebug>
#include <QFontDialog>
#include <QUrl>

#include <QButtonGroup>
#include <QCheckBox>
#include <QFont>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QTimeEdit>

using namespace Korganizer;

namespace KPrefsWidFactory
{
KPrefsWid *create(KConfigSkeletonItem *item, QWidget *parent)
{
    auto boolItem = dynamic_cast<KConfigSkeleton::ItemBool *>(item);
    if (boolItem) {
        return new KPrefsWidBool(boolItem, parent);
    }

    auto stringItem = dynamic_cast<KConfigSkeleton::ItemString *>(item);
    if (stringItem) {
        return new KPrefsWidString(stringItem, parent);
    }

    auto enumItem = dynamic_cast<KConfigSkeleton::ItemEnum *>(item);
    if (enumItem) {
        QList<KConfigSkeleton::ItemEnum::Choice> choices = enumItem->choices();
        if (choices.isEmpty()) {
            qCritical() << "Enum has no choices.";
            return nullptr;
        } else {
            auto radios = new KPrefsWidRadios(enumItem, parent);
            QList<KConfigSkeleton::ItemEnum::Choice>::ConstIterator it;
            int value = 0;
            QList<KConfigSkeleton::ItemEnum::Choice>::ConstIterator end(choices.constEnd());
            for (it = choices.constBegin(); it != end; ++it) {
                radios->addRadio(value++, (*it).label);
            }
            return radios;
        }
    }

    auto intItem = dynamic_cast<KConfigSkeleton::ItemInt *>(item);
    if (intItem) {
        return new KPrefsWidInt(intItem, parent);
    }

    return nullptr;
}
} // namespace KPrefsWidFactory

QList<QWidget *> KPrefsWid::widgets() const
{
    return QList<QWidget *>();
}

KPrefsWidBool::KPrefsWidBool(KConfigSkeleton::ItemBool *item, QWidget *parent)
    : mItem(item)
{
    mCheck = new QCheckBox(mItem->label(), parent);
    connect(mCheck, &QCheckBox::clicked, this, &KPrefsWidBool::changed);
    QString toolTip = mItem->toolTip();
    if (!toolTip.isEmpty()) {
        mCheck->setToolTip(toolTip);
    }
    QString whatsThis = mItem->whatsThis();
    if (!whatsThis.isEmpty()) {
        mCheck->setWhatsThis(whatsThis);
    }
}

void KPrefsWidBool::readConfig()
{
    mCheck->setChecked(mItem->value());
}

void KPrefsWidBool::writeConfig()
{
    mItem->setValue(mCheck->isChecked());
}

QCheckBox *KPrefsWidBool::checkBox()
{
    return mCheck;
}

QList<QWidget *> KPrefsWidBool::widgets() const
{
    QList<QWidget *> widgets;
    widgets.append(mCheck);
    return widgets;
}

KPrefsWidInt::KPrefsWidInt(KConfigSkeleton::ItemInt *item, QWidget *parent)
    : mItem(item)
{
    mLabel = new QLabel(mItem->label() + QLatin1Char(':'), parent);
    mSpin = new QSpinBox(parent);
    if (!mItem->minValue().isNull()) {
        mSpin->setMinimum(mItem->minValue().toInt());
    }
    if (!mItem->maxValue().isNull()) {
        mSpin->setMaximum(mItem->maxValue().toInt());
    }
    connect(mSpin, qOverload<int>(&QSpinBox::valueChanged), this, &KPrefsWidInt::changed);
    mLabel->setBuddy(mSpin);
    QString toolTip = mItem->toolTip();
    if (!toolTip.isEmpty()) {
        mLabel->setToolTip(toolTip);
        mSpin->setToolTip(toolTip);
    }
    QString whatsThis = mItem->whatsThis();
    if (!whatsThis.isEmpty()) {
        mLabel->setWhatsThis(whatsThis);
        mSpin->setWhatsThis(whatsThis);
    }
}

void KPrefsWidInt::readConfig()
{
    mSpin->setValue(mItem->value());
}

void KPrefsWidInt::writeConfig()
{
    mItem->setValue(mSpin->value());
}

QLabel *KPrefsWidInt::label() const
{
    return mLabel;
}

QSpinBox *KPrefsWidInt::spinBox()
{
    return mSpin;
}

QList<QWidget *> KPrefsWidInt::widgets() const
{
    QList<QWidget *> widgets;
    widgets.append(mLabel);
    widgets.append(mSpin);
    return widgets;
}

KPrefsWidColor::KPrefsWidColor(KConfigSkeleton::ItemColor *item, QWidget *parent)
    : mItem(item)
{
    mButton = new KColorButton(parent);
    connect(mButton, &KColorButton::changed, this, &KPrefsWidColor::changed);
    mLabel = new QLabel(mItem->label() + QLatin1Char(':'), parent);
    mLabel->setBuddy(mButton);
    QString toolTip = mItem->toolTip();
    if (!toolTip.isEmpty()) {
        mButton->setToolTip(toolTip);
    }
    QString whatsThis = mItem->whatsThis();
    if (!whatsThis.isEmpty()) {
        mButton->setWhatsThis(whatsThis);
    }
}

KPrefsWidColor::~KPrefsWidColor()
{
}

void KPrefsWidColor::readConfig()
{
    mButton->setColor(mItem->value());
}

void KPrefsWidColor::writeConfig()
{
    mItem->setValue(mButton->color());
}

QLabel *KPrefsWidColor::label()
{
    return mLabel;
}

KColorButton *KPrefsWidColor::button()
{
    return mButton;
}

KPrefsWidFont::KPrefsWidFont(KConfigSkeleton::ItemFont *item, QWidget *parent, const QString &sampleText)
    : mItem(item)
{
    mLabel = new QLabel(mItem->label() + QLatin1Char(':'), parent);

    mPreview = new QLabel(sampleText, parent);
    mPreview->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);

    mButton = new QPushButton(i18n("Choose..."), parent);
    connect(mButton, &QPushButton::clicked, this, &Korganizer::KPrefsWidFont::selectFont);
    QString toolTip = mItem->toolTip();
    if (!toolTip.isEmpty()) {
        mPreview->setToolTip(toolTip);
        mButton->setToolTip(toolTip);
    }
    QString whatsThis = mItem->whatsThis();
    if (!whatsThis.isEmpty()) {
        mPreview->setWhatsThis(whatsThis);
        mButton->setWhatsThis(whatsThis);
    }
}

KPrefsWidFont::~KPrefsWidFont()
{
}

void KPrefsWidFont::readConfig()
{
    mPreview->setFont(mItem->value());
}

void KPrefsWidFont::writeConfig()
{
    mItem->setValue(mPreview->font());
}

QLabel *KPrefsWidFont::label()
{
    return mLabel;
}

QFrame *KPrefsWidFont::preview()
{
    return mPreview;
}

QPushButton *KPrefsWidFont::button()
{
    return mButton;
}

void KPrefsWidFont::selectFont()
{
#ifndef QT_NO_FONTDIALOG
    bool ok;
    QFont myFont = QFontDialog::getFont(&ok, mPreview->font());
    if (ok) {
        mPreview->setFont(myFont);
        Q_EMIT changed();
    }
#endif
}

KPrefsWidTime::KPrefsWidTime(KConfigSkeleton::ItemDateTime *item, QWidget *parent)
    : mItem(item)
{
    mLabel = new QLabel(mItem->label() + QLatin1Char(':'), parent);
    mTimeEdit = new KTimeComboBox(parent);
    mLabel->setBuddy(mTimeEdit);
    connect(mTimeEdit, &KTimeComboBox::timeEdited, this, &KPrefsWidTime::changed);
    connect(mTimeEdit, &KTimeComboBox::timeEntered, this, &KPrefsWidTime::changed);
    QString toolTip = mItem->toolTip();
    if (!toolTip.isEmpty()) {
        mTimeEdit->setToolTip(toolTip);
    }
    QString whatsThis = mItem->whatsThis();
    if (!whatsThis.isEmpty()) {
        mTimeEdit->setWhatsThis(whatsThis);
    }
}

void KPrefsWidTime::readConfig()
{
    mTimeEdit->setTime(mItem->value().time());
}

void KPrefsWidTime::writeConfig()
{
    // Don't overwrite the date value of the QDateTime, so we can use a
    // KPrefsWidTime and a KPrefsWidDate on the same config entry!
    QDateTime dt(mItem->value());
    dt.setTime(mTimeEdit->time());
    mItem->setValue(dt);
}

QLabel *KPrefsWidTime::label()
{
    return mLabel;
}

KTimeComboBox *KPrefsWidTime::timeEdit()
{
    return mTimeEdit;
}

KPrefsWidDuration::KPrefsWidDuration(KConfigSkeleton::ItemDateTime *item, const QString &format, QWidget *parent)
    : mItem(item)
{
    mLabel = new QLabel(mItem->label() + QLatin1Char(':'), parent);
    mTimeEdit = new QTimeEdit(parent);
    mLabel->setBuddy(mTimeEdit);
    if (format.isEmpty()) {
        mTimeEdit->setDisplayFormat(QStringLiteral("hh:mm:ss"));
    } else {
        mTimeEdit->setDisplayFormat(format);
    }
    mTimeEdit->setMinimumTime(QTime(0, 1)); // [1 min]
    mTimeEdit->setMaximumTime(QTime(24, 0)); // [24 hr]
    connect(mTimeEdit, &QTimeEdit::timeChanged, this, &KPrefsWidDuration::changed);
    QString toolTip = mItem->toolTip();
    if (!toolTip.isEmpty()) {
        mTimeEdit->setToolTip(toolTip);
    }
    QString whatsThis = mItem->whatsThis();
    if (!whatsThis.isEmpty()) {
        mTimeEdit->setWhatsThis(whatsThis);
    }
}

void KPrefsWidDuration::readConfig()
{
    mTimeEdit->setTime(mItem->value().time());
}

void KPrefsWidDuration::writeConfig()
{
    QDateTime dt(mItem->value());
    dt.setTime(mTimeEdit->time());
    mItem->setValue(dt);
}

QLabel *KPrefsWidDuration::label()
{
    return mLabel;
}

QTimeEdit *KPrefsWidDuration::timeEdit()
{
    return mTimeEdit;
}

KPrefsWidDate::KPrefsWidDate(KConfigSkeleton::ItemDateTime *item, QWidget *parent)
    : mItem(item)
{
    mLabel = new QLabel(mItem->label() + QLatin1Char(':'), parent);
    mDateEdit = new KDateComboBox(parent);
    mLabel->setBuddy(mDateEdit);
    connect(mDateEdit, &KDateComboBox::dateEdited, this, &KPrefsWidDate::changed);
    QString toolTip = mItem->toolTip();
    if (!toolTip.isEmpty()) {
        mDateEdit->setToolTip(toolTip);
    }
    QString whatsThis = mItem->whatsThis();
    if (!whatsThis.isEmpty()) {
        mDateEdit->setWhatsThis(whatsThis);
    }
}

void KPrefsWidDate::readConfig()
{
    if (!mItem->value().date().isValid()) {
        mItem->setValue(QDateTime::currentDateTime());
    }
    mDateEdit->setDate(mItem->value().date().isValid() ? mItem->value().date() : QDate::currentDate());
}

void KPrefsWidDate::writeConfig()
{
    QDateTime dt(mItem->value());
    dt.setDate(mDateEdit->date());
    mItem->setValue(dt);
    if (!mItem->value().date().isValid()) {
        mItem->setValue(QDateTime::currentDateTime());
    }
}

QLabel *KPrefsWidDate::label()
{
    return mLabel;
}

KDateComboBox *KPrefsWidDate::dateEdit()
{
    return mDateEdit;
}

KPrefsWidRadios::KPrefsWidRadios(KConfigSkeleton::ItemEnum *item, QWidget *parent)
    : mItem(item)
{
    mBox = new QGroupBox(mItem->label(), parent);
    new QVBoxLayout(mBox);
    mGroup = new QButtonGroup(parent);
    connect(mGroup, qOverload<QAbstractButton *>(&QButtonGroup::buttonClicked), this, &KPrefsWidRadios::changed);
}

KPrefsWidRadios::~KPrefsWidRadios()
{
}

void KPrefsWidRadios::addRadio(int value, const QString &text, const QString &toolTip, const QString &whatsThis)
{
    auto r = new QRadioButton(text, mBox);
    mBox->layout()->addWidget(r);
    mGroup->addButton(r, value);
    if (!toolTip.isEmpty()) {
        r->setToolTip(toolTip);
    }
    if (!whatsThis.isEmpty()) {
        r->setWhatsThis(whatsThis);
    }
}

QGroupBox *KPrefsWidRadios::groupBox() const
{
    return mBox;
}

void KPrefsWidRadios::readConfig()
{
    if (!mGroup->button(mItem->value())) {
        return;
    }
    mGroup->button(mItem->value())->setChecked(true);
}

void KPrefsWidRadios::writeConfig()
{
    mItem->setValue(mGroup->checkedId());
}

QList<QWidget *> KPrefsWidRadios::widgets() const
{
    QList<QWidget *> w;
    w.append(mBox);
    return w;
}

KPrefsWidCombo::KPrefsWidCombo(KConfigSkeleton::ItemEnum *item, QWidget *parent)
    : mItem(item)
{
    mLabel = new QLabel(mItem->label(), parent);
    mCombo = new KComboBox(parent);
    connect(mCombo, &KComboBox::activated, this, &KPrefsWidCombo::changed);
    mLabel->setBuddy(mCombo);
    QString toolTip = mItem->toolTip();
    if (!toolTip.isEmpty()) {
        mLabel->setToolTip(toolTip);
        mCombo->setToolTip(toolTip);
    }
    QString whatsThis = mItem->whatsThis();
    if (!whatsThis.isEmpty()) {
        mLabel->setWhatsThis(whatsThis);
        mCombo->setWhatsThis(whatsThis);
    }
}

KPrefsWidCombo::~KPrefsWidCombo()
{
}

void KPrefsWidCombo::readConfig()
{
    mCombo->setCurrentIndex(mItem->value());
}

void KPrefsWidCombo::writeConfig()
{
    mItem->setValue(mCombo->currentIndex());
}

QList<QWidget *> KPrefsWidCombo::widgets() const
{
    QList<QWidget *> w;
    w.append(mCombo);
    return w;
}

KComboBox *KPrefsWidCombo::comboBox()
{
    return mCombo;
}

QLabel *KPrefsWidCombo::label() const
{
    return mLabel;
}

KPrefsWidString::KPrefsWidString(KConfigSkeleton::ItemString *item, QWidget *parent, KLineEdit::EchoMode echomode)
    : mItem(item)
{
    mLabel = new QLabel(mItem->label() + QLatin1Char(':'), parent);
    mEdit = new KLineEdit(parent);
    mLabel->setBuddy(mEdit);
    connect(mEdit, &KLineEdit::textChanged, this, &KPrefsWidString::changed);
    mEdit->setEchoMode(echomode);
    QString toolTip = mItem->toolTip();
    if (!toolTip.isEmpty()) {
        mEdit->setToolTip(toolTip);
    }
    QString whatsThis = mItem->whatsThis();
    if (!whatsThis.isEmpty()) {
        mEdit->setWhatsThis(whatsThis);
    }
}

KPrefsWidString::~KPrefsWidString()
{
}

void KPrefsWidString::readConfig()
{
    mEdit->setText(mItem->value());
}

void KPrefsWidString::writeConfig()
{
    mItem->setValue(mEdit->text());
}

QLabel *KPrefsWidString::label()
{
    return mLabel;
}

KLineEdit *KPrefsWidString::lineEdit()
{
    return mEdit;
}

QList<QWidget *> KPrefsWidString::widgets() const
{
    QList<QWidget *> widgets;
    widgets.append(mLabel);
    widgets.append(mEdit);
    return widgets;
}

KPrefsWidPath::KPrefsWidPath(KConfigSkeleton::ItemPath *item, QWidget *parent, const QString &filter, KFile::Modes mode)
    : mItem(item)
{
    mLabel = new QLabel(mItem->label() + QLatin1Char(':'), parent);
    mURLRequester = new KUrlRequester(parent);
    mLabel->setBuddy(mURLRequester);
    mURLRequester->setMode(mode);
    mURLRequester->setFilter(filter);
    connect(mURLRequester, &KUrlRequester::textChanged, this, &KPrefsWidPath::changed);
    QString toolTip = mItem->toolTip();
    if (!toolTip.isEmpty()) {
        mURLRequester->setToolTip(toolTip);
    }
    QString whatsThis = mItem->whatsThis();
    if (!whatsThis.isEmpty()) {
        mURLRequester->setWhatsThis(whatsThis);
    }
}

KPrefsWidPath::~KPrefsWidPath()
{
}

void KPrefsWidPath::readConfig()
{
    mURLRequester->setUrl(QUrl(mItem->value()));
}

void KPrefsWidPath::writeConfig()
{
    mItem->setValue(mURLRequester->url().path());
}

QLabel *KPrefsWidPath::label()
{
    return mLabel;
}

KUrlRequester *KPrefsWidPath::urlRequester()
{
    return mURLRequester;
}

QList<QWidget *> KPrefsWidPath::widgets() const
{
    QList<QWidget *> widgets;
    widgets.append(mLabel);
    widgets.append(mURLRequester);
    return widgets;
}

KPrefsWidManager::KPrefsWidManager(KConfigSkeleton *prefs)
    : mPrefs(prefs)
{
}

KPrefsWidManager::~KPrefsWidManager()
{
    qDeleteAll(mPrefsWids);
    mPrefsWids.clear();
}

void KPrefsWidManager::addWid(KPrefsWid *wid)
{
    mPrefsWids.append(wid);
}

KPrefsWidBool *KPrefsWidManager::addWidBool(KConfigSkeleton::ItemBool *item, QWidget *parent)
{
    auto w = new KPrefsWidBool(item, parent);
    addWid(w);
    return w;
}

KPrefsWidTime *KPrefsWidManager::addWidTime(KConfigSkeleton::ItemDateTime *item, QWidget *parent)
{
    auto w = new KPrefsWidTime(item, parent);
    addWid(w);
    return w;
}

KPrefsWidDuration *KPrefsWidManager::addWidDuration(KConfigSkeleton::ItemDateTime *item, const QString &format, QWidget *parent)
{
    auto w = new KPrefsWidDuration(item, format, parent);
    addWid(w);
    return w;
}

KPrefsWidDate *KPrefsWidManager::addWidDate(KConfigSkeleton::ItemDateTime *item, QWidget *parent)
{
    auto w = new KPrefsWidDate(item, parent);
    addWid(w);
    return w;
}

KPrefsWidColor *KPrefsWidManager::addWidColor(KConfigSkeleton::ItemColor *item, QWidget *parent)
{
    auto w = new KPrefsWidColor(item, parent);
    addWid(w);
    return w;
}

KPrefsWidRadios *KPrefsWidManager::addWidRadios(KConfigSkeleton::ItemEnum *item, QWidget *parent)
{
    auto w = new KPrefsWidRadios(item, parent);
    QList<KConfigSkeleton::ItemEnum::Choice2> choices;
    choices = item->choices2();
    QList<KConfigSkeleton::ItemEnum::Choice2>::ConstIterator it;
    QList<KConfigSkeleton::ItemEnum::Choice2>::ConstIterator end(choices.constEnd());
    int value = 0;
    for (it = choices.constBegin(); it != end; ++it) {
        w->addRadio(value++, (*it).label, (*it).toolTip, (*it).whatsThis);
    }
    addWid(w);
    return w;
}

KPrefsWidCombo *KPrefsWidManager::addWidCombo(KConfigSkeleton::ItemEnum *item, QWidget *parent)
{
    auto w = new KPrefsWidCombo(item, parent);
    QList<KConfigSkeleton::ItemEnum::Choice> choices;
    choices = item->choices();
    QList<KConfigSkeleton::ItemEnum::Choice>::ConstIterator end(choices.constEnd());
    for (QList<KConfigSkeleton::ItemEnum::Choice>::ConstIterator it = choices.constBegin(); it != end; ++it) {
        w->comboBox()->addItem((*it).label);
    }
    addWid(w);
    return w;
}

KPrefsWidString *KPrefsWidManager::addWidString(KConfigSkeleton::ItemString *item, QWidget *parent)
{
    auto w = new KPrefsWidString(item, parent, KLineEdit::Normal);
    addWid(w);
    return w;
}

KPrefsWidPath *KPrefsWidManager::addWidPath(KConfigSkeleton::ItemPath *item, QWidget *parent, const QString &filter, KFile::Modes mode)
{
    auto w = new KPrefsWidPath(item, parent, filter, mode);
    addWid(w);
    return w;
}

KPrefsWidString *KPrefsWidManager::addWidPassword(KConfigSkeleton::ItemString *item, QWidget *parent)
{
    auto w = new KPrefsWidString(item, parent, KLineEdit::Password);
    addWid(w);
    return w;
}

KPrefsWidFont *KPrefsWidManager::addWidFont(KConfigSkeleton::ItemFont *item, QWidget *parent, const QString &sampleText)
{
    auto w = new KPrefsWidFont(item, parent, sampleText);
    addWid(w);
    return w;
}

KPrefsWidInt *KPrefsWidManager::addWidInt(KConfigSkeleton::ItemInt *item, QWidget *parent)
{
    auto w = new KPrefsWidInt(item, parent);
    addWid(w);
    return w;
}

void KPrefsWidManager::setWidDefaults()
{
    bool tmp = mPrefs->useDefaults(true);
    readWidConfig();
    mPrefs->useDefaults(tmp);
}

void KPrefsWidManager::readWidConfig()
{
    QList<KPrefsWid *>::Iterator it;
    for (it = mPrefsWids.begin(); it != mPrefsWids.end(); ++it) {
        (*it)->readConfig();
    }
}

void KPrefsWidManager::writeWidConfig()
{
    QList<KPrefsWid *>::Iterator it;
    for (it = mPrefsWids.begin(); it != mPrefsWids.end(); ++it) {
        (*it)->writeConfig();
    }

    mPrefs->save();
}

KPrefsDialog::KPrefsDialog(KConfigSkeleton *prefs, QWidget *parent, bool modal)
    : KPageDialog(parent)
    , KPrefsWidManager(prefs)
{
    setFaceType(List);
    setWindowTitle(i18nc("@title:window", "Preferences"));
    setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Apply | QDialogButtonBox::Cancel | QDialogButtonBox::RestoreDefaults);
    button(QDialogButtonBox::Ok)->setDefault(true);
    setModal(modal);
    connect(button(QDialogButtonBox::Ok), &QPushButton::clicked, this, &KPrefsDialog::slotOk);
    connect(button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &KPrefsDialog::slotApply);
    connect(button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked, this, &KPrefsDialog::slotDefault);
    connect(button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &KPrefsDialog::reject);
}

KPrefsDialog::~KPrefsDialog()
{
}

void KPrefsDialog::autoCreate()
{
    KConfigSkeletonItem::List items = prefs()->items();

    QMap<QString, QWidget *> mGroupPages;
    QMap<QString, QGridLayout *> mGroupLayouts;
    QMap<QString, int> mCurrentRows;

    KConfigSkeletonItem::List::ConstIterator it;
    for (it = items.constBegin(); it != items.constEnd(); ++it) {
        QString group = (*it)->group();

        QWidget *page = nullptr;
        QGridLayout *layout = nullptr;
        int currentRow;
        if (!mGroupPages.contains(group)) {
            page = new QWidget(this);
            addPage(page, group);
            layout = new QGridLayout(page);
            mGroupPages.insert(group, page);
            mGroupLayouts.insert(group, layout);
            currentRow = 0;
            mCurrentRows.insert(group, currentRow);
        } else {
            page = mGroupPages[group];
            layout = mGroupLayouts[group];
            currentRow = mCurrentRows[group];
        }

        KPrefsWid *wid = KPrefsWidFactory::create(*it, page);

        if (wid) {
            QList<QWidget *> widgets = wid->widgets();
            if (widgets.count() == 1) {
                layout->addWidget(widgets[0], currentRow, currentRow, 0, 1);
            } else if (widgets.count() == 2) {
                layout->addWidget(widgets[0], currentRow, 0);
                layout->addWidget(widgets[1], currentRow, 1);
            } else {
                qCritical() << "More widgets than expected:" << widgets.count();
            }

            if ((*it)->isImmutable()) {
                QList<QWidget *>::Iterator it2;
                for (it2 = widgets.begin(); it2 != widgets.end(); ++it2) {
                    (*it2)->setEnabled(false);
                }
            }
            addWid(wid);
            mCurrentRows.insert(group, ++currentRow);
        }
    }

    readConfig();
}

void KPrefsDialog::setDefaults()
{
    setWidDefaults();
}

void KPrefsDialog::readConfig()
{
    readWidConfig();
    usrReadConfig();
}

void KPrefsDialog::writeConfig()
{
    writeWidConfig();
    usrWriteConfig();
    readConfig();
}

void KPrefsDialog::slotApply()
{
    writeConfig();

    Q_EMIT configChanged();
}

void KPrefsDialog::slotOk()
{
    slotApply();
    accept();
}

void KPrefsDialog::slotDefault()
{
    if (KMessageBox::warningContinueCancel(this,
                                           i18n("You are about to set all preferences to default values. "
                                                "All custom modifications will be lost."),
                                           i18n("Setting Default Preferences"),
                                           KGuiItem(i18n("Reset to Defaults")))
        == KMessageBox::Continue) {
        setDefaults();
    }
}

KPrefsModule::KPrefsModule(KConfigSkeleton *prefs, QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
    , KPrefsWidManager(prefs)
{
    Q_EMIT changed(false);
}

void KPrefsModule::addWid(KPrefsWid *wid)
{
    KPrefsWidManager::addWid(wid);
    connect(wid, &KPrefsWid::changed, this, &KPrefsModule::slotWidChanged);
}

void KPrefsModule::slotWidChanged()
{
    Q_EMIT changed(true);
}

void KPrefsModule::load()
{
    readWidConfig();
    usrReadConfig();

    Q_EMIT changed(false);
}

void KPrefsModule::save()
{
    writeWidConfig();
    usrWriteConfig();
}

void KPrefsModule::defaults()
{
    setWidDefaults();

    Q_EMIT changed(true);
}
