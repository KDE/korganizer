// $Id$

#include <qhbox.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qspinbox.h>

#include <klocale.h>
#include <kprocess.h>
#include <kaudioplayer.h>

#include "event.h"

#include "koeventviewer.h"

#include "alarmdialog.h"
#include "alarmdialog.moc"

AlarmDialog::AlarmDialog(QWidget *parent,const char *name)
  : KDialogBase(parent,name,false,i18n("Alarm"),Ok|User1,Ok,false,
                i18n("Suspend"))
{
  QVBox *topBox = new QVBox(this);
  topBox->setSpacing(spacingHint());
  setMainWidget(topBox);

  (void)new QLabel(i18n("The following events triggered alarms:"),topBox);

  mEventViewer = new KOEventViewer(topBox);

  QHBox *suspendBox = new QHBox(topBox);
  suspendBox->setSpacing(spacingHint());
  
  (void)new QLabel(i18n("Suspend duration (minutes):"),suspendBox);
  mSuspendSpin = new QSpinBox(1,60,1,suspendBox);
  mSuspendSpin->setValue(5);  // default suspend duration

  setMinimumSize(300,200);
}

AlarmDialog::~AlarmDialog()
{
}

void AlarmDialog::appendEvent(Event *event)
{
  mEventViewer->appendEvent(event);
  mEvents.append(event);
}

void AlarmDialog::clearEvents()
{
  mEventViewer->clearEvents();
  mEvents.clear();
}

void AlarmDialog::slotOk()
{
  clearEvents();
  accept();
}

void AlarmDialog::slotUser1()
{
  emit suspendSignal(mSuspendSpin->value());
  accept();
}

void AlarmDialog::eventNotification()
{
  Event *anEvent;

  for (anEvent = mEvents.first(); anEvent;
      anEvent = mEvents.next()) {
    if (!anEvent->getProgramAlarmFile().isEmpty()) {
      KProcess proc;
      proc << anEvent->getProgramAlarmFile().latin1();
      proc.start(KProcess::DontCare);
    }

    if (!anEvent->getAudioAlarmFile().isEmpty()) {
      KAudioPlayer::play(anEvent->getAudioAlarmFile().latin1());
    }
  }
}
