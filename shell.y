
/*
 * CS-413 Spring 98
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *	cmd [arg]* [> filename]
 *
 * you must extend it to understand the complete shell grammar
 *
 */

%token	<string_val> WORD

%token 	NOTOKEN GREAT NEWLINE PIPELINE APPEND INPUT AMPERSAND EXIT CHANGEDIR GREATAMPERSAND APPENDAMPERSAND

%union	{
		char   *string_val;
	}

%{
extern "C" 
{
	int yylex();
	void yyerror (char const *s);
}
#define yylex yylex
#include <stdio.h>
#include <unistd.h>
#include "command.h"
%}

%%

goal:	
	commands
	;

commands: 
	command
	| commands command 
	;

command: 
	simple_command
	| command_with_pipeline {
		//printf("   Yacc: Execute command with pipeline\n");
		Command::_currentCommand.execute();
	}
	| background_command NEWLINE {
		printf("   Yacc: Execute command in background\n");
		Command::_currentCommand.execute();
	}
	| EXIT NEWLINE {
		printf("\n	Goodbye!!\n\n");
		exit(0);
		
	}
	| CHANGEDIR WORD NEWLINE {
		printf("   Yacc: Change directory to %s\n", $2);
		if (chdir($2) != 0) {
			perror("chdir failed");
		}
		Command::_currentCommand.execute();
	}
	| CHANGEDIR NEWLINE {
		printf("   Yacc: Change directory to home\n");
		const char *home = getenv("HOME");
		if (home && chdir(home) != 0) {
			perror("chdir to home failed");
		}
		Command::_currentCommand.execute();
	}
	;


simple_command:	
	command_and_args iomodifier_opt NEWLINE {
		printf("   Yacc: Execute command\n");
		Command::_currentCommand.execute();
	}
	 
	| NEWLINE 
	| error NEWLINE { yyerrok; }
	;


command_and_args:
	command_word arg_list {
		Command::_currentCommand.
			insertSimpleCommand( Command::_currentSimpleCommand );
	}
	;

command_with_pipeline:
	pipeline_sequence iomodifier_opt NEWLINE{
		//printf("   Yacc: Insert pipeline command sequence with redirection\n");
		//Command::_currentCommand.execute();
		//printf("I am in pipline command\n");
		Command::_currentCommand._pipline = true;
	}
	| NEWLINE
	| error NEWLINE { yyerrok; }
	;

background_command:
	command_and_args iomodifier_opt AMPERSAND{
		printf("   Yacc: Execute command with background operator\n");
		Command::_currentCommand._background = 'Y';

	}
	;
pipeline_sequence:
	 command_and_args {
	// printf("I am in pipline sequence\n");
	 	
	}
	| 
	pipeline_sequence PIPELINE command_and_args {
		//printf("I am in recursive pipline sequence\n");
		//Command::_currentSimpleCommand = new SimpleCommand();
	}
	|
	;


    
arg_list:
	arg_list argument
	| /* can be empty */
	;

argument:
	WORD {
               printf("   Yacc: insert argument \"%s\"\n", $1);

	       Command::_currentSimpleCommand->insertArgument( $1 );\
	}
	;

command_word:
	WORD {
               printf("   Yacc: insert command \"%s\"\n", $1);
	       
	       Command::_currentSimpleCommand = new SimpleCommand();
	       Command::_currentSimpleCommand->insertArgument( $1 );
	}
	;

iomodifier_opt:
	GREAT  WORD {
		printf("   Yacc: insert output \"%s\"\n", $2);
		Command::_currentCommand._outFile = $2;
	}
	| APPEND WORD {
		printf("   Yacc: append output \"%s\"\n", $2);
		Command::_currentCommand._outFile = $2;
		Command::_currentCommand._append = true;
	}
	| INPUT  WORD {
		printf("   Yacc: insert intput \"%s\"\n", $2);
		Command::_currentCommand._inputFile = $2;
	}
	| GREATAMPERSAND WORD {
		printf("   Yacc: output in error file \"%s\"\n", $2);
		Command::_currentCommand._errFile = $2;
	}
	| APPENDAMPERSAND WORD {
		printf("   Yacc: append in error file \"%s\"\n", $2);
		Command::_currentCommand._errFile = $2;
		Command::_currentCommand._append = true;
	}
	| GREAT WORD INPUT WORD {
		printf("   Yacc: output \"%s\", input \"%s\"\n", $2, $4);
		Command::_currentCommand._outFile = $2;
		Command::_currentCommand._inputFile = $4;
	}
	| INPUT WORD GREAT WORD {
		printf("   Yacc: output \"%s\", input \"%s\"\n", $2, $4);
		Command::_currentCommand._inputFile = $2;
		Command::_currentCommand._outFile = $4;
	}
	| /* can be empty */ 
	;

%%

void
yyerror(const char * s)
{
	fprintf(stderr,"%s", s);
	Command::_currentCommand.clear();
}

#if 0
main()
{
	yyparse();
}
#endif
