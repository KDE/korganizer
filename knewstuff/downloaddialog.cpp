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

#include "downloaddialog.h"
#include "downloaddialog.moc"

#include <klocale.h>
#include <ktabctl.h>
#include <klistview.h>
#include <kdebug.h>
#include <kio/job.h>
#include <kio/netaccess.h>
#include <kmessagebox.h>
#include <kurl.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kiconloader.h>

#include <knewstuff/entry.h>
#include <knewstuff/knewstuffgeneric.h>
#include <knewstuff/engine.h>

#include <qlayout.h>
#include <qpushbutton.h>
#include <qdom.h>
#include <qlabel.h>
#include <qtextbrowser.h>
#include <qtimer.h> // hack

using namespace KNS;

DownloadDialog::DownloadDialog(Engine *engine, QWidget *)
: KDialogBase(KDialogBase::IconList, i18n("Get Hot New Stuff"),
  KDialogBase::Close, KDialogBase::Close)
{
  init(engine);
}

DownloadDialog::DownloadDialog(QWidget *)
: KDialogBase(KDialogBase::IconList, i18n("Get Hot New Stuff"),
  KDialogBase::Close, KDialogBase::Close)
{
  init(0);
}

void DownloadDialog::init(Engine *engine)
{
  resize(700, 400);

  m_engine = engine;
  m_page = NULL;

  connect(this, SIGNAL(aboutToShowPage(QWidget*)), SLOT(slotPage(QWidget*)));

  if(!engine)
  {
    m_loader = new ProviderLoader(this);
    connect(m_loader, SIGNAL(providersLoaded(Provider::List*)), SLOT(slotProviders(Provider::List*)));
  }
}

DownloadDialog::~DownloadDialog()
{
}

void DownloadDialog::load()
{
  m_loader->load(m_filter);
}

void DownloadDialog::clear()
{
  QMap<QWidget*, QValueList<KListView*>* >::Iterator it;
  for(it = m_map.begin(); it != m_map.end(); ++it)
  {
    QValueList<KListView*> *v = it.data();
    kdDebug(5850) << "clear listviews in " << v << endl;
    if(v)
    {
      (*(v->at(0)))->clear();
      (*(v->at(1)))->clear();
      (*(v->at(2)))->clear();

      //delete (*it);
    }

    delete it.key();
  }
  m_map.clear();
}

void DownloadDialog::slotProviders(Provider::List *list)
{
  Provider *p;
  /*QFrame *frame;*/

  for(p = list->first(); p; p = list->next())
  {
    kdDebug(5850) << "++ provider ++ " << p->name() << endl;

    if(!m_filter.isEmpty())
      loadProvider(p);
    else
      addProvider(p);
    /*if(p == list->getFirst())
      slotPage(m_frame);*/ // only if !qtbug
  }
}

void DownloadDialog::addProvider(Provider *p)
{
  QFrame *frame;
  KTabCtl *ctl;
  QWidget *w_d, *w_r, *w_l;
  QWidget *w2;
  KListView *lvtmp_r, *lvtmp_d, *lvtmp_l;
  QTextBrowser *rt;
  QString tmp;

  if(m_map.count() == 0)
  {
    frame = addPage(i18n("Welcome"), i18n("Welcome"), QPixmap(""));
    delete frame;
  }

  kdDebug(5850) << "addProvider()/begin" << endl;

  KIO::NetAccess::download(p->icon(), tmp, this);
  frame = addPage(p->name(), p->name(), QPixmap(tmp));
  m_frame = frame;

  w2 = new QWidget(frame);
  w_d = new QWidget(frame);
  w_r = new QWidget(frame);
  w_l = new QWidget(frame);

  ctl = new KTabCtl(frame);
  ctl->addTab(w_r, i18n("Highest Rated"));
  ctl->addTab(w_d, i18n("Most Downloads"));
  ctl->addTab(w_l, i18n("Latest"));

  m_curtab = 0;
  connect(ctl, SIGNAL(tabSelected(int)), SLOT(slotTab(int)));

  QHBoxLayout *box = new QHBoxLayout(frame);
  box->add(ctl);

  lvtmp_r = new KListView(w_r);
  lvtmp_r->addColumn(i18n("Name"));
  lvtmp_r->addColumn(i18n("Version"));
  lvtmp_r->addColumn(i18n("Rating"));
  lvtmp_r->setSorting(2, false);

  lvtmp_d = new KListView(w_d);
  lvtmp_d->addColumn(i18n("Name"));
  lvtmp_d->addColumn(i18n("Version"));
  lvtmp_d->addColumn(i18n("Downloads"));
  lvtmp_d->setSorting(2, false);

  lvtmp_l = new KListView(w_l);
  lvtmp_l->addColumn(i18n("Name"));
  lvtmp_l->addColumn(i18n("Version"));
  lvtmp_l->addColumn(i18n("Release Date"));
  lvtmp_l->setSorting(2, false);

  connect(lvtmp_r, SIGNAL(selectionChanged()), SLOT(slotSelected()));
  connect(lvtmp_d, SIGNAL(selectionChanged()), SLOT(slotSelected()));
  connect(lvtmp_l, SIGNAL(selectionChanged()), SLOT(slotSelected()));

  rt = new QTextBrowser(frame);
  rt->setMinimumWidth(150);

  QPushButton *in = new QPushButton(i18n("Install"), frame);
  QPushButton *de = new QPushButton(i18n("Details"), frame);
  in->setEnabled(false);
  de->setEnabled(false);

  box->addSpacing(spacingHint());
  QVBoxLayout *vbox = new QVBoxLayout(box);
  vbox->add(rt);
  vbox->addSpacing(spacingHint());
  vbox->add(de);
  vbox->add(in);

  connect(in, SIGNAL(clicked()), SLOT(slotInstall()));
  connect(de, SIGNAL(clicked()), SLOT(slotDetails()));

  QVBoxLayout *box2 = new QVBoxLayout(w_r);
  box2->add(lvtmp_r);
  QVBoxLayout *box3 = new QVBoxLayout(w_d);
  box3->add(lvtmp_d);
  QVBoxLayout *box4 = new QVBoxLayout(w_l);
  box4->add(lvtmp_l);

  QValueList<KListView*> *v = new QValueList<KListView*>;
  *v << lvtmp_r << lvtmp_d << lvtmp_l;
  m_map[frame] = v;
  m_rts[frame] = rt;
  QValueList<QPushButton*> *vb = new QValueList<QPushButton*>;
  *vb << in << de;
  m_buttons[frame] = vb;
  m_providers[frame] = p;

  kdDebug(5850) << "addProvider()/end; lvtmp_r = " << lvtmp_r << endl;

  if(m_engine) slotPage(frame);

  QTimer::singleShot(100, this, SLOT(slotFinish()));
}

void DownloadDialog::slotResult(KIO::Job *job)
{
  QDomDocument dom;
  QDomElement knewstuff;

  kdDebug(5850) << "got data: " << m_data[job] << endl;

  kapp->config()->setGroup("KNewStuffStatus");

  dom.setContent(m_data[job]);
  knewstuff = dom.documentElement();

  for(QDomNode pn = knewstuff.firstChild(); !pn.isNull(); pn = pn.nextSibling())
  {
    QDomElement stuff = pn.toElement();

    kdDebug(5850) << "element: " << stuff.tagName() << endl;

    if(stuff.tagName() == "stuff")
    {
      Entry *entry = new Entry(stuff);
      kdDebug(5850) << "TYPE::" << entry->type() << " FILTER::" << m_filter << endl;
      if(!entry->type().isEmpty())
      {
        if((!m_filter.isEmpty()) && (entry->type() != m_filter)) continue;
      }

      if((!m_filter.isEmpty()) && (m_jobs[job]))
      {
        Provider *p = m_jobs[job];
        addProvider(p);
        slotPage(m_frame);
        m_jobs[job] = 0;
      }
      addEntry(entry);
    }
  }

  m_data[job] = "";
}

void DownloadDialog::addEntry(Entry *entry)
{
  QPixmap pix;
  QDate date;
  int installed;

  /*if(m_engine)
  {
    if(m_map.count() == 0)
    {
      m_frame = addPage(i18n("Welcome"), i18n("Welcome"), QPixmap(""));
      Provider *p = new Provider();
      p->setName(i18n("Generic"));
      addProvider(p);
      slotPage(m_frame);
    }
  }*/
  kapp->config()->setGroup("KNewStuffStatus");
  date = QDate::fromString(kapp->config()->readEntry(entry->name()));
  if(!date.isValid()) installed = 0;
  else if(date < entry->releaseDate()) installed = -1;
  else installed = 1;

  if(installed > 0) pix = KGlobal::iconLoader()->loadIcon("ok", KIcon::Small);
  else if(installed < 0) pix = KGlobal::iconLoader()->loadIcon("history", KIcon::Small);
  else pix = QPixmap();

  KListViewItem *tmp_r = new KListViewItem(lv_r,
    entry->name(), entry->version(), QString("%1").arg(entry->rating()));
  KListViewItem *tmp_d = new KListViewItem(lv_d,
    entry->name(), entry->version(), QString("%1").arg(entry->downloads()));
  KListViewItem *tmp_l = new KListViewItem(lv_l,
    entry->name(), entry->version(), entry->releaseDate().toString());

  tmp_r->setPixmap(0, pix);
  tmp_d->setPixmap(0, pix);
  tmp_l->setPixmap(0, pix);

  m_entries.append(entry);

  kdDebug(5850) << "added entry " << entry->name() << endl;
}

void DownloadDialog::slotData(KIO::Job *job, const QByteArray &a)
{
  QCString tmp(a, a.size() + 1);
  m_data[job].append(QString::fromUtf8(tmp));
}

void DownloadDialog::slotDetails()
{
  Entry *e = getEntry();
  if(!e) return;

  QString info = i18n
  (
    "Name: %1\n"
    "Author: %2\n"
    "License: %3\n"
    "Version: %4\n"
    "Release: %5\n"
    "Rating: %6\n"
    "Downloads: %7\n"
    "Release date: %8\n"
    "Summary: %9\n"
    ).arg(e->name()
    ).arg(e->author()
    ).arg(e->license()
    ).arg(e->version()
    ).arg(e->release()
    ).arg(e->rating()
    ).arg(e->downloads()
    ).arg(e->releaseDate().toString()
    ).arg(e->summary()
  );

  info.append(i18n
  (
    "Preview: %1\n"
    "Payload: %2\n"
    ).arg(e->preview().url()
    ).arg(e->payload().url()
  ));

  KMessageBox::information(this, info, i18n("Details"));
}

void DownloadDialog::slotInstall()
{
  Entry *e = getEntry();
  if(!e) return;

  kdDebug(5850) << "download entry now" << endl;

  if(m_engine)
  {
    install(e);
    m_engine->download(e);
  }
  else
  {
    m_s = new KNewStuffGeneric(e->type(), this);

    m_entry = e;

    KURL source = e->payload();
    KURL dest = KURL(m_s->downloadDestination(e));

    KIO::FileCopyJob *job = KIO::file_copy(source, dest, -1, true);
    connect(job, SIGNAL(result(KIO::Job*)), SLOT(slotInstalled(KIO::Job*)));
  }
}

void DownloadDialog::install(Entry *e)
{
  kapp->config()->setGroup("KNewStuffStatus");
  kapp->config()->writeEntry(m_entryname, e->releaseDate().toString());
  kapp->config()->sync();

  QPixmap pix = KGlobal::iconLoader()->loadIcon("ok", KIcon::Small);
  m_entryitem = lv_r->findItem(m_entryname, 0);
  if(m_entryitem) m_entryitem->setPixmap(0, pix);
  m_entryitem = lv_d->findItem(m_entryname, 0);
  if(m_entryitem) m_entryitem->setPixmap(0, pix);
  m_entryitem = lv_l->findItem(m_entryname, 0);
  if(m_entryitem) m_entryitem->setPixmap(0, pix);
}

void DownloadDialog::slotInstalled(KIO::Job *job)
{
  bool ret = (job->error() == 0);
  KIO::FileCopyJob *cjob;

  if(ret)
  {
    cjob = static_cast<KIO::FileCopyJob*>(job);
    if(cjob)
    {
      ret = m_s->install(cjob->destURL().path());
    }
    else ret = false;
  }

  if(ret)
  {
    install(m_entry);

    KMessageBox::information(this, i18n("Installation successful."), i18n("Installation"), "NewStuffDownloadSuccessful" );
  }
  else KMessageBox::error(this, i18n("Installation failed."), i18n("Installation"));

  delete m_s;
}

void DownloadDialog::slotTab(int tab)
{
  kdDebug(5850) << "switch tab to: " << tab << endl;
  m_curtab = tab;
}

void DownloadDialog::slotSelected()
{
  QString tmp;
  Entry *e = getEntry();
  if(e)
  {
    if(!e->preview().isValid())
    {
      m_rt->setText(QString("<b>%1</b><br>%2<br>%3<br><br><i>%4</i><br>(%5)").arg(
        e->name()).arg(e->author()).arg(e->releaseDate().toString()).arg(e->summary()).arg(e->license()));
    }
    else
    {
      KIO::NetAccess::download(e->preview(), tmp, this);
      m_rt->setText(QString("<b>%1</b><br>%2<br>%3<br><br><img src='%4'><br><i>%5</i><br>(%6)").arg(
        e->name()).arg(e->author()).arg(e->releaseDate().toString()).arg(tmp).arg(e->summary()).arg(e->license()));
    }
  }
}

Entry *DownloadDialog::getEntry()
{
  if(m_curtab == 0) m_entryitem = lv_r->currentItem();
  else if(m_curtab == 1) m_entryitem = lv_d->currentItem();
  else if(m_curtab == 2) m_entryitem = lv_l->currentItem();
  else return 0;

  QPushButton *de, *in;
  in = *(m_buttons[m_page]->at(0));
  de = *(m_buttons[m_page]->at(1));
  if(in) in->setEnabled(true);
  if(de) de->setEnabled(true);

  m_entryname = m_entryitem->text(0);

  for(Entry *e = m_entries.first(); e; e = m_entries.next())
  {
    if(e->name() == m_entryname) return e;
  }

  return 0;
}

void DownloadDialog::slotPage(QWidget *w)
{
  Provider *p;

  kdDebug(5850) << "changed widget!!!" << endl;

  if(m_map.find(w) == m_map.end()) return;

  m_page = w;

  lv_r = *(m_map[w]->at(0));
  lv_d = *(m_map[w]->at(1));
  lv_l = *(m_map[w]->at(2));
  p = m_providers[w];
  m_rt = m_rts[w];

  kdDebug(5850) << "valid change!!!; lv_r = " << lv_r << endl;

  if(m_engine) return;

  if(!m_filter.isEmpty()) return;

  lv_r->clear();
  lv_d->clear();
  lv_l->clear();

  kdDebug(5850) << "-- fetch -- " << p->downloadUrl() << endl;

  loadProvider(p);
}

void DownloadDialog::loadProvider(Provider *p)
{
  KIO::TransferJob *job = KIO::get(p->downloadUrl());

  m_jobs[job] = p;

  connect(job, SIGNAL(result(KIO::Job*)), SLOT(slotResult(KIO::Job*)));
  connect(job, SIGNAL(data(KIO::Job*, const QByteArray&)),
    SLOT(slotData(KIO::Job*, const QByteArray&)));
}

void DownloadDialog::setType( const QString &type )
{
  m_filter = type;
}

void DownloadDialog::slotOk()
{
}

void DownloadDialog::slotApply()
{
}

void DownloadDialog::open(const QString &type)
{
  DownloadDialog d;
  d.setType(type);
  d.load();
  d.exec();
}

void DownloadDialog::slotFinish()
{
  showPage(1);
  //updateBackground();
}

