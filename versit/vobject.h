/***************************************************************************
(C) Copyright 1996 Apple Computer, Inc., AT&T Corp., International             
Business Machines Corporation and Siemens Rolm Communications Inc.             
                                                                               
For purposes of this license notice, the term Licensors shall mean,            
collectively, Apple Computer, Inc., AT&T Corp., International                  
Business Machines Corporation and Siemens Rolm Communications Inc.             
The term Licensor shall mean any of the Licensors.                             
                                                                               
Subject to acceptance of the following conditions, permission is hereby        
granted by Licensors without the need for written agreement and without        
license or royalty fees, to use, copy, modify and distribute this              
software for any purpose.                                                      
                                                                               
The above copyright notice and the following four paragraphs must be           
reproduced in all copies of this software and any software including           
this software.                                                                 
                                                                               
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS AND NO LICENSOR SHALL HAVE       
ANY OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS OR       
MODIFICATIONS.                                                                 
                                                                               
IN NO EVENT SHALL ANY LICENSOR BE LIABLE TO ANY PARTY FOR DIRECT,              
INDIRECT, SPECIAL OR CONSEQUENTIAL DAMAGES OR LOST PROFITS ARISING OUT         
OF THE USE OF THIS SOFTWARE EVEN IF ADVISED OF THE POSSIBILITY OF SUCH         
DAMAGE.                                                                        
                                                                               
EACH LICENSOR SPECIFICALLY DISCLAIMS ANY WARRANTIES, EXPRESS OR IMPLIED,       
INCLUDING BUT NOT LIMITED TO ANY WARRANTY OF NONINFRINGEMENT OR THE            
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR             
PURPOSE.                                                                       

The software is provided with RESTRICTED RIGHTS.  Use, duplication, or         
disclosure by the government are subject to restrictions set forth in          
DFARS 252.227-7013 or 48 CFR 52.227-19, as applicable.                         

***************************************************************************/

/*
 
The vCard/vCalendar C interface is implemented in the set 
of files as follows:

vcc.y, yacc source, and vcc.c, the yacc output you will use
implements the core parser

vobject.c implements an API that insulates the caller from
the parser and changes in the vCard/vCalendar BNF

port.h defines compilation environment dependent stuff

vcc.h and vobject.h are header files for their .c counterparts

vcaltmp.h and vcaltmp.c implement vCalendar "macro" functions
which you may find useful.

test.c is a standalone test driver that exercises some of
the features of the APIs provided. Invoke test.exe on a
VCARD/VCALENDAR input text file and you will see the pretty
print output of the internal representation (this pretty print
output should give you a good idea of how the internal 
representation looks like -- there is one such output in the 
following too). Also, a file with the .out suffix is generated 
to show that the internal representation can be written back 
in the original text format.

For more information on this API see the readme.txt file
which accompanied this distribution.

  Also visit:

		http://www.versit.com
		http://www.ralden.com

*/


#ifndef __VOBJECT_H__
#define __VOBJECT_H__ 1


#include "port.h"
#include <stdlib.h>
#include <stdio.h>

#if defined(__CPLUSPLUS__) || defined(__cplusplus)
extern "C" {
#endif


#define VC7bitProp				"7BIT"
#define VC8bitProp				"8BIT"
#define VCAAlarmProp			"AALARM"
#define VCAdditionalNamesProp	"ADDN"
#define VCAdrProp				"ADR"
#define VCAgentProp				"AGENT"
#define VCAIFFProp				"AIFF"
#define VCAOLProp				"AOL"
#define VCAppleLinkProp			"APPLELINK"
#define VCAttachProp			"ATTACH"
#define VCAttendeeProp			"ATTENDEE"
#define VCATTMailProp			"ATTMAIL"
#define VCAudioContentProp		"AUDIOCONTENT"
#define VCAVIProp				"AVI"
#define VCBase64Prop			"BASE64"
#define VCBBSProp				"BBS"
#define VCBirthDateProp			"BDAY"
#define VCBMPProp				"BMP"
#define VCBodyProp				"BODY"
#define VCBusinessRoleProp		"ROLE"
#define VCCalProp				"VCALENDAR"
#define VCCaptionProp			"CAP"
#define VCCardProp				"VCARD"
#define VCCarProp				"CAR"
#define VCCategoriesProp		"CATEGORIES"
#define VCCellularProp			"CELL"
#define VCCGMProp				"CGM"
#define VCCharSetProp			"CS"
#define VCCIDProp				"CID"
#define VCCISProp				"CIS"
#define VCCityProp				"L"
#define VCClassProp				"CLASS"
#define VCCommentProp			"NOTE"
#define VCCompletedProp			"COMPLETED"
#define VCContentIDProp			"CONTENT-ID"
#define VCCountryNameProp		"C"
#define VCDAlarmProp			"DALARM"
#define VCDataSizeProp			"DATASIZE"
#define VCDayLightProp			"DAYLIGHT"
#define VCDCreatedProp			"DCREATED"
#define VCDeliveryLabelProp     "LABEL"
#define VCDescriptionProp		"DESCRIPTION"
#define VCDIBProp				"DIB"
#define VCDisplayStringProp		"DISPLAYSTRING"
#define VCDomesticProp			"DOM"
#define VCDTendProp				"DTEND"
#define VCDTstartProp			"DTSTART"
#define VCDueProp				"DUE"
#define VCEmailAddressProp		"EMAIL"
#define VCEncodingProp			"ENCODING"
#define VCEndProp				"END"
#define VCEventProp				"VEVENT"
#define VCEWorldProp			"EWORLD"
#define VCExNumProp				"EXNUM"
#define VCExDateProp			"EXDATE"
#define VCExpectProp			"EXPECT"
#define VCExtAddressProp		"EXT ADD"
#define VCFamilyNameProp		"F"
#define VCFaxProp				"FAX"
#define VCFullNameProp			"FN"
#define VCGeoProp				"GEO"
#define VCGeoLocationProp		"GEO"
#define VCGIFProp				"GIF"
#define VCGivenNameProp			"G"
#define VCGroupingProp			"Grouping"
#define VCHomeProp				"HOME"
#define VCIBMMailProp			"IBMMail"
#define VCInlineProp			"INLINE"
#define VCInternationalProp		"INTL"
#define VCInternetProp			"INTERNET"
#define VCISDNProp				"ISDN"
#define VCJPEGProp				"JPEG"
#define VCLanguageProp			"LANG"
#define VCLastModifiedProp		"LAST-MODIFIED"
#define VCLastRevisedProp		"REV"
#define VCLocationProp			"LOCATION"
#define VCLogoProp				"LOGO"
#define VCMailerProp			"MAILER"
#define VCMAlarmProp			"MALARM"
#define VCMCIMailProp			"MCIMAIL"
#define VCMessageProp			"MSG"
#define VCMETProp				"MET"
#define VCModemProp				"MODEM"
#define VCMPEG2Prop				"MPEG2"
#define VCMPEGProp				"MPEG"
#define VCMSNProp				"MSN"
#define VCNamePrefixesProp		"NPRE"
#define VCNameProp				"N"
#define VCNameSuffixesProp		"NSUF"
#define VCNoteProp				"NOTE"
#define VCOrgNameProp			"ORGNAME"
#define VCOrgProp				"ORG"
#define VCOrgUnit2Prop			"OUN2"
#define VCOrgUnit3Prop			"OUN3"
#define VCOrgUnit4Prop			"OUN4"
#define VCOrgUnitProp			"OUN"
#define VCPagerProp				"PAGER"
#define VCPAlarmProp			"PALARM"
#define VCParcelProp			"PARCEL"
#define VCPartProp				"PART"
#define VCPCMProp				"PCM"
#define VCPDFProp				"PDF"
#define VCPGPProp				"PGP"
#define VCPhotoProp				"PHOTO"
#define VCPICTProp				"PICT"
#define VCPMBProp				"PMB"
#define VCPostalBoxProp			"BOX"
#define VCPostalCodeProp		"PC"
#define VCPostalProp			"POSTAL"
#define VCPowerShareProp		"POWERSHARE"
#define VCPreferredProp			"PREF"
#define VCPriorityProp			"PRIORITY"
#define VCProcedureNameProp		"PROCEDURENAME"
#define VCProdIdProp			"PRODID"
#define VCProdigyProp			"PRODIGY"
#define VCPronunciationProp		"SOUND"
#define VCPSProp				"PS"
#define VCPublicKeyProp			"KEY"
#define VCQPProp				"QP"
#define VCQuickTimeProp			"QTIME"
#define VCQuotedPrintableProp	"QUOTED-PRINTABLE"
#define VCRDateProp				"RDATE"
#define VCRegionProp			"R"
#define VCRelatedToProp			"RELATED-TO"
#define VCRepeatCountProp		"REPEATCOUNT"
#define VCResourcesProp			"RESOURCES"
#define VCRNumProp				"RNUM"
#define VCRoleProp				"ROLE"
#define VCRRuleProp				"RRULE"
#define VCRSVPProp				"RSVP"
#define VCRunTimeProp			"RUNTIME"
#define VCSequenceProp			"SEQUENCE"
#define VCSnoozeTimeProp		"SNOOZETIME"
#define VCStartProp				"START"
#define VCStatusProp			"STATUS"
#define VCStreetAddressProp		"STREET"
#define VCSubTypeProp			"SUBTYPE"
#define VCSummaryProp			"SUMMARY"
#define VCTelephoneProp			"TEL"
#define VCTIFFProp				"TIFF"
#define VCTimeZoneProp			"TZ"
#define VCTitleProp				"TITLE"
#define VCTLXProp				"TLX"
#define VCTodoProp				"VTODO"
#define VCTranspProp			"TRANSP"
#define VCUniqueStringProp		"UID"
#define VCURLProp				"URL"
#define VCURLValueProp			"URLVAL"
#define VCValueProp				"VALUE"
#define VCVersionProp			"VERSION"
#define VCVideoProp				"VIDEO"
#define VCVoiceProp				"VOICE"
#define VCWAVEProp				"WAVE"
#define VCWMFProp				"WMF"
#define VCWorkProp				"WORK"
#define VCX400Prop				"X400"
#define VCX509Prop				"X509"
#define VCXRuleProp				"XRULE"

/* extensions for KOrganizer / KPilot */
#define KPilotIdProp                            "X-PILOTID"
#define KPilotStatusProp                        "X-PILOTSTAT"

/* extensions for iMIP / iTIP */
#define ICOrganizerProp                         "X-ORGANIZER"
#define ICMethodProp                            "X-METHOD"
#define ICRequestStatusProp                     "X-REQUEST-STATUS"

typedef struct VObject VObject;

typedef union ValueItem {
    const char *strs;
    const wchar_t *ustrs;
    unsigned int i;
    unsigned long l;
    void *any;
    VObject *vobj;
    } ValueItem;

struct VObject {
    VObject *next;
    const char *id;
    VObject *prop;
    unsigned short valType;
    ValueItem val;
    };

typedef struct StrItem StrItem;

struct StrItem {
    StrItem *next;
    const char *s;
    unsigned int refCnt;
    };

typedef struct VObjectIterator {
    VObject* start;
    VObject* next;
    } VObjectIterator;

extern VObject* newVObject(const char *id);
extern void deleteVObject(VObject *p);
extern char* dupStr(const char *s, unsigned int size);
extern void deleteStr(const char *p);
extern void unUseStr(const char *s);

extern void setVObjectName(VObject *o, const char* id);
extern void setVObjectStringZValue(VObject *o, const char *s);
extern void setVObjectStringZValue_(VObject *o, const char *s);
extern void setVObjectUStringZValue(VObject *o, const wchar_t *s);
extern void setVObjectUStringZValue_(VObject *o, const wchar_t *s);
extern void setVObjectIntegerValue(VObject *o, unsigned int i);
extern void setVObjectLongValue(VObject *o, unsigned long l);
extern void setVObjectAnyValue(VObject *o, void *t);
extern VObject* setValueWithSize(VObject *prop, void *val, unsigned int size);
extern VObject* setValueWithSize_(VObject *prop, void *val, unsigned int size);

extern const char* vObjectName(VObject *o);
extern const char* vObjectStringZValue(VObject *o);
extern const wchar_t* vObjectUStringZValue(VObject *o);
extern unsigned int vObjectIntegerValue(VObject *o);
extern unsigned long vObjectLongValue(VObject *o);
extern void* vObjectAnyValue(VObject *o);
extern VObject* vObjectVObjectValue(VObject *o);
extern void setVObjectVObjectValue(VObject *o, VObject *p);

extern VObject* addVObjectProp(VObject *o, VObject *p);
extern VObject* addProp(VObject *o, const char *id);
extern VObject* addProp_(VObject *o, const char *id);
extern VObject* addPropValue(VObject *o, const char *p, const char *v);
extern VObject* addPropSizedValue_(VObject *o, const char *p, const char *v, unsigned int size);
extern VObject* addPropSizedValue(VObject *o, const char *p, const char *v, unsigned int size);
extern VObject* addGroup(VObject *o, const char *g);
extern void addList(VObject **o, VObject *p);

extern VObject* isAPropertyOf(VObject *o, const char *id);

extern VObject* nextVObjectInList(VObject *o);
extern void initPropIterator(VObjectIterator *i, VObject *o);
extern int moreIteration(VObjectIterator *i);
extern VObject* nextVObject(VObjectIterator *i);

extern char* writeMemVObject(char *s, int *len, VObject *o);
extern char* writeMemVObjects(char *s, int *len, VObject *list);

extern const char* lookupStr(const char *s);
extern void cleanStrTbl();

extern void cleanVObject(VObject *o);
extern void cleanVObjects(VObject *list);

extern const char* lookupProp(const char* str);
extern const char* lookupProp_(const char* str);

extern wchar_t* fakeUnicode(const char *ps, int *bytes);
extern int uStrLen(const wchar_t *u);
extern char* fakeCString(const wchar_t *u);

extern void printVObjectToFile(char *fname,VObject *o);
extern void printVObjectsToFile(char *fname,VObject *list);
extern void writeVObjectToFile(char *fname, VObject *o);
extern void writeVObjectsToFile(char *fname, VObject *list);

extern int vObjectValueType(VObject *o);

/* return type of vObjectValueType: */
#define VCVT_NOVALUE	0
	/* if the VObject has no value associated with it. */
#define VCVT_STRINGZ	1
	/* if the VObject has value set by setVObjectStringZValue. */
#define VCVT_USTRINGZ	2
	/* if the VObject has value set by setVObjectUStringZValue. */
#define VCVT_UINT		3
	/* if the VObject has value set by setVObjectIntegerValue. */
#define VCVT_ULONG		4
	/* if the VObject has value set by setVObjectLongValue. */
#define VCVT_RAW		5
	/* if the VObject has value set by setVObjectAnyValue. */
#define VCVT_VOBJECT	6
	/* if the VObject has value set by setVObjectVObjectValue. */

extern const char** fieldedProp;

extern void printVObject(FILE *fp,VObject *o);
extern void writeVObject(FILE *fp, VObject *o);


#if defined(__CPLUSPLUS__) || defined(__cplusplus)
}
#endif

#endif /* __VOBJECT_H__ */


