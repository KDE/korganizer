/****************************************************************************
** Form interface generated from reading ui file './incomingdialog_base.ui'
**
** Created: Tue Oct 31 03:34:17 2000
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef INCOMINGDIALOG_BASE_H
#define INCOMINGDIALOG_BASE_H

#include <qvariant.h>
#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QGroupBox;
class QListView;
class QListViewItem;
class QPushButton;

class IncomingDialog_base : public QDialog
{ 
    Q_OBJECT

public:
    IncomingDialog_base( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~IncomingDialog_base();

    QGroupBox* GroupBox1;
    QPushButton* PushButton9;
    QPushButton* PushButton8;
    QPushButton* PushButton4;
    QPushButton* PushButton7;
    QListView* mMessageListView;
    QPushButton* PushButton7_2;

protected slots:
    virtual void acceptAllMessages();
    virtual void acceptMessage();
    virtual void rejectMessage();
    virtual void retrieve();

protected:
    QGridLayout* grid;
    QGridLayout* grid_2;
};

#endif // INCOMINGDIALOG_BASE_H
