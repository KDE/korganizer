/*
  This file is part of KOrganizer.

  Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>
  Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "kcmdesignerfields.h"

#include <KAboutData>
#include "korganizer_debug.h"
#include <KDirWatch>
#include <QFileDialog>
#include <KMessageBox>
#include <KRun>
#include <KShell>
#include <QStandardPaths>
#include <KIO/DeleteJob>
#include <KIO/FileCopyJob>
#include <KIO/MkdirJob>
#include <KJobWidgets>
#include <KLocalizedString>
#include <QDialog>

#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTreeWidget>
#include <QUiLoader>
#include <QWhatsThis>
#include <QDir>
#include <KConfigGroup>

class PageItem : public QTreeWidgetItem
{
public:
    PageItem(QTreeWidget *parent, const QString &path)
        : QTreeWidgetItem(parent),
          mPath(path), mIsActive(false)
    {
        setFlags(flags() | Qt::ItemIsUserCheckable);
        setCheckState(0, Qt::Unchecked);
        mName = path.mid(path.lastIndexOf(QLatin1Char('/')) + 1);

        QFile f(mPath);
        if (!f.open(QFile::ReadOnly)) {
            return;
        }
        QUiLoader builder;
        QWidget *wdg = builder.load(&f, Q_NULLPTR);
        f.close();
        if (wdg) {
            setText(0, wdg->windowTitle());

            QPixmap pm = wdg->grab();
            QImage img = pm.toImage().scaled(300, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation);
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

            QList<QWidget *> list = wdg->findChildren<QWidget *>();
            QWidget *it;
            Q_FOREACH (it, list) {
                if (allowedTypes.contains(QLatin1String(it->metaObject()->className()))) {
                    const QString name = it->objectName();
                    if (name.startsWith(QStringLiteral("X_"))) {
                        new QTreeWidgetItem(this, QStringList()
                                            << name
                                            << allowedTypes[ QLatin1String(it->metaObject()->className()) ]
                                            << QLatin1String(it->metaObject()->className())
                                            << it->whatsThis());
                    }
                }
            }

        }
    }

    QString name() const
    {
        return mName;
    }

    QString path() const
    {
        return mPath;
    }

    QPixmap preview() const
    {
        return mPreview;
    }

    void setIsActive(bool isActive)
    {
        mIsActive = isActive;
    }

    bool isActive() const
    {
        return mIsActive;
    }

    bool isOn() const
    {
        return checkState(0) == Qt::Checked;
    }

private:
    QString mName;
    QString mPath;
    QPixmap mPreview;
    bool mIsActive;
};

KCMDesignerFields::KCMDesignerFields(QWidget *parent,
                                     const QVariantList &args)
    : KCModule(parent, args),
      mPageView(Q_NULLPTR),
      mPagePreview(Q_NULLPTR),
      mPageDetails(Q_NULLPTR),
      mDeleteButton(Q_NULLPTR),
      mImportButton(Q_NULLPTR),
      mDesignerButton(Q_NULLPTR)
{
    KAboutData *about = new KAboutData(QStringLiteral("KCMDesignerfields"),
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
    KDirWatch *dw = new KDirWatch(this);
    QDir().mkpath(localUiDir());
    dw->addDir(localUiDir(), KDirWatch::WatchFiles);
    connect(dw, &KDirWatch::created, this, &KCMDesignerFields::rebuildList);
    connect(dw, &KDirWatch::deleted, this, &KCMDesignerFields::rebuildList);
    connect(dw, &KDirWatch::dirty, this, &KCMDesignerFields::rebuildList);
}

void KCMDesignerFields::deleteFile()
{
    foreach (QTreeWidgetItem *item, mPageView->selectedItems()) {
        PageItem *pageItem = static_cast<PageItem *>(item->parent() ? item->parent() : item);
        if (KMessageBox::warningContinueCancel(
                    this,
                    i18n("<qt>Do you really want to delete '<b>%1</b>'?</qt>",
                         pageItem->text(0)), QString(), KStandardGuiItem::del()) == KMessageBox::Continue) {
            auto job = KIO::del(pageItem->path());
            job->exec();
        }
    }
    // The actual view refresh will be done automagically by the slots connected to kdirwatch
}

void KCMDesignerFields::importFile()
{
    QUrl src = QFileDialog::getOpenFileUrl(this, i18n("Import Page"), QUrl::fromLocalFile(QDir::homePath()),
                                           i18n("Designer Files (*.ui)")
                                          );
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
    Q_FOREACH (const QString &dir, dirs) {
        const QStringList fileNames = QDir(dir).entryList(QStringList() << QStringLiteral("*.ui"));
        Q_FOREACH (const QString &file, fileNames) {
            new PageItem(mPageView, dir + QLatin1Char('/') + file);
        }
    }
}

void KCMDesignerFields::rebuildList()
{
    // If nothing is initialized there is no need to do something
    if (mPageView) {
        QStringList ai = saveActivePages();
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
        if ((*it)->parent() == Q_NULLPTR) {
            PageItem *item = static_cast<PageItem *>(*it);
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
    QTreeWidgetItemIterator it(mPageView, QTreeWidgetItemIterator::Checked |
                               QTreeWidgetItemIterator::Selectable);

    QStringList activePages;
    while (*it) {
        if ((*it)->parent() == Q_NULLPTR) {
            PageItem *item = static_cast<PageItem *>(*it);
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
    QVBoxLayout *layout = new QVBoxLayout(this);

    bool noDesigner = QStandardPaths::findExecutable(QStringLiteral("designer")).isEmpty();

    if (noDesigner) {
        QString txt =
            i18n("<qt><b>Warning:</b> Qt Designer could not be found. It is probably not "
                 "installed. You will only be able to import existing designer files.</qt>");
        QLabel *lbl = new QLabel(txt, this);
        layout->addWidget(lbl);
    }

    QHBoxLayout *hbox = new QHBoxLayout();
    layout->addLayout(hbox);

    mPageView = new QTreeWidget(this);
    mPageView->setHeaderLabel(i18n("Available Pages"));
    mPageView->setRootIsDecorated(true);
    mPageView->setAllColumnsShowFocus(true);
    mPageView->header()->setResizeMode(QHeaderView::Stretch);
    hbox->addWidget(mPageView);

    QGroupBox *box = new QGroupBox(i18n("Preview of Selected Page"), this);
    QVBoxLayout *boxLayout = new QVBoxLayout(box);

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

    QString cwHowto =
        i18n("<qt><p>This section allows you to add your own GUI"
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
             applicationName(), applicationName());

    QLabel *activeLabel = new QLabel(
        i18n("<a href=\"whatsthis:%1\">How does this work?</a>", cwHowto), this);
    activeLabel->setTextInteractionFlags(Qt::LinksAccessibleByMouse |
                                         Qt::LinksAccessibleByKeyboard);
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
    QTreeWidgetItem *item = Q_NULLPTR;
    if (mPageView->selectedItems().size() == 1) {
        item = mPageView->selectedItems().first();
    }
    bool widgetItemSelected = false;

    if (item) {
        if (item->parent()) {
            QString details = QStringLiteral("<qt><table>"
                                             "<tr><td align=\"right\"><b>%1</b></td><td>%2</td></tr>"
                                             "<tr><td align=\"right\"><b>%3</b></td><td>%4</td></tr>"
                                             "<tr><td align=\"right\"><b>%5</b></td><td>%6</td></tr>"
                                             "<tr><td align=\"right\"><b>%7</b></td><td>%8</td></tr>"
                                             "</table></qt>")
                              .arg(i18n("Key:"), item->text(0).replace(QLatin1String("X_"), QStringLiteral("X-")), i18n("Type:"), item->text(1), i18n("Classname:"), item->text(2), i18n("Description:"), item->text(3));

            mPageDetails->setText(details);

            PageItem *pageItem = static_cast<PageItem *>(item->parent());
            mPagePreview->setWindowIcon(pageItem->preview());
        } else {
            mPageDetails->setText(QString());

            PageItem *pageItem = static_cast<PageItem *>(item);
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
    if (!item || item->parent() != Q_NULLPTR) {
        return;
    }

    PageItem *pageItem = static_cast<PageItem *>(item);

    if (pageItem->isOn() != pageItem->isActive()) {
        Q_EMIT changed(true);
        pageItem->setIsActive(pageItem->isOn());
    }
}

void KCMDesignerFields::startDesigner()
{
    QString cmdLine = QStringLiteral("designer");

    // check if path exists and create one if not.
    QString cepPath = localUiDir();
    if (!QDir(cepPath).exists()) {
        auto job = KIO::mkdir(cepPath);
        KJobWidgets::setWindow(job, this);
        job->exec();
    }

    // finally jump there
    QDir::setCurrent(QLatin1String(cepPath.toLocal8Bit()));

    QTreeWidgetItem *item = Q_NULLPTR;
    if (mPageView->selectedItems().size() == 1) {
        item = mPageView->selectedItems().first();
    }
    if (item) {
        PageItem *pageItem = static_cast<PageItem *>(item->parent() ? item->parent() : item);
        cmdLine += QLatin1Char(' ') + KShell::quoteArg(pageItem->path());
    }

    KRun::runCommand(cmdLine, topLevelWidget());
}

void KCMDesignerFields::showWhatsThis(const QString &href)
{
    if (href.startsWith(QStringLiteral("whatsthis:"))) {
        QPoint pos = QCursor::pos();
        QWhatsThis::showText(pos, href.mid(10), this);
    }
}

