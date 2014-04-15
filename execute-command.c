// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"

#include <error.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

void execute(command_t cmd);

static void
execute_and_command(command_t cmd)
{
	// fork a new process to execute the left command.
	// if the left command is a success (exit status)
	// execute the right command by forking another 
	// process. else the entire command returns 1 so 
	// the right command does not need to be executed.
	
	pid_t left_pid;
	pid_t right_pid;
	int eStatus;
	 
	left_pid = fork();
	if (left_pid < 0)
	{
		error(1, errno, "fork was unsuccessful");
	}
	else if (left_pid == 0) // child process executes the left command
	{
		execute(cmd->u.command[0]);
		_exit(cmd->u.command[0]->status);
	}
	else // parent process
	{
		// have to wait for the child process to finish executing the left
		// command. Then, if it succeeded, run the right command with another
		// child process. If it failed, set the command status to the status
		// of the first child.
		waitpid(left_pid, &eStatus, 0); //wait for the left command to finish executing
		if(WEXITSTATUS(eStatus) == 0) //if left side of && is true, need to execute right side
		{
			right_pid = fork();
			if(right_pid < 0)
			{
				error(1, errno, "fork was unsuccessful");
			}
			else if (left_pid == 0) // child process executes the right command
			{
				execute(cmd->u.command[1]);
				_exit(cmd->u.command[1]->status);
			}
			else // parent process
			{
				waitpid(right_pid, &eStatus, 0); //wait for the right command to finish executing
				cmd->status = WEXITSTATUS(eStatus);
			}
			return;
		}
		else //if left side of && is false, don't need to execute right side
		{
			cmd->status = WEXITSTATUS(eStatus);
			return;
		}
	}
}

static void
execute_or_command(command_t cmd)
{
	pid_t left_pid;
	pid_t right_pid;
	int eStatus;

	left_pid = fork();
	if(left_pid < 0)
	{
		error(1, errno, "fork was unsuccessful");
	}
	else if(left_pid == 0) // child process executes the left command
	{
		execute(cmd->u.command[0]);
		_exit(cmd->u.command[0]->status);
	}
	else
	{
		waitpid(left_pid, &eStatus, 0);
		if(WEXITSTATUS(eStatus) == 0) // if left side of || is true, don't need to execute right side
		{
			cmd->status = WEXITSTATUS(eStatus);
			return;
		}
		else // if left side of || is false, need to execute right side
		{
			right_pid = fork();
			if(right_pid < 0)
			{
				error(1, errno, "fork was unsuccessful");
			}
			else if(right_pid == 0) // child process executes the right command
			{
				execute(cmd->u.command[1]);
				_exit(cmd->u.command[1]->status);
			}
			else // parent process
			{
				waitpid(right_pid, &eStatus, 0);
				cmd->status = WEXITSTATUS(eStatus);
			}
			return;
		}
	}
}

static void
execute_sequence_command(command_t cmd)
{
	fprintf(stderr, "%d\n", cmd->type);
	return;
}

static void
execute_pipe_command(command_t cmd)
{
	fprintf(stderr, "%d\n", cmd->type);
	return;
}

static void
execute_simple_command(command_t cmd)
{
	//fork a child process to execute the command and run execvp() as the child process
	pid_t child_pid;
	
	child_pid = fork();
	if(child_pid < 0)
	{
		error(1, errno, "fork was unsuccessful.");
	}
	else if(child_pid == 0) //need to check for input/output first?
	{
		//execvp();
	}
	fprintf(stderr, "%d\n", cmd->type);
	return;
}

static void
execute_subshell_command(command_t cmd)
{
	pid_t child_pid;
	int eStatus;	

	child_pid = fork();
	if(child_pid < 0) 
	{
		error(1, errno, "fork was unsuccessful");
	}
	else if(child_pid == 0) // child process executes subshell command
	{
		execute(cmd->u.subshell_command);
		_exit(cmd->u.subshell_command->status);
	}
	else // parent process
	{
		waitpid(child_pid, &eStatus, 0);
		cmd->status = WEXITSTATUS(eStatus);
	}
	return;
}

int
command_status (command_t c)
{
  return c->status;
}

/* a function which executes a command appropriately according to the type
 * specification in the command structure. This function is called
 * recursively by execute command */
void
execute (command_t cmd)
{
	switch(cmd->type)
	{
		case AND_COMMAND:
			//write a function execute_and_command() to execute
			execute_and_command(cmd);
			break;
		case SEQUENCE_COMMAND:
			//write a function execute_sequence_command() to execute
			execute_sequence_command(cmd);
			break;
		case OR_COMMAND:
			//write a function execute_or_command() to execute
			execute_or_command(cmd);
			break;
		case PIPE_COMMAND:
			//write a function execute_pipe_command() to execute
			execute_pipe_command(cmd);
			break;
		case SIMPLE_COMMAND:
			//write a function execute_simple_command() to execute
			execute_simple_command(cmd);
			break;
		case SUBSHELL_COMMAND:
			//write a function execute_subshell_command() to execute
			execute_subshell_command(cmd);
			break;
		default:
			error(1, 0, "Not a valid command.");
	}
}

void
execute_command (command_t c, bool time_travel)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
	if(time_travel == false)
	{
		execute(c);
	}
}
