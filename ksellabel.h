/*
    This file is part of KOrganizer.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef _KSELLABEL_H 
#define _KSELLABEL_H
// $Id$

#include <qframe.h>

class KSelLabel: public QFrame {
   Q_OBJECT
 public:
   KSelLabel(QWidget *parent = 0, const QString & text = QString::null,
	     int idx =  0, const char *name = 0);
   ~KSelLabel();
   QString text();
   QSize sizeHint() const;

 public slots:
   void setText(const QString &);
   void setActivated(bool activated);
   void setAlignment(int align);

 signals:
   void labelActivated(int);
   void newEventSignal(int);

 protected slots:
   void paintEvent(QPaintEvent *); 
   void focusInEvent(QFocusEvent *);
   void mouseDoubleClickEvent(QMouseEvent *);
   void updateLabel();
 
 private:
   // this label's text.
   QString  labeltext;
   // this labels index. It will be passed back in all activation
   // signals, so it can be used to identify the label being selected.
   int      index;
   int      alignment;
   bool     act;
};

#endif
