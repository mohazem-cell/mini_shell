
/*
 * CS354: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include "command.h"
#include <fcntl.h>

#define LOG_FILE "Termination_log.txt"




SimpleCommand::SimpleCommand()
{
	// Creat available space for 5 arguments
	_numberOfAvailableArguments = 5;
	_numberOfArguments = 0;
	_arguments = (char **)malloc(_numberOfAvailableArguments * sizeof(char *));
}

void SimpleCommand::insertArgument(char *argument)
{
	if (_numberOfAvailableArguments == _numberOfArguments + 1)
	{
		// Double the available space
		_numberOfAvailableArguments *= 2;
		_arguments = (char **)realloc(_arguments,
									  _numberOfAvailableArguments * sizeof(char *));
	}

	_arguments[_numberOfArguments] = argument;

	// Add NULL argument at the end
	_arguments[_numberOfArguments + 1] = NULL;

	_numberOfArguments++;
}

Command::Command()
{
	// Create available space for one simple command
	_numberOfAvailableSimpleCommands = 1;
	_simpleCommands = (SimpleCommand **)
		malloc(_numberOfSimpleCommands * sizeof(SimpleCommand *));

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
}

void Command::insertSimpleCommand(SimpleCommand *simpleCommand)
{
	if (_numberOfAvailableSimpleCommands == _numberOfSimpleCommands)
	{
		_numberOfAvailableSimpleCommands *= 2;
		_simpleCommands = (SimpleCommand **)realloc(_simpleCommands,
													_numberOfAvailableSimpleCommands * sizeof(SimpleCommand *));
	}

	_simpleCommands[_numberOfSimpleCommands] = simpleCommand;
	_numberOfSimpleCommands++;
}

void Command::clear()
{
	for (int i = 0; i < _numberOfSimpleCommands; i++)
	{
		// printf("_numberOfSimpleCommands: %d @ %d\n", _numberOfSimpleCommands, i);
		for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
		{
			// printf("_numberOfArguments: %d @ %d\n", _simpleCommands[ i ]->_numberOfArguments, j);
			free(_simpleCommands[i]->_arguments[j]);
		}

		free(_simpleCommands[i]->_arguments);
		free(_simpleCommands[i]);
	}
	// printf("not for loop\n");

	if (_outFile)
	{
		free(_outFile);
	}

	if (_inputFile)
	{
		free(_inputFile);
	}

	if (_errFile)
	{
		free(_errFile);
	}

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
	_append = 0;
	_pipline = 0;
}

void Command::print()
{
	printf("\n\n");
	printf("              COMMAND TABLE                \n");
	printf("\n");
	printf("  #   Simple Commands\n");
	printf("  --- ----------------------------------------------------------\n");

	for (int i = 0; i < _numberOfSimpleCommands; i++)
	{
		printf("  %-3d ", i);
		// printf("\n %d ", _numberOfSimpleCommands);
		for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
		{

			printf("\"%s\" \t", _simpleCommands[i]->_arguments[j]);
		}
		printf("\n");
	}

	printf("\n\n");
	printf("  Output       Input        Error        Background\n");
	printf("  ------------ ------------ ------------ ------------\n");
	printf("  %-12s %-12s %-12s %-12s\n", _outFile ? _outFile : "default",
		   _inputFile ? _inputFile : "default", _errFile ? _errFile : "default",
		   _background ? "YES" : "NO");
	printf("\n\n");
}



void Command::execute()
{
	// Don't do anything if there are no simple commands
	if (_numberOfSimpleCommands == 0)
	{
		prompt();
		return;
	}

	// printf("\n	%d	\n",_numberOfSimpleCommands);
	//  Print contents of Command data structure
	print();


	// Add execution here
	// For every simple command fork a new process
	// Setup i/o redirection
	// and call exec

	int defaultin = dup(0);	 // Default file Descriptor for stdin
	int defaultout = dup(1); // Default file Descriptor for stdout
	int defaulterr = dup(2); // Default file Descriptor for stderr
	int fdpipe[2];
	int fdprevpipe = -1;
	for (int i = 0; i < _numberOfSimpleCommands; i++)
	{
		SimpleCommand *simpleCommand = _simpleCommands[i];
		if (_inputFile)
		{
			int inputFolder = open(_inputFile, O_RDONLY);
			if (inputFolder < 0)
			{
				printf("Unable to open input file\n");
				exit(1);
			}
			dup2(inputFolder, STDIN_FILENO);
			close(inputFolder);
		}

		if (_outFile)
		{
			int outputFolder;
			if (_append)
			{
				outputFolder = open(_outFile, O_WRONLY | O_CREAT | O_APPEND, 0644);
			}
			else
			{
				outputFolder = open(_outFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
			}

			if (outputFolder < 0)
			{
				printf("Unable to open output folder\n");
				exit(1);
			}
			dup2(outputFolder, STDOUT_FILENO);
			close(outputFolder);
		}

		if (_errFile)
		{
			int errFolder;
			if (_append)
			{
				errFolder = open(_errFile, O_WRONLY | O_CREAT | O_APPEND, 0644);
			}
			else
			{
				errFolder = open(_errFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
			}
			if (errFolder < 0)
			{
				printf("Unable to open error folder\n");
				exit(1);
			}
			dup2(errFolder, 2);
			close(errFolder);
		}

		if (i < _numberOfSimpleCommands - 1)
		{
			if (pipe(fdpipe) == -1)
			{
				perror("cat_grep: pipe");
				exit(2);
			}
		}

		int pid = fork();
		if (pid == -1)
		{
			perror("Fork Failed\n");
			exit(2);
		}
		else if (pid == 0)
		{
			// In the child process,

			if (fdprevpipe != 0)
			{
				dup2(fdprevpipe, STDIN_FILENO);
				close(fdprevpipe);
			}

			if (i < _numberOfSimpleCommands - 1)
			{
				dup2(fdpipe[1], STDOUT_FILENO);
				close(fdpipe[0]);
				close(fdpipe[1]);
			}

			execvp(simpleCommand->_arguments[0], simpleCommand->_arguments);
			perror("Exec failed");
			exit(2);
		}
		else
		{
			if (!_background)
			{
				waitpid(pid, 0, 0);
			}

			close(fdpipe[1]);

			if (fdprevpipe != -1)
			{
				close(fdprevpipe);
			}
			fdprevpipe = fdpipe[0];
		}
		dup2(defaultin, 0);
		dup2(defaultout, 1);
		dup2(defaulterr, 2);
	}

	// Clear to prepare for next command
	clear();

	// Print new prompt
	prompt();
}

// Shell implementation

void Command::prompt()
{
	printf("myshell>");
	fflush(stdout);
}

void handle_sigint(int sig)
{
	printf("\n");
	Command::_currentCommand.prompt();
}

void handle_sigchld(int sig) {
    FILE *log = fopen(LOG_FILE, "a");
    if (log != NULL) {
        time_t now = time(NULL);
        fprintf(log, "Child terminated at %s", ctime(&now));
        fclose(log);
    }
}

// void changedirectory(){}

Command Command::_currentCommand;
SimpleCommand *Command::_currentSimpleCommand;

int yyparse(void);

int main()
{
	signal(SIGINT, handle_sigint);
	signal(SIGCHLD, handle_sigchld);

	Command::_currentCommand.prompt();
	yyparse();
	return 0;
}
