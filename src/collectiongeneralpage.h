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
    KORGANIZERPRIVATE_NO_EXPORT void init();
    QCheckBox *mBlockAlarmsCheckBox = nullptr;
    QLineEdit *mNameEdit = nullptr;
    QCheckBox *mIconCheckBox = nullptr;
    KIconButton *mIconButton = nullptr;
};

AKONADI_COLLECTION_PROPERTIES_PAGE_FACTORY(CollectionGeneralPageFactory, CollectionGeneralPage)
}
