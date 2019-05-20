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

#include "calendardelegate.h"
#include "kohelper.h"
#include "korganizer_debug.h"

#include <AkonadiCore/CollectionStatistics>

#include <CalendarSupport/CalendarSingleton>
#include <CalendarSupport/Utils>

#include <KIconLoader>

#include <QApplication>
#include <QFontDatabase>
#include <QMouseEvent>
#include <QPainter>

StyledCalendarDelegate::StyledCalendarDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
    mPixmap.insert(Quickview,
                   KIconLoader::global()->loadIcon(QStringLiteral("quickview"),
                                                   KIconLoader::Small));
}

StyledCalendarDelegate::~StyledCalendarDelegate()
{
}

static QRect enableButtonRect(const QRect &rect, int pos = 1)
{
    //2px border on each side of the icon
    static int border = 2;
    const int side = rect.height() - (2 * border);
    const int offset = side * pos + border * (pos + 1);
    return rect.adjusted(rect.width() - (offset + side), border, -offset, -border);
}

static QStyle *style(const QStyleOptionViewItem &option)
{
    QWidget const *widget = nullptr;
    if (const QStyleOptionViewItem *v3 = qstyleoption_cast<const QStyleOptionViewItem *>(&option)) {
        widget = v3->widget;
    }
    QStyle *style = widget ? widget->style() : QApplication::style();
    return style;
}

static QStyleOptionButton buttonOpt(const QStyleOptionViewItem &opt, const QPixmap &pixmap, const QModelIndex &index, int pos = 1)
{
    QStyleOptionButton option;
    option.icon = pixmap;
    QRect r = opt.rect;
    const int h = r.height() - 4;
    option.rect = enableButtonRect(r, pos);
    option.state = QStyle::State_Active | QStyle::State_Enabled;
    option.iconSize = QSize(h, h);
    QWidget *w = const_cast<QWidget *>(opt.widget);
    if (w) {
        const Akonadi::Collection col = CalendarSupport::collectionFromIndex(index);
        w->setToolTip(CalendarSupport::toolTipString(col));
    }
    return option;
}


QList<StyledCalendarDelegate::Action> StyledCalendarDelegate::getActions(
    const QStyleOptionViewItem &, const QModelIndex &index) const
{
    const Akonadi::Collection col = CalendarSupport::collectionFromIndex(index);
    // qCDebug(KORGANIZER_LOG) << index.data().toString() << enabled;
    const bool isSearchCollection
        = col.resource().startsWith(QLatin1String("akonadi_search_resource"));
    const bool isKolabCollection = col.resource().startsWith(QLatin1String(
                                                                 "akonadi_kolab_resource"));
    const bool isTopLevelCollection = (col.parentCollection() == Akonadi::Collection::root());
    const bool isToplevelSearchCollection = (isTopLevelCollection && isSearchCollection);
    const bool isToplevelKolabCollection = (isTopLevelCollection && isKolabCollection);

    QList<Action> buttons;
    if (!isToplevelSearchCollection && !isToplevelKolabCollection) {
        buttons << Quickview;
    }
    if (isSearchCollection && !isToplevelSearchCollection) {
        buttons << Total;
    }
    return buttons;
}

void StyledCalendarDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_ASSERT(index.isValid());

    const Akonadi::Collection col = CalendarSupport::collectionFromIndex(index);

    QStyleOptionViewItem opt = option;
    opt.font = QFont(QFontDatabase::systemFont(QFontDatabase::GeneralFont));
    opt.textElideMode = Qt::ElideLeft;

    initStyleOption(&opt, index);
    QStyledItemDelegate::paint(painter, opt, index);

    QStyle *s = style(option);

    //Buttons
    {
        int i = 1;
        const auto lstActions = getActions(option, index);
        for (Action action : lstActions) {
            if (action != Total) {
                QStyleOptionButton buttonOption = buttonOpt(opt, mPixmap.value(action), index, i);
                s->drawControl(QStyle::CE_PushButton, &buttonOption, painter, nullptr);
            } else {
                QStyleOptionButton buttonOption = buttonOpt(opt, QPixmap(), index, i);
                buttonOption.features = QStyleOptionButton::Flat;
                buttonOption.rect.setHeight(buttonOption.rect.height() + 4);
                if (col.statistics().count() > 0) {
                    buttonOption.text = QString::number(col.statistics().count());
                }
                s->drawControl(QStyle::CE_PushButton, &buttonOption, painter, nullptr);
            }
            i++;
        }
    }

    //Color indicator
    if (opt.checkState) {
        QColor color = KOHelper::resourceColorKnown(col);
        if (!color.isValid()) {
            color = KOHelper::resourceColor(col);
        } else {
            color = KOHelper::resourceColor(col);
        }
        if (color.isValid()) {
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing);
            QPen pen = painter->pen();
            pen.setColor(color);
            QPainterPath path;
            path.addRoundedRect(enableButtonRect(opt.rect, 0), 5, 5);
            color.setAlpha(200);
            painter->fillPath(path, color);
            painter->strokePath(path, pen);
            painter->restore();
        }
    }
}

bool StyledCalendarDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    Q_ASSERT(event);
    Q_ASSERT(model);

    // double-click mouse starts the quickview dialog
    if (event->type() == QEvent::MouseButtonDblClick) {
        Q_EMIT action(index, Quickview);
        return true;
    }

    int button = -1;
    // make sure that we have the right event type
    if ((event->type() == QEvent::MouseButtonRelease)
        || (event->type() == QEvent::MouseButtonPress)) {
        QMouseEvent *me = static_cast<QMouseEvent *>(event);

        for (int i = 1; i < 4; i++) {
            if (enableButtonRect(option.rect, i).contains(me->pos())) {
                button = i;
                break;
            }
        }

        if (me->button() != Qt::LeftButton || button < 0) {
            return QStyledItemDelegate::editorEvent(event, model, option, index);
        }

        if (event->type() == QEvent::MouseButtonPress) {
            return true;
        }
    } else {
        return QStyledItemDelegate::editorEvent(event, model, option, index);
    }

    Q_ASSERT(button > 0);
    QStyleOptionViewItem opt = option;
    opt.state |= QStyle::State_MouseOver;

    QList<StyledCalendarDelegate::Action> actions = getActions(opt, index);
    if (actions.count() >= button) {
        const Action a = actions.at(button - 1);
        Q_EMIT action(index, a);
        return true;
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

QSize StyledCalendarDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    // Without this adjustment toplevel resource folders get a slightly greater height,
    // which looks silly and breaks the toolbutton position.
    size.setHeight(mPixmap.value(Quickview).height() + 4);
    return size;
}
