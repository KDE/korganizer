#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <qlistview.h>

#include <kdialogbase.h>

class ConfigDialog : public KDialogBase
{
    Q_OBJECT
  public:
    ConfigDialog(QWidget *parent=0);
    virtual ~ConfigDialog();

  protected:
    void load();
    void save();

  protected slots:
    void slotOk();

  private:
    QComboBox *mHolidayCombo;
    QMap<QString,QString> mCountryMap;
};

#endif
