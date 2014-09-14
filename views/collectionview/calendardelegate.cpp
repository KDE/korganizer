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
#include "calendardelegate.h"

#include <KIcon>
#include <KIconLoader>

#include <QApplication>
#include <QPainter>
#include <QMouseEvent>

#include <calendarsupport/utils.h>
#include <kohelper.h>
#include "controller.h"

StyledCalendarDelegate::StyledCalendarDelegate(QObject * parent)
    : QStyledItemDelegate(parent)
{
    mPixmap.insert(Enable, KIconLoader().loadIcon(QLatin1String("bookmarks"), KIconLoader::Small));
    mPixmap.insert(RemoveFromList, KIconLoader().loadIcon(QLatin1String("list-remove"), KIconLoader::Small));
    mPixmap.insert(AddToList, KIconLoader().loadIcon(QLatin1String("list-add"), KIconLoader::Small));
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
    QWidget const *widget = 0;
    if (const QStyleOptionViewItemV3 *v3 = qstyleoption_cast<const QStyleOptionViewItemV3 *>(&option)) {
        widget = v3->widget;
    }
    QStyle *style = widget ? widget->style() : QApplication::style();
    return style;
}

static QStyleOptionButton buttonOpt(const QStyleOptionViewItemV4 &opt, const QPixmap &pixmap, int pos = 1)
{
    QStyleOptionButton buttonOpt;
    buttonOpt.icon = pixmap;
    QRect r = opt.rect;
    const int h = r.height() - 4;
    buttonOpt.rect = enableButtonRect(r, pos);
    buttonOpt.state = QStyle::State_Active | QStyle::State_Enabled;
    buttonOpt.iconSize = QSize(h, h);
    return buttonOpt;
}

QList<StyledCalendarDelegate::Action> StyledCalendarDelegate::getActions(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const bool isSearchResult = index.data(IsSearchResultRole).toBool();
    const bool hover = option.state & QStyle::State_MouseOver;
    const Akonadi::Collection col = CalendarSupport::collectionFromIndex(index);
    const bool enabled = col.shouldList(Akonadi::Collection::ListDisplay);
    const bool referenced = col.referenced();

    QList<Action> buttons;
    if (isSearchResult) {
        buttons << AddToList;
    } else {
        //Folders that have been pulled in due to a subfolder
        // if (!enabled && !referenced) {
        //     return QList<Action>() << RemoveFromList;
        // }
        if (hover) {
            if (!enabled) {
                buttons << Enable;
            }
            buttons << RemoveFromList;
        } else {
            if (enabled) {
                buttons << Enable;
            }
        }
    }
    return buttons;
}

void StyledCalendarDelegate::paint( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    Q_ASSERT(index.isValid());

    const Akonadi::Collection col = CalendarSupport::collectionFromIndex(index);

    QStyleOptionViewItemV4 opt = option;
    initStyleOption(&opt, index);
    QStyledItemDelegate::paint(painter, opt, index);

    QStyle *s = style(option);

    //Buttons
    {
        QList<Action> buttons;
        int i = 1;
        Q_FOREACH (Action action, getActions(option, index)) {
            QStyleOptionButton buttonOption = buttonOpt(opt, mPixmap.value(action), i);
            s->drawControl(QStyle::CE_PushButton, &buttonOption, painter, 0);
            i++;
        }
    }

    //Color indicator
    if (opt.checkState){
        QColor color = KOHelper::resourceColor(col);
        if (color.isValid()){
            QRect r = opt.rect;
            const int h = r.height()- 4;
            r.adjust(r.width()- h - 2, 2, - 2, -2);
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing);
            QPen pen = painter->pen();
            pen.setColor(color);
            QPainterPath path;
            path.addRoundedRect(r, 5, 5);
            color.setAlpha(200);
            painter->fillPath(path, color);
            painter->strokePath(path, pen);
            painter->restore();
        }
    }
}

bool StyledCalendarDelegate::editorEvent(QEvent *event,
                                QAbstractItemModel *model,
                                const QStyleOptionViewItem &option,
                                const QModelIndex &index)
{
    Q_ASSERT(event);
    Q_ASSERT(model);

    int button = -1;
    // make sure that we have the right event type
    if ((event->type() == QEvent::MouseButtonRelease)
        || (event->type() == QEvent::MouseButtonDblClick)
        || (event->type() == QEvent::MouseButtonPress)) {

        QMouseEvent *me = static_cast<QMouseEvent*>(event);

        if (enableButtonRect(option.rect, 1).contains(me->pos())) {
            button = 1;
        }
        if (enableButtonRect(option.rect, 2).contains(me->pos())) {
            button = 2;
        }
        if (enableButtonRect(option.rect, 3).contains(me->pos())) {
            button = 3;
        }
        if (me->button() != Qt::LeftButton || button < 0) {
            return QStyledItemDelegate::editorEvent(event, model, option, index);
        }

        if ((event->type() == QEvent::MouseButtonPress)
            || (event->type() == QEvent::MouseButtonDblClick)) {
            return true;
        }
    } else {
        return QStyledItemDelegate::editorEvent(event, model, option, index);
    }

    Q_ASSERT(button > 0);
    QStyleOptionViewItem opt = option;
    opt.state |= QStyle::State_MouseOver;

    const Action a = getActions(opt, index).at(button - 1);
    // kDebug() << "Button clicked: " << a;
    emit action(index, a);

    return true;
}

QSize StyledCalendarDelegate::sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
    return QStyledItemDelegate::sizeHint(option, index);
}

