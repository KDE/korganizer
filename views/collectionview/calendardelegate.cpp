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
#include <akonadi/collectionidentificationattribute.h>

StyledCalendarDelegate::StyledCalendarDelegate(QObject * parent)
    : QStyledItemDelegate(parent)
{
    mPixmap.insert(Enable, KIconLoader().loadIcon(QLatin1String("bookmarks"), KIconLoader::Small));
    mPixmap.insert(RemoveFromList, KIconLoader().loadIcon(QLatin1String("list-remove"), KIconLoader::Small));
    mPixmap.insert(AddToList, KIconLoader().loadIcon(QLatin1String("list-add"), KIconLoader::Small));
    mPixmap.insert(Quickview, KIconLoader().loadIcon(QLatin1String("quickview"), KIconLoader::Small));
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
    QStyleOptionButton option;
    option.icon = pixmap;
    QRect r = opt.rect;
    const int h = r.height() - 4;
    option.rect = enableButtonRect(r, pos);
    option.state = QStyle::State_Active | QStyle::State_Enabled;
    option.iconSize = QSize(h, h);
    return option;
}

static bool isChildOfPersonCollection(const QModelIndex &index)
{
    QModelIndex parent = index.parent();
    while (parent.isValid()) {
        if (parent.data(NodeTypeRole).toInt() == PersonNodeRole) {
            return true;
        }
        parent = parent.parent();
    }
    return false;
}

QList<StyledCalendarDelegate::Action> StyledCalendarDelegate::getActions(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const bool isSearchResult = index.data(IsSearchResultRole).toBool();
    const bool hover = option.state & QStyle::State_MouseOver;
    const Akonadi::Collection col = CalendarSupport::collectionFromIndex(index);
    Qt::CheckState enabled = static_cast<Qt::CheckState>(index.data(EnabledRole).toInt());
    // kDebug() << index.data().toString() << enabled;

    QList<Action> buttons;

    CollectionIdentificationAttribute *attr = col.attribute<CollectionIdentificationAttribute>();
    if (attr && attr->collectionNamespace().startsWith("user")) {
        buttons << Quickview;
    }
    if (isSearchResult) {
        buttons << AddToList;
    } else {
        if (hover) {
            if (enabled != Qt::Checked) {
                buttons << Enable;
            }
            //The remove button should not be available for person subfolders
            if (!isChildOfPersonCollection(index)) {
                buttons << RemoveFromList;
            }
        } else {
            if (enabled == Qt::Checked) {
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
    //We display the toolbuttons while hovering
    const bool showButtons = option.state & QStyle::State_MouseOver;
    // const bool enabled = col.shouldList(Akonadi::Collection::ListDisplay);
    Qt::CheckState enabled = static_cast<Qt::CheckState>(index.data(EnabledRole).toInt());

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
            if (action == Enable && showButtons) {
                buttonOption.state = QStyle::State_Active;
            }
            if (action == Enable && !showButtons && enabled == Qt::PartiallyChecked) {
                buttonOption.state = QStyle::State_Active;
            }
            s->drawControl(QStyle::CE_PushButton, &buttonOption, painter, 0);
            i++;
        }
    }

    //Color indicator
    if (opt.checkState){
        QColor color = KOHelper::resourceColor(col);
        if (color.isValid()){
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

        for (int i = 1; i < 4; i++) {
            if (enableButtonRect(option.rect, i).contains(me->pos())) {
                button = i;
                break;
            }
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

    QList<StyledCalendarDelegate::Action> actions = getActions(opt, index);
    if (actions.count() >= button) {
        const Action a = actions.at(button - 1);
        emit action(index, a);
        return true;
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

QSize StyledCalendarDelegate::sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    //Without this adjustment toplevel resource folders get a slightly greater height, which looks silly and breaks the toolbutton position.
    size.setHeight(mPixmap.value(AddToList).height() + 4);
    return size;
}

