%{ 
/* Scanner for KDayplan
 * Copyright (c) 1997 Preston Brown, Yale University
 * preston.brown@yale.edu
*/

#include <string.h>
#include <stdlib.h>

char *putSymbol(char *_name);
char *putString(char *_name);

%}

CHAR		[\001-\177]
SPACE		" "
HTAB		\t
LWSP		({SPACE}|{HTAB})+
CRLF		\r\n

LWSPFOLD	(\r\n)(" "|\t)
SPECIALS	"("|")"|"<"|">"|"@"|","|";"|":"|"\"|<">|"."|"["|"]"
NOTSPECIAL	"!"|"#"|"$"|"%"|"&"|"'"|"*"|"+"|"-"|"/"|"="|"?"|"^"|"_"|"`"|"{"|"|"|"}"|"~"

ATOM		({NOTSPECIAL}|[0-9a-zA-Z])+

INTEGER		("+"|"-")*{DIGIT}+
FLOAT		("+"|"-")*{DIGIT}*"."([0-9])+

QUOTED_PAIR	"\"{CHAR}
QTEXT		[\000-\012\014-\041\043-\133\135-177]
QUOTED_STRING	"\""({QTEXT}|{QUOTED_PAIR})*"\""
2DIGIT		[0-9][0-9]
4DIGIT		[0-9][0-9][0-9][0-9]

%s value
%%
(X|x)"-"{ATOM}	{ yylval.name = putSymbol(yytext); return X_TOKEN; }
:			{ BEGIN value; return COLON; }

<value>{ATOM}		{ yylval.name = putSymbol(yytext); return ATOM; }
<value> /* do one for quoted string here */
<value>TRUE|true	{ yylval.vali = 1; return BOOLEAN; }
<value>FALSE|false	{ yylval.vali = 0; return BOOLEAN; }
<value>{2DIGIT}		{ yylval.vali = atoi(yytext); return 2DIGIT; }
<value>{4DIGIT}		{ yylval.vali = atoi(yytext); return 4DIGIT; }
<value>{INTEGER}		{ yylval.vali = atoi(yytext); return INTEGER; }
<value>{FLOAT}	{ yylval.vald = atof(yytext); return FLOAT; }
<value>{QUOTED_STRING}	{ yylval.vali = putSymbol(yytext); return QUOTED_STRING;
<value>{
{LWSPFOLD}		/* eat it */
{CRLF}			{ BEGIN(0); return CRLF; }

%%

char *putSymbol(char *_name)
{
  char *p = (char*)malloc(strlen(_name)+1);
  strcpy(p, _name);
  return p;
}
