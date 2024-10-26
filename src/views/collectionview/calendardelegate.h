/*
 * SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
 */

#pragma once

#include <QStyledItemDelegate>

class StyledCalendarDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit StyledCalendarDelegate(QObject *parent);
    ~StyledCalendarDelegate() override;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    enum Action {
        Quickview,
        Total
    };

Q_SIGNALS:
    void action(const QModelIndex &, int);

protected:
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

private:
    QList<Action> getActions(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QHash<Action, QIcon> mIcon;
};
