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

StyledCalendarDelegate::StyledCalendarDelegate(QObject * parent)
    : QStyledItemDelegate(parent)
{
}

StyledCalendarDelegate::~StyledCalendarDelegate()
{

}

static QRect enableButtonRect(const QRect &rect)
{
    QRect r = rect;
    const int h = r.height()- 4;
    r.adjust(r.width()- h*2 - 2*2, 2, -2 - h - 2, -2);
    return r;
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

void StyledCalendarDelegate::paint( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    Q_ASSERT(index.isValid());

    QStyledItemDelegate::paint( painter, option, index );
    QStyleOptionViewItemV4 opt = option;
    initStyleOption(&opt, index);

    QStyle *s = style(option);

    const Akonadi::Collection col = CalendarSupport::collectionFromIndex(index);

    //Favorite button
    {
        static QPixmap enablePixmap = KIconLoader().loadIcon(QLatin1String("bookmarks"), KIconLoader::Small);
        static QPixmap disablePixmap = KIconLoader().loadIcon(QLatin1String("window-close"), KIconLoader::Small);
        QStyleOptionButton buttonOpt;
        if (!col.shouldList(Akonadi::Collection::ListDisplay)) {
            buttonOpt.icon = enablePixmap;
        } else {
            buttonOpt.icon = disablePixmap;
        }
        QRect r = opt.rect;
        const int h = r.height()- 4;
        buttonOpt.rect = enableButtonRect(r);
        buttonOpt.state = QStyle::State_Active | QStyle::State_Enabled;
        buttonOpt.iconSize = QSize(h, h);

        s->drawControl(QStyle::CE_PushButton, &buttonOpt, painter, 0);
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

    // make sure that we have the right event type
    if ((event->type() == QEvent::MouseButtonRelease)
        || (event->type() == QEvent::MouseButtonDblClick)
        || (event->type() == QEvent::MouseButtonPress)) {

        QRect buttonRect = enableButtonRect(option.rect);

        QMouseEvent *me = static_cast<QMouseEvent*>(event);
        if (me->button() != Qt::LeftButton || !buttonRect.contains(me->pos())) {
            return QStyledItemDelegate::editorEvent(event, model, option, index);
        }

        if ((event->type() == QEvent::MouseButtonPress)
            || (event->type() == QEvent::MouseButtonDblClick)) {
            return true;
        }
    } else {
        return QStyledItemDelegate::editorEvent(event, model, option, index);
    }

    onEnableButtonClicked(index);
    return true;
}

void StyledCalendarDelegate::onEnableButtonClicked(const QModelIndex &index)
{
    const Akonadi::Collection col = CalendarSupport::collectionFromIndex(index);
    emit enabled(index, !col.enabled());
}

QSize StyledCalendarDelegate::sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
    return QStyledItemDelegate::sizeHint(option, index);
}

