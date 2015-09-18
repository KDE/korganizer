/*
  Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    Author: Tobias Koenig <tokoe@kdab.com>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
*/

#include "collectiongeneralpage.h"
#include "PimCommon/PimUtil"
#include <Collection>
#include <EntityDisplayAttribute>
#include <Akonadi/Calendar/BlockAlarmsAttribute>

#include <KCalCore/Event>
#include <KCalCore/Journal>
#include <KCalCore/Todo>

#include <QDialog>
#include <KIconButton>
#include <QLineEdit>
#include <KLocalizedString>

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <KConfigGroup>

#include <MailCommon/CollectionAnnotationsAttribute>
#include <MailCommon/CollectionTypeUtil>
#include <MailCommon/IncidencesForWidget>

using namespace Akonadi;
using namespace CalendarSupport;

CollectionGeneralPage::CollectionGeneralPage(QWidget *parent)
    : CollectionPropertiesPage(parent),
      mBlockAlarmsCheckBox(Q_NULLPTR),
      mNameEdit(Q_NULLPTR),
      mIconCheckBox(Q_NULLPTR),
      mIconButton(Q_NULLPTR),
      mIncidencesForComboBox(Q_NULLPTR)
{
    setObjectName(QStringLiteral("CalendarSupport::CollectionGeneralPage"));
    setPageTitle(i18nc("@title:tab General settings for a folder.", "General"));
}

void CollectionGeneralPage::init(const Akonadi::Collection &collection)
{
    QVBoxLayout *topLayout = new QVBoxLayout(this);

    QHBoxLayout *hbox = new QHBoxLayout();
    topLayout->addItem(hbox);

    QLabel *label = new QLabel(i18nc("@label:textbox Name of the folder.", "&Name:"), this);
    hbox->addWidget(label);

    mNameEdit = new QLineEdit(this);
    mNameEdit->setToolTip(
        i18nc("@info:tooltip", "Set the folder name"));
    mNameEdit->setWhatsThis(
        i18nc("@info:whatsthis",
              "Enter a name here to set the name of this folder."));
    label->setBuddy(mNameEdit);
    hbox->addWidget(mNameEdit);

    // should replies to mails in this folder be kept in this same folder?
    hbox = new QHBoxLayout();
    topLayout->addItem(hbox);

    mBlockAlarmsCheckBox = new QCheckBox(i18nc("@option:check", "Block reminders locally"), this);
    mBlockAlarmsCheckBox->setToolTip(
        i18nc("@info:tooltip", "Ignore reminders from this calendar"));
    mBlockAlarmsCheckBox->setWhatsThis(
        i18nc("@info:whatsthis",
              "Check this box if you do not want to receive reminders from items "
              "associated with this calendar."));
    hbox->addWidget(mBlockAlarmsCheckBox);

    hbox = new QHBoxLayout();
    topLayout->addItem(hbox);
    mIconCheckBox = new QCheckBox(i18nc("@option:check", "&Use custom icon:"), this);
    mIconCheckBox->setToolTip(
        i18nc("@info:tooltip", "Set a custom icon"));
    mIconCheckBox->setWhatsThis(
        i18nc("@info:whatsthis",
              "Check this box if you want to set a custom icon for this folder."));
    mIconButton = new KIconButton(this);
    mIconButton->setIconSize(16);
    hbox->addWidget(mIconCheckBox);
    hbox->addWidget(mIconButton);
    hbox->addStretch();

    if ((collection.parentCollection() != Akonadi::Collection::root()) && PimCommon::Util::isImapResource(collection.resource())) {
        const MailCommon::CollectionAnnotationsAttribute *annotationAttribute =
            collection.attribute<MailCommon::CollectionAnnotationsAttribute>();

        const QMap<QByteArray, QByteArray> annotations =
            (annotationAttribute ?
             annotationAttribute->annotations() :
             QMap<QByteArray, QByteArray>());

        MailCommon::CollectionTypeUtil collectionUtil;
        const MailCommon::CollectionTypeUtil::IncidencesFor incidencesFor =
            collectionUtil.incidencesForFromString(QLatin1String(annotations.value(MailCommon::CollectionTypeUtil::kolabIncidencesFor())));
        hbox = new QHBoxLayout();
        topLayout->addItem(hbox);
        mIncidencesForComboBox = new MailCommon::IncidencesForWidget(this);
        hbox->addWidget(mIncidencesForComboBox);

        mIncidencesForComboBox->setCurrentIndex(incidencesFor);
    }
    topLayout->addStretch(100);   // eat all superfluous space
}

CollectionGeneralPage::~CollectionGeneralPage()
{
}

void CollectionGeneralPage::load(const Akonadi::Collection &collection)
{
    init(collection);
    mNameEdit->setEnabled(collection.rights() & Collection::CanChangeCollection);

    const QString displayName = collection.displayName();

    mNameEdit->setText(displayName);
    mBlockAlarmsCheckBox->setChecked(collection.hasAttribute<BlockAlarmsAttribute>() && collection.attribute<BlockAlarmsAttribute>()->isEverythingBlocked());

    QString iconName;
    if (collection.hasAttribute<EntityDisplayAttribute>()) {
        iconName = collection.attribute<EntityDisplayAttribute>()->iconName();
    }

    if (iconName.isEmpty()) {
        const QStringList mimeTypes = collection.contentMimeTypes();
        if (collection.contentMimeTypes().count() > 1 ||
                collection.contentMimeTypes().contains(KCalCore::Event::eventMimeType())) {
            mIconButton->setIcon(QIcon::fromTheme(QStringLiteral("view-pim-calendar")));
        } else if (collection.contentMimeTypes().contains(KCalCore::Todo::todoMimeType())) {
            mIconButton->setIcon(QIcon::fromTheme(QStringLiteral("view-pim-tasks")));
        } else if (collection.contentMimeTypes().contains(KCalCore::Journal::journalMimeType())) {
            mIconButton->setIcon(QIcon::fromTheme(QStringLiteral("view-pim-journal")));
        } else if (mimeTypes.isEmpty()) {
            mIconButton->setIcon(QIcon::fromTheme(QStringLiteral("folder-grey")));
        } else {
            mIconButton->setIcon(QIcon::fromTheme(QStringLiteral("folder")));
        }
    } else {
        mIconButton->setIcon(iconName);
    }
    mIconCheckBox->setChecked(!iconName.isEmpty());
}

void CollectionGeneralPage::save(Collection &collection)
{
    if (collection.hasAttribute<EntityDisplayAttribute>() &&
            !collection.attribute<EntityDisplayAttribute>()->displayName().isEmpty()) {
        collection.attribute<EntityDisplayAttribute>()->setDisplayName(mNameEdit->text());
    } else {
        collection.setName(mNameEdit->text());
    }

    BlockAlarmsAttribute *attr = collection.attribute<BlockAlarmsAttribute>(Collection::AddIfMissing);
    attr->blockEverything(mBlockAlarmsCheckBox->isChecked());

    if (mIconCheckBox->isChecked()) {
        collection.attribute<EntityDisplayAttribute>(Collection::AddIfMissing)->
        setIconName(mIconButton->icon());
    } else if (collection.hasAttribute<EntityDisplayAttribute>()) {
        collection.attribute<EntityDisplayAttribute>()->setIconName(QString());
    }
    MailCommon::CollectionAnnotationsAttribute *annotationsAttribute =
        collection.attribute<MailCommon::CollectionAnnotationsAttribute>(Entity::AddIfMissing);

    QMap<QByteArray, QByteArray> annotations = annotationsAttribute->annotations();

    MailCommon::CollectionTypeUtil collectionUtil;
    if (mIncidencesForComboBox && mIncidencesForComboBox->isEnabled()) {
        annotations[ MailCommon::CollectionTypeUtil::kolabIncidencesFor() ] =
            collectionUtil.incidencesForToString(
                static_cast<MailCommon::CollectionTypeUtil::IncidencesFor>(mIncidencesForComboBox->currentIndex())).toLatin1();
    }

    if (annotations.isEmpty()) {
        collection.removeAttribute<MailCommon::CollectionAnnotationsAttribute>();
    } else {
        annotationsAttribute->setAnnotations(annotations);
    }

}

