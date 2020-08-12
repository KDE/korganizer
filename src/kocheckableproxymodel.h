/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2012 Sergio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#ifndef KORG_KOCHECKABLE_PROXYMODEL_H
#define KORG_KOCHECKABLE_PROXYMODEL_H

#include <KCheckableProxyModel>

// TODO: This functionality could be ported to the KCheckableProxyModel
/**
 * A KCheckableProxyModel that emits a signal before and after toggling.
 *
 * Listeners, like to-do view, restore tree expand state before unchecking,
 * and restore after checking.
 */
class KOCheckableProxyModel : public KCheckableProxyModel
{
    Q_OBJECT
public:
    explicit KOCheckableProxyModel(QObject *parent);

    /**reimp*/
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

Q_SIGNALS:
    void aboutToToggle(bool oldState);
    void toggled(bool newState);
};

#endif
