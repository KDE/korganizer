/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2004 Tobias Koenig <tokoe@kde.org>
  SPDX-FileCopyrightText: 2004 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kcmdesignerfields.h"

#include "korganizer_debug.h"
#include <KAboutData>
#include <KDialogJobUiDelegate>
#include <KDirWatch>
#include <KIO/CommandLauncherJob>
#include <KIO/DeleteJob>
#include <KIO/FileCopyJob>
#include <KIO/MkdirJob>
#include <KJobWidgets>
#include <KLocalizedString>
#include <KMessageBox>
#include <KShell>
#include <QFileDialog>

#include <QDir>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QStandardPaths>
#include <QTreeWidget>
#include <QUiLoader>
#include <QWhatsThis>

class PageItem : public QTreeWidgetItem
{
public:
    PageItem(QTreeWidget *parent, const QString &path)
        : QTreeWidgetItem(parent)
        , mPath(path)
    {
        setFlags(flags() | Qt::ItemIsUserCheckable);
        setCheckState(0, Qt::Unchecked);
        mName = path.mid(path.lastIndexOf(QLatin1Char('/')) + 1);

        QFile f(mPath);
        if (!f.open(QFile::ReadOnly)) {
            return;
        }
        QUiLoader builder;
        QWidget *wdg = builder.load(&f, nullptr);
        f.close();
        if (wdg) {
            setText(0, wdg->windowTitle());

            QPixmap pm = wdg->grab();
            const QImage img = pm.toImage().scaled(300, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            mPreview = QPixmap::fromImage(img);

            QMap<QString, QString> allowedTypes;
            allowedTypes.insert(QStringLiteral("QLineEdit"), i18n("Text"));
            allowedTypes.insert(QStringLiteral("QTextEdit"), i18n("Text"));
            allowedTypes.insert(QStringLiteral("QSpinBox"), i18n("Numeric Value"));
            allowedTypes.insert(QStringLiteral("QCheckBox"), i18n("Boolean"));
            allowedTypes.insert(QStringLiteral("QComboBox"), i18n("Selection"));
            allowedTypes.insert(QStringLiteral("QDateTimeEdit"), i18n("Date & Time"));
            allowedTypes.insert(QStringLiteral("KLineEdit"), i18n("Text"));
            allowedTypes.insert(QStringLiteral("KTextEdit"), i18n("Text"));
            allowedTypes.insert(QStringLiteral("KDateTimeWidget"), i18n("Date & Time"));
            allowedTypes.insert(QStringLiteral("KDatePicker"), i18n("Date"));

            const QList<QWidget *> list = wdg->findChildren<QWidget *>();
            for (QWidget *it : list) {
                if (allowedTypes.contains(QLatin1String(it->metaObject()->className()))) {
                    const QString objectName = it->objectName();
                    if (objectName.startsWith(QLatin1String("X_"))) {
                        new QTreeWidgetItem(this,
                                            QStringList() << objectName << allowedTypes[QLatin1String(it->metaObject()->className())]
                                                          << QLatin1String(it->metaObject()->className()) << it->whatsThis());
                    }
                }
            }
        }
    }

    Q_REQUIRED_RESULT QString name() const
    {
        return mName;
    }

    Q_REQUIRED_RESULT QString path() const
    {
        return mPath;
    }

    Q_REQUIRED_RESULT QPixmap preview() const
    {
        return mPreview;
    }

    void setIsActive(bool isActive)
    {
        mIsActive = isActive;
    }

    Q_REQUIRED_RESULT bool isActive() const
    {
        return mIsActive;
    }

    Q_REQUIRED_RESULT bool isOn() const
    {
        return checkState(0) == Qt::Checked;
    }

private:
    QString mName;
    const QString mPath;
    QPixmap mPreview;
    bool mIsActive = false;
};

KCMDesignerFields::KCMDesignerFields(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
{
    auto about = new KAboutData(QStringLiteral("KCMDesignerfields"),
                                i18n("KCMDesignerfields"),
                                QString(),
                                i18n("Qt Designer Fields Dialog"),
                                KAboutLicense::LGPL,
                                i18n("(c) 2004 Tobias Koenig"));
    about->addAuthor(ki18n("Tobias Koenig").toString(), QString(), QStringLiteral("tokoe@kde.org"));
    about->addAuthor(ki18n("Cornelius Schumacher").toString(), QString(), QStringLiteral("schumacher@kde.org"));
    setAboutData(about);
}

void KCMDesignerFields::delayedInit()
{
    qCDebug(KORGANIZER_LOG) << "KCMDesignerFields::delayedInit()";

    initGUI();

    connect(mPageView, &QTreeWidget::itemSelectionChanged, this, &KCMDesignerFields::updatePreview);
    connect(mPageView, &QTreeWidget::itemClicked, this, &KCMDesignerFields::itemClicked);

    connect(mDeleteButton, &QPushButton::clicked, this, &KCMDesignerFields::deleteFile);
    connect(mImportButton, &QPushButton::clicked, this, &KCMDesignerFields::importFile);
    connect(mDesignerButton, &QPushButton::clicked, this, &KCMDesignerFields::startDesigner);

    load();

    // Install a dirwatcher that will detect newly created or removed designer files
    auto dw = new KDirWatch(this);
    QDir().mkpath(localUiDir());
    dw->addDir(localUiDir(), KDirWatch::WatchFiles);
    connect(dw, &KDirWatch::created, this, &KCMDesignerFields::rebuildList);
    connect(dw, &KDirWatch::deleted, this, &KCMDesignerFields::rebuildList);
    connect(dw, &KDirWatch::dirty, this, &KCMDesignerFields::rebuildList);
}

void KCMDesignerFields::deleteFile()
{
    const auto selectedItems = mPageView->selectedItems();
    for (QTreeWidgetItem *item : selectedItems) {
        auto pageItem = static_cast<PageItem *>(item->parent() ? item->parent() : item);
        if (KMessageBox::warningContinueCancel(this,
                                               i18n("<qt>Do you really want to delete '<b>%1</b>'?</qt>", pageItem->text(0)),
                                               QString(),
                                               KStandardGuiItem::del())
            == KMessageBox::Continue) {
            QFile::remove(pageItem->path());
        }
    }
    // The actual view refresh will be done automagically by the slots connected to kdirwatch
}

void KCMDesignerFields::importFile()
{
    const QUrl src =
        QFileDialog::getOpenFileUrl(this, i18n("Import Page"), QUrl::fromLocalFile(QDir::homePath()), QStringLiteral("%1 (*.ui)").arg(i18n("Designer Files")));
    QUrl dest = QUrl::fromLocalFile(localUiDir());
    QDir().mkpath(localUiDir());
    dest = dest.adjusted(QUrl::RemoveFilename);
    dest.setPath(src.fileName());
    KIO::Job *job = KIO::file_copy(src, dest, -1, KIO::Overwrite);
    KJobWidgets::setWindow(job, this);
    job->exec();
    // The actual view refresh will be done automagically by the slots connected to kdirwatch
}

void KCMDesignerFields::loadUiFiles()
{
    const QStringList dirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, uiPath(), QStandardPaths::LocateDirectory);
    for (const QString &dir : dirs) {
        const QStringList fileNames = QDir(dir).entryList(QStringList() << QStringLiteral("*.ui"));
        for (const QString &file : fileNames) {
            new PageItem(mPageView, dir + QLatin1Char('/') + file);
        }
    }
}

void KCMDesignerFields::rebuildList()
{
    // If nothing is initialized there is no need to do something
    if (mPageView) {
        const QStringList ai = saveActivePages();
        updatePreview();
        mPageView->clear();
        loadUiFiles();
        loadActivePages(ai);
    }
}

void KCMDesignerFields::loadActivePages(const QStringList &ai)
{
    QTreeWidgetItemIterator it(mPageView);
    while (*it) {
        if ((*it)->parent() == nullptr) {
            auto item = static_cast<PageItem *>(*it);
            if (ai.contains(item->name())) {
                item->setCheckState(0, Qt::Checked);
                item->setIsActive(true);
            }
        }

        ++it;
    }
}

void KCMDesignerFields::load()
{
    // see KCModule::showEvent()
    if (!mPageView) {
        delayedInit();
    }
    loadActivePages(readActivePages());
}

QStringList KCMDesignerFields::saveActivePages()
{
    QTreeWidgetItemIterator it(mPageView, QTreeWidgetItemIterator::Checked | QTreeWidgetItemIterator::Selectable);

    QStringList activePages;
    while (*it) {
        if ((*it)->parent() == nullptr) {
            auto item = static_cast<PageItem *>(*it);
            activePages.append(item->name());
        }

        ++it;
    }

    return activePages;
}

void KCMDesignerFields::save()
{
    writeActivePages(saveActivePages());
}

void KCMDesignerFields::defaults()
{
}

void KCMDesignerFields::initGUI()
{
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins({});

    const bool noDesigner = QStandardPaths::findExecutable(QStringLiteral("designer")).isEmpty();

    if (noDesigner) {
        const QString txt = i18n(
            "<qt><b>Warning:</b> Qt Designer could not be found. It is probably not "
            "installed. You will only be able to import existing designer files.</qt>");
        auto lbl = new QLabel(txt, this);
        layout->addWidget(lbl);
    }

    auto hbox = new QHBoxLayout();
    layout->addLayout(hbox);

    mPageView = new QTreeWidget(this);
    mPageView->setHeaderLabel(i18n("Available Pages"));
    mPageView->setRootIsDecorated(true);
    mPageView->setAllColumnsShowFocus(true);
    mPageView->header()->setSectionResizeMode(QHeaderView::Stretch);
    hbox->addWidget(mPageView);

    auto box = new QGroupBox(i18n("Preview of Selected Page"), this);
    auto boxLayout = new QVBoxLayout(box);

    mPagePreview = new QLabel(box);
    mPagePreview->setMinimumWidth(300);
    boxLayout->addWidget(mPagePreview);

    mPageDetails = new QLabel(box);
    boxLayout->addWidget(mPageDetails);
    boxLayout->addStretch(1);

    hbox->addWidget(box);

    loadUiFiles();

    hbox = new QHBoxLayout();
    layout->addLayout(hbox);

    const QString cwHowto = i18n(
        "<qt><p>This section allows you to add your own GUI"
        " Elements ('<i>Widgets</i>') to store your own values"
        " into %1. Proceed as described below:</p>"
        "<ol>"
        "<li>Click on '<i>Edit with Qt Designer</i>'</li>"
        "<li>In the dialog, select '<i>Widget</i>', then click <i>OK</i></li>"
        "<li>Add your widgets to the form</li>"
        "<li>Save the file in the directory proposed by Qt Designer</li>"
        "<li>Close Qt Designer</li>"
        "</ol>"
        "<p>In case you already have a designer file (*.ui) located"
        " somewhere on your hard disk, simply choose '<i>Import Page</i>'</p>"
        "<p><b>Important:</b> The name of each input widget you place within"
        " the form must start with '<i>X_</i>'; so if you want the widget to"
        " correspond to your custom entry '<i>X-Foo</i>', set the widget's"
        " <i>name</i> property to '<i>X_Foo</i>'.</p>"
        "<p><b>Important:</b> The widget will edit custom fields with an"
        " application name of %2.  To change the application name"
        " to be edited, set the widget name in Qt Designer.</p></qt>",
        applicationName(),
        applicationName());

    auto activeLabel = new QLabel(i18n("<a href=\"whatsthis:%1\">How does this work?</a>", cwHowto), this);
    activeLabel->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);
    connect(activeLabel, &QLabel::linkActivated, this, &KCMDesignerFields::showWhatsThis);
    activeLabel->setContextMenuPolicy(Qt::NoContextMenu);
    hbox->addWidget(activeLabel);

    // ### why is this needed? Looks like a KActiveLabel bug...
    activeLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

    hbox->addStretch(1);

    mDeleteButton = new QPushButton(i18n("Delete Page"), this);
    mDeleteButton->setEnabled(false);
    hbox->addWidget(mDeleteButton);
    mImportButton = new QPushButton(i18n("Import Page..."), this);
    hbox->addWidget(mImportButton);
    mDesignerButton = new QPushButton(i18n("Edit with Qt Designer..."), this);
    hbox->addWidget(mDesignerButton);

    if (noDesigner) {
        mDesignerButton->setEnabled(false);
    }
}

void KCMDesignerFields::updatePreview()
{
    QTreeWidgetItem *item = nullptr;
    if (mPageView->selectedItems().size() == 1) {
        item = mPageView->selectedItems().first();
    }
    bool widgetItemSelected = false;

    if (item) {
        if (item->parent()) {
            const QString details = QStringLiteral(
                                        "<qt><table>"
                                        "<tr><td align=\"right\"><b>%1</b></td><td>%2</td></tr>"
                                        "<tr><td align=\"right\"><b>%3</b></td><td>%4</td></tr>"
                                        "<tr><td align=\"right\"><b>%5</b></td><td>%6</td></tr>"
                                        "<tr><td align=\"right\"><b>%7</b></td><td>%8</td></tr>"
                                        "</table></qt>")
                                        .arg(i18n("Key:"),
                                             item->text(0).replace(QLatin1String("X_"), QStringLiteral("X-")),
                                             i18n("Type:"),
                                             item->text(1),
                                             i18n("Classname:"),
                                             item->text(2),
                                             i18n("Description:"),
                                             item->text(3));

            mPageDetails->setText(details);

            auto pageItem = static_cast<PageItem *>(item->parent());
            mPagePreview->setWindowIcon(pageItem->preview());
        } else {
            mPageDetails->setText(QString());

            auto pageItem = static_cast<PageItem *>(item);
            mPagePreview->setWindowIcon(pageItem->preview());

            widgetItemSelected = true;
        }

        mPagePreview->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    } else {
        mPagePreview->setWindowIcon(QPixmap());
        mPagePreview->setFrameStyle(0);
        mPageDetails->setText(QString());
    }

    mDeleteButton->setEnabled(widgetItemSelected);
}

void KCMDesignerFields::itemClicked(QTreeWidgetItem *item)
{
    if (!item || item->parent() != nullptr) {
        return;
    }

    auto pageItem = static_cast<PageItem *>(item);

    if (pageItem->isOn() != pageItem->isActive()) {
        Q_EMIT changed(true);
        pageItem->setIsActive(pageItem->isOn());
    }
}

void KCMDesignerFields::startDesigner()
{
    // check if path exists and create one if not.
    QString cepPath = localUiDir();
    if (!QDir(cepPath).exists()) {
        QDir().mkdir(cepPath);
    }

    // finally jump there
    QDir::setCurrent(QLatin1String(cepPath.toLocal8Bit()));

    QStringList args;
    QTreeWidgetItem *item = nullptr;
    if (mPageView->selectedItems().size() == 1) {
        item = mPageView->selectedItems().constFirst();
    }
    if (item) {
        auto pageItem = static_cast<PageItem *>(item->parent() ? item->parent() : item);
        args.append(pageItem->path());
    }

    auto job = new KIO::CommandLauncherJob(QStringLiteral("designer"), args, this);
    job->setUiDelegate(new KDialogJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, this));
    job->start();
}

void KCMDesignerFields::showWhatsThis(const QString &href)
{
    if (href.startsWith(QLatin1String("whatsthis:"))) {
        const QPoint pos = QCursor::pos();
        QWhatsThis::showText(pos, href.mid(10), this);
    }
}
