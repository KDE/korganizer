// 	$Id$	

#ifndef _TODOWINGENERAL_H
#define _TODOWINGENERAL_H

#include <qframe.h>
#include <qlabel.h>
#include <qchkbox.h>
#include <qpushbt.h>
#include <qgrpbox.h>
#include <qlined.h>
#include <qcombo.h>
#include <qmlined.h>
#include <kapp.h>
#include <krestrictedline.h>

#include "ktimed.h"
#include "kdated.h"
#include "wingeneral.h"

class EventWin;
class TodoEventWin;

class TodoWinGeneral : public WinGeneral
{
    Q_OBJECT

    friend EventWin;
    friend TodoEventWin;

  public:
    TodoWinGeneral(QWidget* parent = 0, const char* name = 0);
    virtual ~TodoWinGeneral() {};

  protected slots:
    virtual void setEnabled(bool);
    void timeStuffDisable(bool disable);
    void dueStuffDisable(bool disable);

  signals:
    void nullSignal(QWidget *);

  protected:
    void initMisc();
    void initLayout();
    void initTimeBox();
//  void initAlarmBox();
//  void initTodoSpecific();
};

#endif
