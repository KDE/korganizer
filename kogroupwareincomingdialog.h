#ifndef KOGROUPWAREINCOMINGDIALOG_H
#define KOGROUPWAREINCOMINGDIALOG_H

#include "kogroupwareincomingdialog_base.h"

namespace KCal { class Incidence; };


class KOGroupwareIncomingDialog : public KOGroupwareIncomingDialog_base
{
    Q_OBJECT

public:
    KOGroupwareIncomingDialog( KCal::Incidence*, QWidget* parent = 0, 
			       const char* name = 0, bool modal = FALSE, 
			       WFlags fl = 0 );
    ~KOGroupwareIncomingDialog();

    bool isAccepted() const { return mAccepted; }
    bool isConditionallyAccepted() const { return mConditionallyAccepted; }
    bool isDeclined() const { return mDeclined; }

protected slots:
    void slotAcceptEvent();
    void slotAcceptEventConditionally();
    void slotDeclineEvent();

private:
    bool mAccepted;
    bool mConditionallyAccepted;
    bool mDeclined;
};

#endif // KOGROUPWAREINCOMINGDIALOG_H
