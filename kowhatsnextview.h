/*
    This file is part of KOrganizer.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef KOWHATSNEXTVIEW_H
#define KOWHATSNEXTVIEW_H

#include <qtextbrowser.h>

#include <korganizer/baseview.h>

class QListView;

class KOEventViewerDialog;

class WhatsNextTextBrowser : public QTextBrowser {
    Q_OBJECT
  public:
    WhatsNextTextBrowser(QWidget *parent) : QTextBrowser(parent) {}

    void setSource(const QString &);

  signals:
    void showIncidence(const QString &uid);
};


/**
  This class provides a view of the next events and todos
*/
class KOWhatsNextView : public KOrg::BaseView
{
    Q_OBJECT
  public:
    KOWhatsNextView(Calendar *calendar, QWidget *parent = 0,
                    const char *name = 0);
    ~KOWhatsNextView();

    virtual int currentDateCount();
    virtual Incidence::List selectedIncidences() { return Incidence::List(); }
    DateList selectedDates() { return DateList(); }

  public slots:
    virtual void updateView();
    virtual void showDates(const QDate &start, const QDate &end);
    virtual void showIncidences( const Incidence::List &incidenceList );

    void changeIncidenceDisplay(Incidence *, int);

  protected:
    void appendEvent( Incidence *, const QDateTime &start = QDateTime(), 
                      const QDateTime &end = QDateTime() );
    void appendTodo( Incidence *, int );
    Incidence*  findToplevelTodo( Incidence *todo);

  private slots:
    void showIncidence(const QString &);

  private:
    QTextBrowser *mView;
    QString mText;
    QDate mStartDate;
    QDate mEndDate;

    Incidence::List mTodos;
};

#endif
