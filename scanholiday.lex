%{
#include <string.h>
#include "parseholiday.h"

#undef yywrap
int yywrap(void) { return(1); }
%}

%%

#.*\n				;
^:.*\n				;
[ \t\n]				;
[-+*/%.!?:()\[\]]		{ return(*yytext); }

january				{ yylval.ival =   1; return(MONTH); }
february			{ yylval.ival =   2; return(MONTH); }
march				{ yylval.ival =   3; return(MONTH); }
april				{ yylval.ival =   4; return(MONTH); }
may				{ yylval.ival =   5; return(MONTH); }
june				{ yylval.ival =   6; return(MONTH); }
july				{ yylval.ival =   7; return(MONTH); }
august				{ yylval.ival =   8; return(MONTH); }
september			{ yylval.ival =   9; return(MONTH); }
october				{ yylval.ival =  10; return(MONTH); }
november			{ yylval.ival =  11; return(MONTH); }
december			{ yylval.ival =  12; return(MONTH); }

monday				{ yylval.ival =   1; return(WDAY); }
tuesday				{ yylval.ival =   2; return(WDAY); }
wednesday			{ yylval.ival =   3; return(WDAY); }
thursday			{ yylval.ival =   4; return(WDAY); }
friday				{ yylval.ival =   5; return(WDAY); }
saturday			{ yylval.ival =   6; return(WDAY); }
sunday				{ yylval.ival =   7; return(WDAY); }

1st|first			{ yylval.ival =   1; return(NUMBER); }
2nd|second			{ yylval.ival =   2; return(NUMBER); }
3rd|third			{ yylval.ival =   3; return(NUMBER); }
4th|fourth			{ yylval.ival =   4; return(NUMBER); }
5th|fifth			{ yylval.ival =   5; return(NUMBER); }
last				{ yylval.ival = 999; return(NUMBER); }
any				{ yylval.ival =   0; return(NUMBER); }

before				{ yylval.ival =  -1; return(NUMBER); }
after				{ yylval.ival =  -2; return(NUMBER); }

[0-9]+				{ yylval.ival = atoi(yytext); return(NUMBER); }

\"[^"]*\"			{ yylval.sval = strdup(yytext+1);
				  yylval.sval[strlen(yylval.sval)-1] = 0;
				  return(STRING); }

every				;
day				;
days				;
on				;
in				{ return(IN);    }
plus				{ return(PLUS);  }
minus				{ return(MINUS); }
small				{ return(SMALL); }
year				{ return(CYEAR);  }
leapyear			{ return(LEAPYEAR); }
easter				{ return(EASTER); }
length				{ return(LENGTH); }

black				{ yylval.ival = 1; return(COLOR); }
red				{ yylval.ival = 2; return(COLOR); }
green				{ yylval.ival = 3; return(COLOR); }
yellow				{ yylval.ival = 4; return(COLOR); }
blue				{ yylval.ival = 5; return(COLOR); }
magenta				{ yylval.ival = 6; return(COLOR); }
cyan				{ yylval.ival = 7; return(COLOR); }
white				{ yylval.ival = 8; return(COLOR); }
weekend				{ yylval.ival = 9; return(COLOR); }

"=="				{ return(EQ); }
"!="				{ return(NE); }
"<="				{ return(LE); }
">="				{ return(GE); }
"<"				{ return(LT); }
">"				{ return(GT); }
"&&"				{ return(AND);}
"||"				{ return(OR); }

.				{ printf("holiday: bad char: %s \n", yytext); }

%%
