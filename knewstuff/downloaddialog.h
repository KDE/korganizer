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

/**
 * @short Common download dialog for data browsing and installation.
 *
 * It provides an easy-to-use convenience method named open() which does all
 * the work, unless a more complex operation is needed.
 * \code
 * KNewStuff::DownloadDialog::open("kdesktop/wallpapers");
 * \endcode
 *
 * @author Josef Spillner (spillner@kde.org)
 * \par Maintainer:
 * Josef Spillner (spillner@kde.org)
 */
class DownloadDialog : public KDialogBase
{
    Q_OBJECT
  public:
    /**
      Constructor.

      @param engine a pre-built engine object, or NULL if the download
                    dialog should create an engine on its own
      @param parent the parent window
    */
    DownloadDialog(Engine *engine, QWidget *parent = 0);

    /**
      Alternative constructor.
      Always uses an internal engine.

      @param parent the parent window
    */
    DownloadDialog(QWidget *parent = 0);

    /**
      Destructor.
    */
    ~DownloadDialog();

    /**
      Restricts the display of available data to a certain data type.

      @param type a Hotstuff data type such as "korganizer/calendar"
    */
    void setType(const QString &type);

    /**
      Fetches descriptions of all available data, optionally considering
      a previously set type.
    */
    void load();

    /**
      Adds another provider to the download dialog.
      This is normally done internally.

      @param p the Hotstuff provider to be added
    */
    void addProvider(Provider *p);

    /**
      Adds an additional entry to the current provider.
      This is normally done internally.

      @param entry a Hotstuff data entry to be added
    */
    void addEntry(Entry *entry);

    /**
      Clears the entry list of the current provider.
      This is normally done internally.
    */
    void clear();

    /**
      Opens the download dialog.
      This is a convenience method which automatically sets up the dialog.
      @see setType()
      @see load()

      @param type a data type such as "korganizer/calendar"
    */
    static void open(const QString &type);

  public slots:
    /**
      Availability of the provider list.

      @param list list of Hotstuff providers
    */
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

