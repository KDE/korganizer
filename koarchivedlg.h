#ifndef _ARCHIVE_DLG
#define _ARCHIVE_DLG

#include <qdialog.h>

class ArchiveDialog : public QDialog
{
  Q_OBJECT
    
public:
  ArchiveDialog(QWidget *parent = 0, const char *name = 0);
  virtual ~ArchiveDialog();

public slots:

protected slots:
    void accept();

signals:

protected:

};

#endif
