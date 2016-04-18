/*
 * Copyright (c) 2014 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * As a special exception, permission is given to link this program
 * with any edition of Qt, and distribute the resulting executable,
 * without including the source code for Qt in the source distribution.
 */

#ifndef CALENDARDELEGATE_H
#define CALENDARDELEGATE_H

#include <QStyledItemDelegate>

class StyledCalendarDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit StyledCalendarDelegate(QObject *parent);
    virtual ~StyledCalendarDelegate();

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;

    enum Action {
        AddToList,
        RemoveFromList,
        Enable,
        Quickview,
        Total
    };

Q_SIGNALS:
    void action(const QModelIndex &, int);

protected:
    bool editorEvent(QEvent *event,
                     QAbstractItemModel *model,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index) Q_DECL_OVERRIDE;

private:
    QList<Action> getActions(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QHash<Action, QPixmap> mPixmap;
};

#endif

