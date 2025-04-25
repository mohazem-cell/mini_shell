
/*
 *
 * CS-413 Spring 98
 * shell.l: lexical analyzer for shell
 * You have to extend it.
 *
 */

%{

#include <string.h>
#include "y.tab.h"

%}

%%

\n 	{
		return NEWLINE;
	}
	
"|"	{
		return PIPELINE;
	}

[ \t] 	{
		/* Discard spaces and tabs */
     	}
     	
"&"	{
		return AMPERSAND;
	}

"<"	{
		return INPUT;
	}

">" 	{
		return GREAT;
	}
">&"	{
		return GREATAMPERSAND;
	}
">>"	{
		return APPEND;
	}
">>&"	{
		return APPENDAMPERSAND;
	}
"exit"	{
		return EXIT;
	}
"cd"	{
		return CHANGEDIR;
	}
[^&><| \t\n]*  {
		/* Assume that file names have only alpha chars */
		yylval.string_val = strdup(yytext);
		return WORD;
	}

.	{
		/* Invalid character in input */
		return NOTOKEN;
	}

%%
	
