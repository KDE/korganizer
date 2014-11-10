/*
  This file is part of KOrganizer.

  Copyright (C) 2012 Sergio Martins <iamsergio@gmail.com>

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
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole);

Q_SIGNALS:
    void aboutToToggle(bool oldState);
    void toggled(bool newState);
};

#endif
