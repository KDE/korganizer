
// A barebones mail client designed to grow as we need it
// Copyright (c) 1998 Barry D Benowitz
// $Id$

#include <unistd.h>
#include <qmsgbox.h>

#include "komailclient.h"
#include "komailclient.moc"
#include <kconfig.h>
#include <kdated.h>
#include "version.h"

MailMsgString::MailMsgString()
{
    numberOfAddresses=0;
    xMailer = new QString(XMAILER);
    xMailer->append(korgVersion);
    Addressee= new QString("To:   ");
    From = new QString("From: ");

    Subject = new QString("Subject: NOTICE OF MEETING");
    Headers = new QString();
    textString = new QString();	
}

MailMsgString::~MailMsgString()
{
    delete xMailer;
    delete Addressee;
    delete From;
    delete Subject;
    delete Headers;
    delete textString;
}

void MailMsgString::addAddressee(Attendee *newAddressee)
{
    if(numberOfAddresses != 0){
	Addressee->append(COMMA);
    }

    QString addr;
    addr = newAddressee->getEmail().copy();
    if (!newAddressee->getName().isEmpty()) {
      addr.prepend(" <");
      addr.append(">");
      addr.prepend(newAddressee->getName().data());
    }
    Addressee->append(addr.data());
    numberOfAddresses++;
}

void MailMsgString::addFrom(const char  * from)
{
    From->append(from);
}

void MailMsgString::buildTextMsg(KOEvent * selectedEvent)
{
    QString CR;
    QString recurrence[]= {"None","Daily","Weekly","Monthly Same Day","Monthly Same Position","Yearly","Yearly"};
	
    CR.sprintf("\n");
    
    if (selectedEvent->getOrganizer() != "") {
	textString->append("Organizer: ");
	textString->append(selectedEvent->getOrganizer());
	textString->append(CR);
    }

    if (selectedEvent->doesFloat())
	textString->sprintf("\nSummary: %s",
			    selectedEvent->getSummary().data());
    else {
	textString->append(CR);
	textString->append("Summary: ");
	textString->append(selectedEvent->getSummary());
	textString->append(CR);
	textString->append("Start Date: ");
	textString->append(selectedEvent->getDtStartDateStr());
	textString->append(CR);
	textString->append("Start Time: ");
	textString->append(selectedEvent->getDtStartTimeStr());
	textString->append(CR);
	if(selectedEvent->doesRecur()){
	    textString->append("Recurs: ");
	    debug("%d",selectedEvent->getRecursFrequency());
	    textString->append(recurrence[selectedEvent->getRecursFrequency()]);
	    textString->append(CR);
	    if(selectedEvent->getRecursDuration() > 0 ){
		textString->append("Repeats ");
		QString tmpStr;
		tmpStr.setNum(selectedEvent->getRecursDuration());
		textString->append(tmpStr);
		textString->append(" times");
		textString->append(CR);
	   
	    }else {
		if(selectedEvent->getRecursDuration() != -1){
		    textString->append("End Date  :");
		    textString->append(selectedEvent->getRecursEndDateStr());
		    textString->append(CR);
		}else {
		    textString->append("Repeats forever");
		    textString->append(CR);
		}
	    }
	}
	textString->append("End Time  : ");
	textString->append(selectedEvent->getDtEndTimeStr());
	textString->append(CR);
	
    } 
}

QString * MailMsgString::getHeaders()
{
    QString CR,date;
    QDateTime theDate(QDate::currentDate(),QTime::currentTime());
    CR.sprintf("\n");
    date.sprintf("Date: ");

    Headers->append(Addressee->data());
    Headers->append(CR);
    Headers->append(From->data());
    Headers->append(CR);
    Headers->append(Subject->data());
    Headers->append(CR);
    Headers->append(date);
    Headers->append(theDate.toString());
    Headers->append(CR);
    Headers->append(xMailer->data());
    Headers->append(CR);
  

    return Headers;
}

KoMailClient::KoMailClient(CalObject *cal)
{   
    calendar = cal;
}

KoMailClient::~KoMailClient()
{
}

void KoMailClient::emailEvent(KOEvent *selectedEvent,QWidget * parent)
{
#include <stdio.h>
    FILE *sendMail;;
    int numAttendees=0;
    QString sender;
    MailMsgString headers;
    QList<Attendee> Participants;
    QListIterator<Attendee> it(Participants);
    bool      dirty = false;

//
// Generate List of Addressees
    Participants=selectedEvent->getAttendeeList();
    it.toFirst();
    for(;numAttendees < selectedEvent->attendeeCount();numAttendees++){
        if (it.current()->getStatus() == it.current()->NEEDS_ACTION) {
          headers.addAddressee(it.current());
          dirty = true;
        }

        ++it;	
    }

    if (! dirty)
        return;  // no recips were in NEEDS_ACTION status - bail out

    sender=calendar->getEmail();
    headers.addFrom(sender.data());// Apptmt always from Owner
//    debug("%s",headers.getHeaders()->data());
    headers.buildTextMsg(selectedEvent);
//    debug("%s",headers.getBody());

    // something needs to be done about this...
    // maybe just needs to be user configurable
    sendMail = popen("sendmail -t","w"); // doesn't work unless you're root (?)
    if( sendMail == NULL ){
	debug("Can't fork sendmail!");
	return;
    }
    ::fprintf(sendMail,"%s%s\n.\n",headers.getHeaders()->data(), 
	    headers.getBody()->data());
    ::pclose(sendMail);

    if (selectedEvent->isReadOnly())
        return;

    // at this point we assume that all recipiants have been notified.
    // so set all the recip status and event status to SENT.
    // then set the dirty bit(s)

    // update the status on all attendees to SENT
    it.toFirst();
    numAttendees = 0;
    for(;numAttendees < selectedEvent->attendeeCount();numAttendees++){
        if (it.current()->getStatus() == it.current()->NEEDS_ACTION)
          it.current()->setStatus(it.current()->SENT);

        ++it;	
    }

    // update the status on the event object
    if (selectedEvent->getStatus() == selectedEvent->NEEDS_ACTION)
      selectedEvent->setStatus(selectedEvent->SENT);

    return;
}
