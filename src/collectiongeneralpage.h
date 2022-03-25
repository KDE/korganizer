/*
  SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
  SPDX-FileContributor: Tobias Koenig <tokoe@kdab.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
#pragma once

#include "korganizerprivate_export.h"

#include <Akonadi/CollectionPropertiesPage>

class QLineEdit;
class KIconButton;
class QCheckBox;

namespace PimCommon
{
class IncidencesForWidget;
}
namespace CalendarSupport
{
class KORGANIZERPRIVATE_EXPORT CollectionGeneralPage : public Akonadi::CollectionPropertiesPage
{
    Q_OBJECT

public:
    explicit CollectionGeneralPage(QWidget *parent = nullptr);
    ~CollectionGeneralPage() override;

    void load(const Akonadi::Collection &collection) override;
    void save(Akonadi::Collection &collection) override;

private:
    void init(const Akonadi::Collection &collection);
    QCheckBox *mBlockAlarmsCheckBox = nullptr;
    QLineEdit *mNameEdit = nullptr;
    QCheckBox *mIconCheckBox = nullptr;
    KIconButton *mIconButton = nullptr;
    PimCommon::IncidencesForWidget *mIncidencesForComboBox = nullptr;
};

AKONADI_COLLECTION_PROPERTIES_PAGE_FACTORY(CollectionGeneralPageFactory, CollectionGeneralPage)
}
