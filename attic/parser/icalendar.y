/* 
 * Parser for KDayplan
 * this parser will parse an iCalendar object
 * some features of iCalendar are NOT YET IMPLEMENTED!!!
 * Copyright (c) 1997 Preston Brown, Yale University
 */

%token CRLF X_TOKEN ATOM QUOTED_STRING BOOLEAN INTEGER FLOAT
%token 2DIGIT 4DIGIT TEXT

%%
maybeparamlist
	: /* empty */
	| ';' paramlist
 
paramlist
	: parameter
	| paramlist ';' parameter

parameter
	: encodingparm
	| valuetypeparm
	| languageparm
	| maybeparmtype parmvalues

maybeparmtype
	: /* empty */
	| parmtype '='

encodingparm
	: "encoding" '='  encodetype

encodetype
	: "8bit"
	| "7bit"
	| "q"
	| "b"

valuetypeparm
	: "value" '=' valuetype

valuetype
	: "url"
	| "text"
	| "date"
	| "time"
	| "date_time"
	| "period"
	| "boolean"
	| "duration"
	| "integer"
	| "float"
	| "cal_address"
	| "utc_offset"
	| X_TOKEN
/*	| iana_value*/

languageparm
	: "language" '=' language

language
	: "us_EN"
/* as defined by RFC 1766, this is A HARD-CODED RULE RIGHT NOW */

parmtype
	: X_TOKEN
/*	| iana_ptype*/

parmvalues
	: parmvalue
	| parmvalues ',' parmvalue

parmvalue
	: X_TOKEN
/*	| iana_pvalue*/

value
	: url
	| TEXT
	| date
	| time
	| date_time
	| period
	| duration
	| BOOLEAN
	| INTEGER
	| FLOAT
	| cal_address
	| utc_offset
	| X_TOKEN
/*	| iana_value*/

cal_address
	: addr_spec
	| phrase '<' addr_spec '>'

addr_spec
	: local_part '@' domain

local_part
	: word local_part2

local_part2
	: /* empty */
	| local_part2 '.' word

domain
	: domain_ref domain2

domain2
	: /* empty */
	| domain2 '.' word

domain_ref
	: ATOM

word
	: ATOM
	| QUOTED_STRING

date_fullyear
	: 4DIGIT

date_month
	: 2DIGIT

date_mday
	: 2DIGIT

date
	: date_fullyear date_month date_mday

date_time
	: date 'T' time

dur_second
	: INTEGER 'S'

dur_minute
	: INTEGER 'M'

dur_hour
	: INTEGER 'H'

dur_time
	: 'T' dur_time2

dur_time2
	: dur_hour
	| dur_minute
	| dur_second

dur_week
	: INTEGER 'W'

dur_day
	: INTEGER 'D'

dur_month
	: INTEGER 'M'

dur_year
	: INTEGER 'Y'

dur_date
	: dur_date2 dur_date3

dur_date2
	: dur_day
	| dur_month
	| dur_year

dur_date3
	: /* empty */
	| dur_time

duration
	: 'P' duration2

duration2
	: dur_date
	| dur_time
	| dur_week

period_explicit
	: date_time '/' date_time

period_start
	: date_time '/' duration

period
	: period_explicit
	| period_start

time_hour
	: 2DIGIT

time_minute
	: 2DIGIT

time_second
	: 2DIGIT

time_numzone
	: time_numzone2 time_hour time_minute

time_numzone2
	: /* empty */
	| '+'
	| '-'

time_zone
	: 'Z'
	| time_numzone

time
	: time_hour time_minute time_second time2

time2
	: /* empty */
	| time_zone

url
	: "file://" TEXT
	| "http://" TEXT
	| "ftp://"  TEXT
	| "nntp://" TEXT
/* PLEASE NOTE __ IMPLEMENTION IS NOT EXACTLY TO SPEC! */

utc_offset
	: time_numzone

icalobject
	: "BEGIN" ":" "VCALENDAR" CRLF 
	icalbody 
	"END" ":" "VCALENDAR" CRLF icalobject2

icalobject2
	: /* empty */
	| icalobject

property
	: propname property2 ':' value CRLF

property2
	: /* empty */
	| ';' paramlist

propname
	: /* any properties defined by the document, fill me in */
/*	| iana_prop*/
	| X_TOKEN

icalbody
	: calprops icalbody2

icalbody2
	: component
	| icalbody2 component

calprops
	: calprops2 prodid calprops3 calprops4 calprops5 calprops6 version

calprops2
	: /* empty */
	| calscale

calprops3
	: /* empty */
	| profile

calprops4
	: /* empty */
	| profile_version

calprops5
	: /* empty */
	| source

calprops6
	: /* empty */
	| name

component
	: eventc
	| todoc
	| journalc
	| freebusyc
	| timezonec
	| component /* is this broken? yes... */

eventc
	: "BEGIN" ':' "VEVENT" CRLF
	eventc2 eventc3
	"END" ':' "VEVENT" CRLF

eventc2
	: /* empty */
	| eventprop
	| eventc2 eventprop

eventc3
	: /* empty */
	| alarmc
	| eventc3 alarmc

eventprop
	: attachlist attendeelist categorieslist maybeclass maybecreated
	description maybedtend dtstart exdatelist exrulelist maybegeo
	last_modlist maybelocation maybepriority mayberstatus relatedlist
	resourceslist rdatelist rrulelist dtstamp mayberesp_seq maybeseq
	maybestatus maybesummary maybetransp uid urllist mayberecurid
	commentlist

attachlist
	: /* empty */
	| attachlist attach

attendeelist
	: /* empty */
	| attendeelist attendee

categorieslist
	: /* empty */
	| categorieslist categories

maybeclass
	: /* empty */
	| class

maybecreated
	: /* empty */
	| created

maybedtend
	: /* empty */
	| dtend

exdatelist
	: /* empty */
	| exdatelist exdate

exrulelist
	: /* empty */
	| exrulelist exrule

maybegeo
	: /* empty */
	| geo

last_modlist
	: /* empty */
	| last_modlist last_mod

maybelocation
	: /* empty */
	| location

maybepriority
	: /* empty */
	| priority

mayberstatus
	: /* empty */
	| rstatus

relatedlist
	: /* empty */
	| relatedlist related

resourceslist
	: /* empty */
	| resourceslist resources

rdatelist
	: /* empty */
	| rdatelist rdate

rrulelist
	: /* empty */
	| rrulelist rrule

mayberesp_seq
	: /* empty */
	| resp_seq

maybeseq
	: /* empty */
	| seq

maybestatus
	: /* empty */
	| status

maybesummary
	: /* empty */
	| summary

maybetransp
	: /* empty */
	| transp

urllist
	: /* empty */
	| urllist url

mayberecurid
	: /* empty */
	| recurid

commentlist
	: /* empty */
	| commentlist comment

todoc
	: "BEGIN" ':' "VTODO" CRLF
	todoproplist alarmclist
	"END" ':' "VTODO" CRLF

todoproplist
	: /* empty */
	| todoproplist todoprop

alarmclist
	: /* empty */
	| alarmclist alarmc

todoprop
	: attachlist attendeelist categorieslist maybeclass maybecompleted
	maybecreated description dtstamp dtstart due exdatelist exrulelist
	maybegeo last_modlist maybelocation priority mayberstatus
	relatedlist resourceslist rdatelist rrulelist mayberesp_seq
	mayberecurid maybeseq maybestatus maybesummary maybetransp
	uid urllist commentlist

journalc
	: "BEGIN" ':' "VJOURNAL" CRLF
	jourproplist
	"END" ':' "VJOURNAL" CRLF

jourproplist
	: /* empty */
	| jourproplist jourprop

jourprop
	: attachlist attendeelist categorieslist maybeclass maybecreated
	description dtstart dtstamp last_modlist relatedlist mayberdate
	mayberrule mayberstatus mayberesp_seq maybeseq uid urllist
	mayberecurid commentlist

freebusyc
	: "BEGIN" ':' "VFREEBUSY" CRLF
	fbproplist
	"END" ':' "VFREEBUSY" CRLF

fbproplist
	: /* empty */
	| fbproplist fbprop

fbprop
	: attendeelist maybecreated maybeduration maybedtend maybedtstart
	dtstamp freebusylist last_modlist relatedlist mayberstatus
	mayberesp_seq maybeseq uid urllist maybecomment

alarmc
	: "BEGIN" ':' "VALARM" CRLF
	alarmproplist
	"END" ':' "VALARM" CRLF

alarmproplist
	: /* empty */
	| alarmproplist alarmprop

alarmprop
	: attachlist maybecreated maybedescription dtstart alarmprop2
	relatedlist repeat maybesummary commentlist

alarmprop2
	: durationprop
	| last_modlist

timezonec
	: "BEGIN" ':' "VTIMEZONE" CRLF
	tzproplist
	"END" ':' "VTIMEZONE" CRLF

tzproplist
	: /* empty */
	| tzproplist tzprop

tzprop
	: maybecreated maybedaylight dtstart tzprop2
	maybetzname tzoffset commentlist

tzprop2
	: mayberdate
	| mayberrule

/* calendar properties */

calscale
	: "CALSCALE" ':' calvalue CRLF

calvalue
	: "GREGORIAN"
/*	| iana_scale*/

prodid
	: "prodid" ':' pidvalue CRLF

pidvalue
	: TEXT /* any text which describes the product and version
		and that is generally assured of being unique */

profile
	: "PROFILE" ':' profvalue CRLF

profvalue
	: component '-' action

component
	: "EVENT"
	| "TODO"
	| "JOURNAL"
	| "FREEBUSY"
/*	| iana_component*/
	| X_TOKEN

action
	: /* any IANA registered iCalendar action type, fix me */

prof_version
	: "PROFILE_VERSION" ':' profvalue CRLF

profvalue
	: iana_prfver

iana_prfver
	: max_prfver
	| min_prfver ';' max_prfver

min_prfver
	: /* A IANA registered iCalendar profile identifier */

max_prfver
	: /* A IANA registered iCalendar profile identifier */

source
	: "SOURCE" ':' url CRLF

name
	: "NAME" ':' TEXT CRLF

version
	: "VERSION" ':' vervalue CRLF

vervalue
	: "2.0"
	| maxver
	| minver ';' maxver

minver
	: /* A IANA REGISTERED iCalendar profile id */

maxver
	: /* A IANA registered iCalendar profile id */

/* component properties */
attach
	: "ATTACH" ':' url CRLF

attendee
	: "ATTENDEE" attendee2 ':' attendee3 CRLF

attendee2
	: /* empty */
	| ';' attparmlist

attendee3
	: cal_address
	| url

attparmlist
	: attparam
	| attparmlist ';' attparam
	| paramlist
	| paramlist ';' attparam
	| paramlist ';' attparmlist ';' attparam

attparam
	: typeparm
	| roleparm
	| statusparm
	| rsvpparm
	| expectparm
	| memberparm
	| deletoparm
	| delefromparm

typeparm
	: "TYPE" '=' typeparm2

typeparm2
	: "INDIVIDUAL"
	| "GROUP"
	| "RESOURCE"
	| "ROOM"
	| "UNKNOWN"

roleparm
	: "ROLE" '=' roleparm2

roleparm2
	: "ATTENDEE"
	| "OWNER"
	| "ORGANIZER"
	| "DELEGATE"
	/* default is attendee */

statusparm
	: "STATUS" '=' statusparm2

statusparm2
	: "NEEDS-ACTION"
	| "ACCEPTED"
	| "DECLINED"
	| "TENTATIVE"
	| "COMPLETED"
	| "DELEGATED"
	| "CANCELED"
	/* default is needs-action */

rsvpparm
	: "RSVP" '=' rsvpparm2

rsvpparm2
	: "TRUE"
	| "FALSE"

expectparm
	: "EXPECT" '=' expectparm2

expectparm2
	: "FYI"
	| "REQUIRE"
	| "REQUEST"

memberparm
	: "MEMBER" '=' cal_address

deletoparm
	: "DELEGATED-TO" '=' cal_address

delefromparm
	: "DELEGATED-FROM" '=' cal_address

/* categories */
categories
	: "CATEGORIES" categories2 ':' catvalue CRLF

categories2
	: /* empty */
	| ';' paramlist

catvalue
	: cat1value catvalue1
	| cat2value catvalue2

catvalue1
	: /* empty */
	| catvalue1 ',' cat1value

catvalue2
	: /* empty */
	| catvalue2 ',' cat2value

cat1value
	: "APPOINTMENT"
	| "BUSINESS"
	| "EDUCATION"
	| "HOLIDAY"
	| "MEETING"
	| "MISCELLANEOUS"
	| "NON-WORKING HOURS"
	| "NOT IN OFFICE"
	| "PERSONAL"
	| "PHONE CALL"
	| "SICK DAY"
	| "SPECIAL OCCASION"
	| "TRAVEL"
	| "VACATION"
	| word

cat2value
	: "AUDIO"
	| "DISPLAY"
	| "EMAIL"
	| "PROCEDURE"
	| X_TOKEN
/*	| iana_word*/

/* classification */
class
	: "CLASS" class2 ':' classvalue CRLF

class2
	: /* empty */
	| ';' paramlist

classvalue
	: "PUBLIC"
	| "PRIVATE"
	| "CONFIDENTIAL"
	| X_TOKEN

comment
	: "COMMENT" ':' TEXT CRLF

completed
	: "COMPLETED" ':' date_time CRLF

created
	: "CREATED" ':' date_time CRLF

due
	: "DUE" ':' due2 CRLF

due2
	: date_time
	| duration

dtend
	: "DTEND" ':' date_time CRLF

dtstamp
	: "DTSTAMP" ':' date_time CRLF

dtstart
	: "DTSTART" ':' dtstart2 CRLF

dtstart2
	: date_time
	| date

daylight
	: "DAYLIGHT" ':' BOOLEAN CRLF

description
	: "DESCRIPTION" description2 ':' TEXT CRLF

description2
	: /* empty */
	| ';' paramlist

durationprop
	: "DURATION" ':' duration CRLF

exdate
	: "EXDATE" ':' date_time exdate2 CRLF

exdate2
	: /* empty */
	| exdate2 ',' date_time

exrule
	: "EXRULE" exrule2 ':' rvalue CRLF

exrule2
	: /* empty */
	| ';' paramlist

freebusy
	: "FREEBUSY" freebusy2 ':' fbvalue CRLF

freebusy2
	: /* empty */
	| ';' fbparmlist

fbparmlist
	: fbparam
	| paramlist ';' fbparam
	| fbparam ';' fbparmlist

fbparam
	: fbtype
	| fbstatus

fbtype
	: "TYPE" '=' fbtype2

fbtype2
	: "FREE"
	| "BUSY"

fbstatus
	: "STATUS" '=' fbstatus2

fbstatus2
	: "BUSY"
	| "OUT"
	| "PRIVATE"
	| "CONFIDENTIAL"

fbvalue
	: period fbvalue2

fbvalue2
	: /* empty */
	| fbvalue2 ',' period

geo
	: "GEO" ':' geovalue CRLF

geovalue
	: FLOAT ';' FLOAT

last_mod
	: "LAST-MODIFIED" ':' date_time last_mod2 CRLF

last_mod2
	: /* empty */
	| ',' date_time

location
	: "LOCATION" location2 ':' locavalue

location2
	: /* empty */
	| ';' paramlist

locavalue
	: TEXT
	| url

priority
	: "PRIORITY" ':' INTEGER CRLF

rdate
	: "RDATE" ':' rdvalue rdate2 CRLF

rdate2
	: /* empty */
	| rdate2 ',' rdvalue

rdvalue
	: date_time
	| period

recurid
	: "RECURRENCE-ID" recurid2 ':' date_time CRLF

recurid2
	: /* empty */
	| ';' rangeparm

rangeparm
	: "RANGE" '=' rangeparm2

rangeparm2
	: "THISANDPRIOR"
	| "THISANDFUTURE"

rrule
	: "RRULE" maybeparamlist ':' rvalue CRLF

rvalue
	: "FREQ" '=' freq rvalue2

rvalue2
	: /* empty */
	| rvalue2 "UNTIL" '=' enddate
	| rvalue2 "COUNT" '=' interval
	| rvalue2 "INTERVAL" '=' rinterval
	| rvalue2 "BYDAY" '=' bdweekdaylist
	| rvalue2 "BYMONTHDAY" '=' bmdaylist
	| rvalue2 "BYYEARDAY" '=' bydaylist
	| rvalue2 "BYSETPOS" '=' bsplist
	| rvalue2 "BYWEEKNO" '=' bwdaylist
	| rvalue2 "BYMONTH" '=' bmlist
	| rvalue2 "WKST" '=' weekday
	| rvalue2 "X-" word '=' word

freq
	: "HOURLY"
	| "DAILY"
	| "WEEKLY"
	| "YEARLY"

rinterval
	: interval
	: duration /* only for rvalue = HOURLY!!! */

interval
	: DIGIT
	| interval DIGIT

enddate
	: date /* a UTC value */

ordmoday
	: DIGIT 
	| DIGIT DIGIT

ordwk
	: DIGIT
	| DIGIT DIGIT

ordyrday
	: DIGIT
	| DIGIT DIGIT
	| DIGIT DIGIT DIGIT

daynumber
	: daynumber2 ordmoday

daynumber2
	: '+'
	| '-'

weekday
	: "SU"
	| "MO"
	| "TU"
	| "WE"
	| "TH"
	| "FR"
	| "SA"

dbweekdaynum
	: dbweekdaynum2 weekday

dbweekdaynum2
	: /* empty */
	| daynumber

bdweekdaylist
	: bdweekdaynum
	| bdweekdaynum ',' bdweekdaylist2

bdweekdaylist2
	: /* empty */
	| bdweekdaylist2 bdweekdaynum /* check these two definitions */

bmposday
	: maybeplus ordmoday

maybeplus
	: /* empty */
	| '+'

bmnegday
	: '-' ordmoday

bmdaylist
	: bmposday bmdaylist2
	| bmnegday bmdaylist3

bmdaylist2
	: /* empty */
	| ',' bmdaylist4

bmdaylist3
	: /* empty */
	| ',' bmdaylist4

bmdaylist4
	: bmposday
	| bmnegday

byposday
	: maybeplus ordyrday

bynegday
	: '-' ordyrday

bydaylist
	: byposday bydaylist2
	| bynegday bydaylist2

bydaylist2
	: /* empty */
	| bwdaylist2 ',' bydaylist3

bydaylist3
	: bynegday
	| byposday

bsplist
	: bydaylist

bwposday
	: maybeplus ordwk

bwnegday
	: '-' ordwk

bwdaylist
	: bwposday bwdaylist2
	| bwnegday bwdaylist2

bwdaylist2
	: /* empty */
	| bwdaylist2 ',' bwdaylist3

bwdaylist3
	: bwposday
	| bwnegday

bmposmon
	: DIGIT
	| DIGIT DIGIT

bmlist
	: bmposmon bmlist2

bmlist2
	: /* empty */
	| bmlist2 ',' bmposmon

related
	: "RELATED-TO" related2 ':' relvalue CRLF

related2
	: /* empty */
	| ';' paramlist

relvalue
	: TEXT

repeatcnt
	: "REPEAT" ':' INTEGER CRLF

rstatus
	: "REQUEST-STATUS" ':' statcode ';' statdesc rstatus2 CRLF

rstatus2
	: /* empty */
	| ';' extdata

statcode
	: DIGIT DIGIT DIGIT
	| statcode DIGIT

statdesc
	: TEXT

extdata
	: TEXT

resource
	: "RESOURCES" resource2 ':' resvalist CRLF

resource2
	: /* empty */
	| ';' paramlist

resvalist
	: resvalue
	| resvalist ',' resvalue

resvalue
	: "CATERING"
	| "CHAIRS"
	| "COMPUTER PROJECTOR"
	| "EASEL"
	| "OVERHEAD PROJECTOR"
	| "SPEAKER PHONE"
	| "TABLE"
	| "TV"
	| "VCR"
	| "VIDEO PHONE"
	| "VEHICLE"
	| word

respseq
	: "RESPONSE-SEQUENCE" ':' INTEGER CRLF

sequence
	: "SEQUENCE" ':' INTEGER CRLF

status
	: "STATUS" status2 ':' statvalue CRLF

status2
	: /* empty */
	| ';' paramlist

statvalue
	: "NEEDS ACTION"
	| "COMPLETED"
	| "TENTATIVE"
	| "CONFIRMED"
	| "CANCELLED"

summary
	: "SUMMARY" summary2 ':' TEXT CRLF

summary2
	: /* empty */
	| ';' paramlist

transp
	: "TRANSP" transp2 ':' transvalue CRLF

transp2
	: /* empty */
	| ';' paramlist

transvalue
	: "BUSY"
	| "OUT"
	| "PRIVATE"
	| "CONFIDENTIAL"
	| "TRANSPARENT"

tzname
	: "TZNAME" tzname2 ':' TEXT CRLF

tzname2
	: /* empty */
	| ';' paramlist

tzoffset
	: "TZOFFSET" ':' utc_offset CRLF

urlprop
	: "URL" ':' url CRLF

uid
	: "UID" uid2 ':' TEXT CRLF

uid2
	: /* empty */
	| ';' paramlist


