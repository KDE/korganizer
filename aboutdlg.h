#ifndef ABOUTDLG_H
#define ABOUTDLG_H

#include <qdialog.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qmultilinedit.h>
#include <qtimer.h>
#include <kbuttonbox.h>

class PostcardDialog : public QDialog
{
  Q_OBJECT

public:
  PostcardDialog(QWidget *parent=0, const char *name=0);
  ~PostcardDialog();

private slots:
  void send();

private:
  KButtonBox *buttonBox;
  QMultiLineEdit *commentArea;

};

class AboutDialog : public QDialog
{
  Q_OBJECT

public:
  AboutDialog(QWidget *parent=0, const char *name=0);
  ~AboutDialog();
  
protected slots:
  void movieStatus(int);
  void movieUpdate(const QRect&);
  void sendPostcard();
  void updateContributers();

private:
  QTimer *timer;
  KButtonBox *buttonBox;
  QPushButton *okButton;
  QPushButton *mailButton;
  QLabel *movieLabel, *contribLabel;
};


#endif
