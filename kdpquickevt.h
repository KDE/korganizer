// kdpquickevt.h
// quick event viewer

#ifndef KDPQUICKEVENTVIEW_H
#define KDPQUICKEVENTVIEW_H

#include <qpushbt.h>
#include <qdialog.h>

#include "calobject.h"
#include "quickdialog.h"

class KDPQuickEventView : public QDialog
{
	Q_OBJECT
 public:
   KDPQuickEventView( CalObject *, QWidget *parent = 0, char *name = 0 );
   ~KDPQuickEventView();
 public slots:
   void updateDialog();
   void setSelected( KOEvent *);
 signals:
   void eventChanged();
 protected:
 protected slots:
   void resizeEvent(QResizeEvent *e);
 private:
   QuickDialog *quickdialog;
   QPushButton *done;
};

#endif
