// $Id$

#include <stdio.h>
#include <qdatetime.h>
#include "dateedit.h"
#include "dateedit.moc"

DateEdit::DateEdit(QWidget *parent, int default_m, int default_y,
		   int time_window, const char *name)
  : QLineEdit(parent,name)
{
  if(name!=NULL)
    {
      char namebuf[100];
      sprintf(namebuf,"DateValidator of %s",name);
      dvalidator=new DateValidator(this,default_m,default_y,time_window,namebuf);
    }
  else
    dvalidator=new DateValidator(this,default_m,default_y,time_window);

  setValidator(dvalidator);

  lastValue=NULL;

  setMaxLength(12); // 8 digits, 2 separators, 2 spaces

  connect(this,SIGNAL(textChanged(const QString &)),
	  this,SLOT(catchTextChanged(const QString &)));
}

void DateEdit::lookNice(void)
{
  QString str(text());
  setText((dvalidator->reFormat(str)));
};

void DateEdit::setValidator(QValidator *v)
{
  QLineEdit::setValidator(v);
}


const QDate *DateEdit::date(void)
{
  return lastValue;
}


DateValidator::DateFormat DateEdit::currentFormat(void)
{
  return dateValidator()->format(text());
}


DateValidator *DateEdit::dateValidator(void)
{
  return dvalidator;
}



void DateEdit::setDate(const QDate &dat, DateValidator::DateFormat f, bool prior)
{
  QString str_tmp;
  DateValidator::DateFormat use_format;

  use_format=f;
  if(!prior)
    if((use_format=dateValidator()->format(text())) ==DateValidator::Unknown)
      use_format=f;

  setText(dateValidator()->reFormat(str_tmp,dat,use_format));
}


void DateEdit::setFormat(DateValidator::DateFormat f)
{
  QString str_tmp(text());
  setText(dateValidator()->reFormat(str_tmp,f));
}



void DateEdit::catchTextChanged(const QString &txt)
{
  QDate *dat=new QDate;
  QString str_tmp(txt);

  if(dateValidator()->translateToDate(str_tmp,*dat) == QValidator::Acceptable)
    {
      if(lastValue==NULL)
	{
	  lastValue=dat;
	  emit dateChanged(dat);
	}
      else
	{
	  if((*dat)==(*lastValue))
	    {
	      delete dat;
	    }
	  else
	    {
	      delete lastValue;
	      lastValue=dat;
	      emit dateChanged(dat);
	    };
	};
    }
  else
    {
      if(lastValue!=NULL)
	{
	  emit dateChanged(NULL);
	  delete lastValue;
	  lastValue=NULL;
	};
      delete dat;
    };
}
