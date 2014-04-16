// UCLA CS 111 Lab 1 command execution

#include <error.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "command.h"
#include "command-internals.h"

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

void execute(struct command* cmd);

static void
execute_and_command(struct command* cmd)
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
      else if (right_pid == 0) // child process executes the right command
      {
        execute(cmd->u.command[1]);
        _exit(cmd->u.command[1]->status);
      }
      else // parent process
      {
        waitpid(right_pid, &eStatus, 0); //wait for the right command to finish executing
        cmd->status = WEXITSTATUS(eStatus);
      }
    }
    else //if left side of && is false, don't need to execute right side
    {
      cmd->status = WEXITSTATUS(eStatus);
    }
  }

  fprintf(stderr, "Executed command of type:%d\n", cmd->type);
  return;
}

static void
execute_or_command(struct command* cmd)
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
    }
  }

  fprintf(stderr, "Executed command of type:%d\n", cmd->type);
  return;
}

static void
execute_sequence_command(struct command* cmd)
{
  // execute left, then execute right
  pid_t left_pid;
  pid_t right_pid;
  int eStatus;

  left_pid = fork();
  if(left_pid < 0) // fork failed
  {
    error(1, errno, "fork was unsuccessful in left child of execute_sequence\n");
  }

  if(left_pid == 0) // child process executes left command
  {
    execute(cmd->u.command[0]);
    _exit(cmd->u.command[0]->status);
  }
  if(left_pid > 0) // parent process
  {
    waitpid(left_pid, &eStatus, 0);

    right_pid = fork();
    if(right_pid < 0)
    {
      error(1, errno, "fork was unsuccessful in right child of execute_sequence\n");
    }

    if(right_pid == 0) // child process executes right command
    {
      execute(cmd->u.command[1]);
      _exit(cmd->u.command[1]->status);
    }

    if(right_pid > 0) // parent process
    {
      waitpid(right_pid, &eStatus, 0);
      // sequence command return status is that of last command
      cmd->status = WEXITSTATUS(eStatus);
    }
  }
        
  fprintf(stderr, "Executed command of type:%d\n", cmd->type);
  return;
}

// TODO: pipe command
static void
execute_pipe_command(struct command* cmd)
{
  fprintf(stderr, "Executed command of type:%d\n", cmd->type);
  return;
}

static void
execute_simple_command(struct command* cmd)
{
  //fork a child process to execute the command and run execvp() as the child process
  pid_t child_pid;
  int eStatus;
  
  child_pid = fork();
  if(child_pid < 0)
  {
    error(1, errno, "fork was unsuccessful.");
  }
  else if(child_pid == 0) //need to check for input/output first?
  {
    int inputRedir;
    int outputRedir;
    if(cmd->input != NULL)
    {
      inputRedir = open(cmd->input, O_RDONLY);
      if(inputRedir < 0)
      {
        error(1, errno, "Input file does not exist.\n");
      }
      if(dup2(inputRedir, 0) < 0)
      {
        error(1, errno, "Could not override stdin.\n");
      }
    }

    if(cmd->output != NULL)
    {
      outputRedir = open(cmd->output, O_WRONLY|O_TRUNC|O_CREAT, S_IRUSR|S_IWUSR);
      if(outputRedir < 0)
      {
        error(1, errno, "Output file does not exist.\n");
      }
      if(dup2(outputRedir, 1) < 0)
      {
        error(1, errno, "Could not override stdout.\n");
      }
    }

    execvp(cmd->u.word[0], cmd->u.word);
    _exit(cmd->status);

    if(cmd->input != NULL)
    {
      close(inputRedir);
    }
    if(cmd->output != NULL)
    {
      close(outputRedir);
    }
  }
  if(child_pid > 0) // parent
  {
    waitpid(child_pid, &eStatus, 0);
    cmd->status = WEXITSTATUS(eStatus);
  }

  fprintf(stderr, "Executed command of type:%d\n", cmd->type);
  return;
}

static void
execute_subshell_command(struct command* cmd)
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
    int inputRedir;
    int outputRedir;
    if(cmd->input != NULL)
    {
      inputRedir = open(cmd->input, O_RDONLY);
      if(inputRedir < 0)
      {
        error(1, errno, "Input file does not exist.\n");
      }
      if(dup2(inputRedir, 0) < 0)
      {
        error(1, errno, "Could not override stdin.\n");
      }
    }

    if(cmd->output != NULL)
    {
      outputRedir = open(cmd->output, O_WRONLY|O_TRUNC|O_CREAT, S_IRUSR|S_IWUSR);
      if(outputRedir < 0)
      {
        error(1, errno, "Output file does not exist.\n");
      }
      if(dup2(outputRedir, 1) < 0)
      {
        error(1, errno, "Could not override stdout.\n");
      }
    }

    execute(cmd->u.subshell_command);
    _exit(cmd->u.subshell_command->status);
    
    if(cmd->input != NULL)
    {
      close(inputRedir);
    }
    if(cmd->output != NULL)
    {
      close(outputRedir);
    }
  }
  else // parent process
  {
    waitpid(child_pid, &eStatus, 0);
    cmd->status = WEXITSTATUS(eStatus);
  }

  fprintf(stderr, "Executed command of type:%d\n", cmd->type);
  return;
}

int
command_status (struct command* c)
{
  return c->status;
}

/* a function which executes a command appropriately according to the type
 * specification in the command structure. This function is called
 * recursively by execute command */
void
execute (struct command* cmd)
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
