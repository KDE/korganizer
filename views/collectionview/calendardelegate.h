/*
    Copyright (c) 2014 Christian Mollekopf <mollekopf@kolabsys.com>

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
#ifndef CALENDARDELEGATE_H
#define CALENDARDELEGATE_H

#include <QStyledItemDelegate>

class StyledCalendarDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    StyledCalendarDelegate(QObject * parent);
    virtual ~StyledCalendarDelegate();

    void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
    QSize sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const;

    enum Action {
        AddToList,
        RemoveFromList,
        Enable
    };

Q_SIGNALS:
    void action(const QModelIndex &, int);

protected:
    bool editorEvent(QEvent *event,
                    QAbstractItemModel *model,
                    const QStyleOptionViewItem &option,
                    const QModelIndex &index);

private:
    QList<Action> getActions(const QStyleOptionViewItem & option, const QModelIndex &index) const;
    QHash<Action, QPixmap> mPixmap;
};
    
#endif

