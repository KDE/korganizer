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

private:
	void init(Engine *e);
	Entry *getEntry();
	void loadProvider(Provider *p);

	ProviderLoader *m_loader;
	QString m_data, m_entryname;
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
	QMap<KIO::Job*, Provider*> m_jobs;
	QString m_filter, m_type;
	Engine *m_engine;
};

}

#endif

