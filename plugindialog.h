#ifndef PLUGINDIALOG_H
#define PLUGINDIALOG_H

#include <qlistview.h>

#include <kdialogbase.h>

class PluginDialog : public KDialogBase
{
    Q_OBJECT
  public:
    PluginDialog(QWidget *parent=0);
    virtual ~PluginDialog();

  signals:
    void configChanged();

  protected slots:
    void slotOk();
    
    void configure();

    void checkSelection();

  private:
    QListView *mListView;
};

#endif
