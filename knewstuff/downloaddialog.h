/*
    This file is part of KNewStuff.
    Copyright (c) 2003 Josef Spillner <spillner@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef KNEWSTUFF_DOWNLOADDIALOG_H
#define KNEWSTUFF_DOWNLOADDIALOG_H

#include <kdialogbase.h>
#include <knewstuff/provider.h>

namespace KIO
{
  class Job;
}

class KListView;
class QTextBrowser;
class QFrame;
class KNewStuffGeneric;

namespace KNS
{

class ProviderLoader;
class Entry;
class Provider;
class Engine;

class DownloadDialog : public KDialogBase
{
    Q_OBJECT
  public:
    DownloadDialog(Engine *engine, QWidget *parent = 0);
    DownloadDialog(QWidget *parent = 0);
    ~DownloadDialog();
    void setType(QString type);
    void load();

    void addProvider(Provider *p);
    void addEntry(Entry *entry);
    void clear();

    static void open(QString type);

  public slots:
    void slotProviders(Provider::List *list);

  protected slots:
    void slotApply();
    void slotOk();

  private slots:
    void slotResult(KIO::Job *job);
    void slotData(KIO::Job *job, const QByteArray &a);
    void slotInstall();
    void slotDetails();
    void slotInstalled(KIO::Job *job);
    void slotTab(int tab);
    void slotSelected();
    void slotPage(QWidget *w);
    void slotFinish();

  private:
    void init(Engine *e);
    Entry *getEntry();
    void loadProvider(Provider *p);
    void install(Entry *e);

    ProviderLoader *m_loader;
    QString m_entryname;
    KListView *lv_r, *lv_d, *lv_l;
    QTextBrowser *m_rt;
    QFrame *m_frame;
    QListViewItem *m_entryitem;
    QPtrList<Entry> m_entries;
    Entry *m_entry;
    KNewStuffGeneric *m_s;
    int m_curtab;
    QMap<QWidget*, QValueList<KListView*>* > m_map;
    QMap<QWidget*, Provider*> m_providers;
    QMap<QWidget*, QTextBrowser*> m_rts;
    QMap<QWidget*, QValueList<QPushButton*>* > m_buttons;
    QMap<KIO::Job*, Provider*> m_jobs;
    QMap<KIO::Job*, QString> m_data;
    QString m_filter;
    Engine *m_engine;
    QWidget *m_page;
};

}

#endif

