// 	$Id$	

#include <stdio.h>

#include <qkeycode.h>

#include <kstddirs.h>

#include "koprefs.h"

#include "ktimed.h" 
#include "ktimed.moc"

static const char *ampmtimes[] = {
  "12:00 am", "12:15 am", "12:30 am", "12:45 am",
  " 1:00 am", " 1:15 am", " 1:30 am", " 1:45 am",
  " 2:00 am", " 2:15 am", " 2:30 am", " 2:45 am",
  " 3:00 am", " 3:15 am", " 3:30 am", " 3:45 am",
  " 4:00 am", " 4:15 am", " 4:30 am", " 4:45 am",
  " 5:00 am", " 5:15 am", " 5:30 am", " 5:45 am",
  " 6:00 am", " 6:15 am", " 6:30 am", " 6:45 am",
  " 7:00 am", " 7:15 am", " 7:30 am", " 7:45 am",
  " 8:00 am", " 8:15 am", " 8:30 am", " 8:45 am",
  " 9:00 am", " 9:15 am", " 9:30 am", " 9:45 am",
  "10:00 am", "10:15 am", "10:30 am", "10:45 am",
  "11:00 am", "11:15 am", "11:30 am", "11:45 am",
  "12:00 pm", "12:15 pm", "12:30 pm", "12:45 pm",
  " 1:00 pm", " 1:15 pm", " 1:30 pm", " 1:45 pm",
  " 2:00 pm", " 2:15 pm", " 2:30 pm", " 2:45 pm",
  " 3:00 pm", " 3:15 pm", " 3:30 pm", " 3:45 pm",
  " 4:00 pm", " 4:15 pm", " 4:30 pm", " 4:45 pm",
  " 5:00 pm", " 5:15 pm", " 5:30 pm", " 5:45 pm",
  " 6:00 pm", " 6:15 pm", " 6:30 pm", " 6:45 pm",
  " 7:00 pm", " 7:15 pm", " 7:30 pm", " 7:45 pm",
  " 8:00 pm", " 8:15 pm", " 8:30 pm", " 8:45 pm",
  " 9:00 pm", " 9:15 pm", " 9:30 pm", " 9:45 pm",
  "10:00 pm", "10:15 pm", "10:30 pm", "10:45 pm",
  "11:00 pm", "11:15 pm", "11:30 pm", "11:45 pm", 0};

static const char *miltimes[] = {
  "00:00", "00:15", "00:30", "00:45",
  "01:00", "01:15", "01:30", "01:45",
  "02:00", "02:15", "02:30", "02:45",
  "03:00", "03:15", "03:30", "03:45",
  "04:00", "04:15", "04:30", "04:45",
  "05:00", "05:15", "05:30", "05:45",
  "06:00", "06:15", "06:30", "06:45",
  "07:00", "07:15", "07:30", "07:45",
  "08:00", "08:15", "08:30", "08:45",
  "09:00", "09:15", "09:30", "09:45",
  "10:00", "10:15", "10:30", "10:45",
  "11:00", "11:15", "11:30", "11:45",
  "12:00", "12:15", "12:30", "12:45",
  "13:00", "13:15", "13:30", "13:45",
  "14:00", "14:15", "14:30", "14:45",
  "15:00", "15:15", "15:30", "15:45",
  "16:00", "16:15", "16:30", "16:45",
  "17:00", "17:15", "17:30", "17:45",
  "18:00", "18:15", "18:30", "18:45",
  "19:00", "19:15", "19:30", "19:45",
  "20:00", "20:15", "20:30", "20:45",
  "21:00", "21:15", "21:30", "21:45",
  "22:00", "22:15", "22:30", "22:45",
  "23:00", "23:15", "23:30", "23:45", 0};


KTimeEdit::KTimeEdit(QWidget *parent, QTime qt, const char *name) 
  : QComboBox(TRUE, parent, name)
{
  timeAmPm = TRUE;

  setInsertionPolicy(NoInsertion);

  //  user_inserted = 0;
  
  connect(this, SIGNAL(activated(int)), this, SLOT(activ(int)));
  connect(this, SIGNAL(highlighted(int)), this, SLOT(hilit(int)));

  time = qt;
  updateConfig();
  setFocusPolicy(QWidget::StrongFocus);
}

KTimeEdit::~KTimeEdit()
{
}

QTime KTimeEdit::getTime(bool &ok)
{
  validateEntry();
  ok = current_display_valid;
  return time;
}

QSizePolicy  KTimeEdit::sizePolicy() const
{
  // Set size policy to Fixed, because edit cannot contain more text than the
  // string representing the time. It doesn't make sense to provide more space.
  QSizePolicy sizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);

  return sizePolicy;
}

void KTimeEdit::setTime(QTime newTime)
{
  if(newTime == time) {
    // It is extremely important not to do anything in this case to
    // avoid infinite signal/slot looping...
    // but we update the display to make sure it is right.
    updateDisp();
    return;
  }
  
  time = newTime;
  updateDisp();
}

void KTimeEdit::insertTimes()
{
  clear();
  if (timeAmPm)
    insertStrList(ampmtimes);
  else
    insertStrList(miltimes);
}

void KTimeEdit::activ(int i) 
{
  int wrap = 0;
  /*  if((i != user_inserted) && (user_inserted != 0)) {
    removeItem(user_inserted);
    if(i > user_inserted) {
      // account for the deletion of an item.
      setCurrentItem(i-1);
    }
    user_inserted = 0;
    }*/

  QString s = text(i);

  int h, m;
  if (timeAmPm) {
    h = s.left(2).toInt();
    m = s.mid(3, 2).toInt();
    QString ampm = s.right(2);

    // check for time wraparound.
    if(h == 12 && ampm == "am" && time >= QTime(23,45)) {
      // time wraparound into the next day.
      wrap = 1;
    } else if(h == 11 && ampm == "pm" && time == QTime(0,0)) {
      // time wraparound into previous day.
      wrap = -1;
    }
    
    if(h == 12) {
      if(ampm == "am") {
	time.setHMS(0,m,0);
	emit timeChanged(time, wrap);
	return;
      }
      time.setHMS(12,m,0);
      emit timeChanged(time, wrap);
      return;
    } else {
      time.setHMS((ampm == "pm")? h+12 : h, m, 0);
      emit timeChanged(time, wrap);
    }
  } else {
    // we are in 24 hour format
    h = s.left(2).toInt();
    m = s.mid(3,2).toInt();

    // check for time wraparound -- not sure how this works?
    time.setHMS(h, m, 0);
    emit timeChanged(time, wrap);
    return;
  }
}

void KTimeEdit::hilit(int )
{
  // we don't currently need to do anything here.
}

void KTimeEdit::addTime(QTime qt)
{
  // Calculate the new time.
  time = qt.addSecs(time.minute()*60+time.hour()*3600);
  emit timeChanged(time, 0);
  updateDisp();
}

void KTimeEdit::subTime(QTime qt)
{
  int h, m;

  // Note that we cannot use the same method for determining the new
  // time as we did in addTime, becuase QTime does not handle adding
  // negative seconds well at all.
  h = time.hour()-qt.hour();
  m = time.minute()-qt.minute();

  if(m < 0) {
    m += 60;
    h -= 1;
  }

  if(h < 0) {
    h += 24;
  }

  // store the newly calculated time.
  time.setHMS(h, m, 0);
  emit timeChanged(time, 0);
  updateDisp();
}                  

void KTimeEdit::keyPressEvent(QKeyEvent *qke)
{
  switch(qke->key()) {
  case Key_Enter:
  case Key_Return:
    validateEntry();
    break;
  case Key_Down:
    addTime(QTime(0,15,0));
    break;
  case Key_Up:
    subTime(QTime(0,15,0));
    break;
  default:
    QComboBox::keyPressEvent(qke);
    break;
  } // switch
}

void KTimeEdit::validateEntry()
{
  QString s = currentText();
  bool ok;
  int colon_pos, space_pos, h, m;

  // make sure that the text entered has the proper format.
  s = s.simplifyWhiteSpace();
  colon_pos = s.find(':');
  if(colon_pos == -1) {
    // this is something of the form "a [am|pm]" make sure that "a" is ok.
    space_pos = s.find(' ');
    if(space_pos == -1) {
      // there is no [am|pm]. take the whole thing as "a".
      h = s.toInt(&ok);
      if(!ok) {
	QMessageBox::information(this, "KOrganizer Error", 
				 "You must specify a valid time");
	current_display_valid = FALSE;
	return;
      }
      if(h > 23) {
	QMessageBox::information(this, "KOrganizer Error", 
				 "You must specify a valid time");
	current_display_valid = FALSE;
	return;
      }
      time.setHMS(h, 0, 0);
      updateDisp();
      current_display_valid = TRUE;
      emit timeChanged(time, 0);
      focusNextPrevChild(TRUE);
      return;
    }

    // there is an [am|pm] field.
    h = s.left(space_pos).toInt(&ok);
    if(!ok) {
      QMessageBox::information(this, "KOrganizer Error", 
			       "You must specify a valid time");
      current_display_valid = FALSE;
      return;
    }
    if(h > 12) {
      QMessageBox::information(this, "KOrganizer Error", 
			       "You must specify a valid time");
      current_display_valid = FALSE;
      return;
    }
    if(s.right(2) == "am") {
      if(h == 12) {
	time.setHMS(0,0,0);
	updateDisp();
	current_display_valid = TRUE;
	emit timeChanged(time, 0);
	focusNextPrevChild(TRUE);
	return;
      }
      time.setHMS(h,0,0);
      updateDisp();
      current_display_valid = TRUE;
      emit timeChanged(time, 0);
      focusNextPrevChild(TRUE);
      return;
    }
    if(s.right(2) == "pm") {
      if(h == 12) {
	time.setHMS(12,0,0);
	updateDisp();
	current_display_valid = TRUE;
	emit timeChanged(time, 0);
	focusNextPrevChild(TRUE);
	return;
      }
      time.setHMS(h+12,0,0);
      updateDisp();
      current_display_valid = TRUE;
      emit timeChanged(time, 0);
      focusNextPrevChild(TRUE);
      return;
    }
    // if we get to here... there was an am/pm field, but it wasn't am
    // or pm... not good.
    QMessageBox::information(this, "KOrganizer Error", 
			     "You must specify a valid time");
    current_display_valid = FALSE;
    return;
  } // end of case where no colon was found.

  if(colon_pos > 2) {
    // this can't be right... colon is too far right.
    QMessageBox::information(this, "KOrganizer Error", 
			     "You must specify a valid time");
    current_display_valid = FALSE;
    return;
  }
  
  // we now know that the hour is delimited by the colon.
  h = s.left(colon_pos).toInt(&ok);
  
  // let's see what's after the colon.
  space_pos = s.find(' ');
  if(space_pos == -1) {
    // there was no space... so we're dealing with either h:m or h:mm
    m = s.right(s.length()-(colon_pos+1)).toInt(&ok);
    if(!ok) {
      QMessageBox::information(this, "KOrganizer Error", 
			       "You must specify a valid time");
      current_display_valid = FALSE;
      return;
    }
    time.setHMS(h,m,0);
    updateDisp();
    current_display_valid = TRUE;
    emit timeChanged(time, 0);
    focusNextPrevChild(TRUE);
    return;
  }
  
  // otherwise, we're dealing with something like
  //     h:m am|pm or h:mm am|pm.

  if(h > 12) {
    QMessageBox::information(this, "KOrganizer Error", 
			     "You must specify a valid time");
    current_display_valid = FALSE;
    return;
  }

  m = s.mid(colon_pos+1, (space_pos-1)-colon_pos).toInt(&ok);
  if(!ok) {
    QMessageBox::information(this, "KOrganizer Error", 
			     "You must specify a valid time");
    current_display_valid = FALSE;
    return;
  }

  if(m > 59) {
    QMessageBox::information(this, "KOrganizer Error", 
			     "You must specify a valid time");
    current_display_valid = FALSE;
    return;
  }


  if(s.right(s.length()-(space_pos+1)) == "am") {
    if(h == 12) {
      time.setHMS(0,m,0);
      updateDisp();
      current_display_valid = TRUE;
      emit timeChanged(time, 0);
      focusNextPrevChild(TRUE);
      return;
    }
    time.setHMS(h,m,0);
    updateDisp();
    current_display_valid = TRUE;
    emit timeChanged(time, 0);
    focusNextPrevChild(TRUE);
    return;
  }
  if(s.right(s.length()-(space_pos+1)) == "pm") {
    if(h == 12) {
      time.setHMS(12,m,0);
      updateDisp();
      current_display_valid = TRUE;
      emit timeChanged(time, 0);
      focusNextPrevChild(TRUE);
      return;
    }
    time.setHMS(h+12,m,0);
    updateDisp();
    current_display_valid = TRUE;
    emit timeChanged(time, 0);
    focusNextPrevChild(TRUE);
    return;
  }
  // if we get to here... there was an am/pm field, but it wasn't am
  // or pm... not good.
  QMessageBox::information(this, "KOrganizer Error", 
			   "You must specify a valid time");
  current_display_valid = FALSE;
  return;
}

void KTimeEdit::updateConfig()
{
  timeAmPm = KOPrefs::instance()->mTimeFormat;
  insertTimes();
  updateDisp();
}

void KTimeEdit::updateDisp() 
{
  QString s, ampm;
  int h;
  
  if (timeAmPm) {
    if(time.hour() == 0) {
      h = 12;
      ampm = "am";
    } else if (time.hour() >= 12) {
      if (time.hour() != 12)
	h = time.hour() - 12;
      else
	h = 12;
      ampm = "pm";
    } else {
      h = time.hour();
      ampm = "am";
    }
    
    s.sprintf("%2d:%02d %s", h, time.minute(), ampm.data());
  } else {
    s.sprintf("%02d:%02d", time.hour(), time.minute());
  }

  setEditText(s.data());

  /*  if(user_inserted) {
    removeItem(user_inserted);
    user_inserted = 0;
    }*/

  /*  if((time.minute()%15) != 0) {
    user_inserted = (time.hour()*4)+(time.minute()/15)+1;
    insertItem(s, user_inserted);
    setCurrentItem(user_inserted);
    return;
    }*/
  if (!time.minute() % 15)
    setCurrentItem((time.hour()*4)+(time.minute()/15));
}
