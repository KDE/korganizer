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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef KOWHATSNEXTVIEW_H
#define KOWHATSNEXTVIEW_H
// $Id$

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

    virtual int maxDatesHint();
    virtual int currentDateCount();
    virtual QPtrList<Incidence> selectedIncidences();
    DateList selectedDates()
      {DateList q;
       return q;}    
    virtual void printPreview(CalPrinter *calPrinter,
                              const QDate &, const QDate &);
  
  public slots:
    virtual void updateView();
    virtual void showDates(const QDate &start, const QDate &end);
    virtual void showEvents(QPtrList<Event> eventList);

    void changeEventDisplay(Event *, int);
  
  protected:
    void appendEvent(Event *, bool reply=false);
    void appendTodo(Todo *);
  
  private slots:
    void showIncidence(const QString &);

  private:
    void createEventViewer();

    QTextBrowser *mView;
    QString mText;

    KOEventViewerDialog *mEventViewer;
};

#endif
