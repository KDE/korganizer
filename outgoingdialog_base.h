/****************************************************************************
** Form interface generated from reading ui file './outgoingdialog_base.ui'
**
** Created: Tue Oct 31 03:34:16 2000
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef OUTGOINGDIALOG_BASE_H
#define OUTGOINGDIALOG_BASE_H

#include <qvariant.h>
#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QListView;
class QListViewItem;
class QPushButton;

class OutgoingDialog_base : public QDialog
{ 
    Q_OBJECT

public:
    OutgoingDialog_base( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~OutgoingDialog_base();

    QListView* mMessageListView;
    QPushButton* PushButton5;
    QPushButton* PushButton6;

protected slots:
    virtual void send();

protected:
    QGridLayout* grid;
};

#endif // OUTGOINGDIALOG_BASE_H
