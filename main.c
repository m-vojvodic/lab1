// UCLA CS 111 Lab 1 main program

#include <errno.h>
#include <error.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "alloc.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "command.h"
#include "command-internals.h"

/********************************** LAB 1C  **********************************/

static
bool check_lists(char** a_list, char** b_list)
{
  int i = 0;
  int j = 0;

  while(a_list[i] != NULL)
  {
    while(b_list[j] != NULL)
    {
      if(strcmp(a_list[i], b_list[j]) == 0) // if two files match, dependency
      {
        return true;
      }
      j++;
    }
    i++;
  }
  return false;
}

static
void process_command(struct queue_node* q_node,struct command* cmd)
{
  // if here
  if(q_node->num_read == q_node->alloc_read)
  {
    q_node->alloc_read *= 2;
    q_node->read_list = (char**)checked_realloc(q_node->read_list, q_node->alloc_read * sizeof(char*));
  }
  if(q_node->num_write == q_node->alloc_write)
  {
    q_node->alloc_write *= 2;
    q_node->write_list = (char**)checked_realloc(q_node->write_list, q_node->alloc_write * sizeof(char*));
  }

  if(cmd->type == SIMPLE_COMMAND)
  {
    if(cmd->output != NULL)
      q_node->write_list[q_node->num_write++] = cmd->output;
    if(cmd->input != NULL)
      q_node->read_list[q_node->num_read++] = cmd->input;
    // check all words but first (which is command)
    // what about exec/flags
    int i = 1;
    while(cmd->u.word[i] != NULL)
    {
      // if here
      if(q_node->num_read == q_node->alloc_read)
      {
        q_node->alloc_read *= 2;
        q_node->read_list = (char**)checked_realloc(q_node->read_list, q_node->alloc_read * sizeof(char*));
      }
      q_node->read_list[q_node->num_read++] = cmd->u.word[i];
      i++;
    }
  }
  else if(cmd->type == SUBSHELL_COMMAND)
  {
    if(cmd->output != NULL)
      q_node->write_list[q_node->num_write++] = cmd->output;
    if(cmd->input != NULL)
      q_node->read_list[q_node->num_read++] = cmd->input;
    process_command(q_node, cmd->u.subshell_command);
  }
  else
  {
    process_command(q_node, cmd->u.command[0]);
    process_command(q_node, cmd->u.command[1]);
  }
}

void create_dependency_graphs(struct dependency_graph* d_graph, struct queue* cmd_queue)
{
  // if only ENDOFFILE, return
  if(cmd_queue->head == NULL)
  {
    d_graph->no_dependencies = NULL;
    d_graph->dependencies = NULL;
    return;
  }
  
  struct queue_node* current = cmd_queue->head;

  while(current != NULL)
  {
    // create a queue node copy new_node
    // new->g_node = current->g_node
    // to ease creation of no/dependencies queue in dependency graph
    struct queue_node* new_node = (struct queue_node*) checked_malloc(sizeof(struct queue_node));

    new_node->next = NULL;
    new_node->g_node = current->g_node;
    new_node->read_list = current->read_list;
    new_node->num_read = current->num_read;
    new_node->write_list = current->write_list;
    new_node->num_write = current->num_write;

    // place in dependent or independent process queues
    // independent
    if(current->g_node->before[0] == NULL) 
    {
      if(d_graph->no_dependencies->head == NULL && d_graph->no_dependencies->tail == NULL) // empty
      {
        d_graph->no_dependencies->head = new_node;
        d_graph->no_dependencies->tail = new_node;
        new_node->prev = NULL;
      }
      else
      {
        d_graph->no_dependencies->tail->next = new_node;
        new_node->prev = d_graph->no_dependencies->tail;
        d_graph->no_dependencies->tail = new_node;
      }
    }
    // dependent
    else
    {
      if(d_graph->dependencies->head == NULL && d_graph->dependencies->tail == NULL) // empty
      {
        d_graph->dependencies->head = new_node;
        d_graph->dependencies->tail = new_node;
        new_node->prev = NULL;
      }
      else
      {
        d_graph->dependencies->tail->next = new_node;
        new_node->prev = d_graph->dependencies->tail;
        d_graph->dependencies->tail = new_node;
      }
    }

    // advance to next queue node
    current = current->next;
  }

  // for debugging
  /*
  int i = 0;
  fprintf(stderr, "NO DEPENDENCIES\n");
  current = d_graph->no_dependencies->head;
  while(current != NULL)
  {
    fprintf(stderr, "%d: Command type %d\n", i, current->g_node->cmd->type);
    i++;
    current = current->next;
  }

  i = 0;
  fprintf(stderr, "DEPENDENCIES\n");
  current = d_graph->dependencies->head;
  while(current != NULL)
  {
    fprintf(stderr, "%d: Command type %d\n", i, current->g_node->cmd->type);
    i++;
    current = current->next;
  }
  */
  return;
}

// Add command trees to the tail of the queue
struct queue_node* enqueue(struct queue* q, struct command* cmd)
{
  // if the command type is EOF, return
  if(cmd == NULL)
    return NULL;

  struct queue_node* new_node = (struct queue_node*) checked_malloc(sizeof(struct queue_node));

  if(q->tail == NULL && q->head == NULL) // empty queue
  {
    q->head = new_node;
    q->tail = new_node;
    new_node->next = NULL;
    new_node->prev = NULL;
    new_node->g_node = (struct graph_node*) checked_malloc(sizeof(struct graph_node));
  }
  else // non-empty queue
  {
    q->tail->next = new_node;
    new_node->prev = q->tail;
    new_node->next = NULL;
    q->tail = new_node;
    new_node->g_node = (struct graph_node*) checked_malloc(sizeof(struct graph_node));
  }

  // initialize rest of new_node
  new_node->g_node->cmd = cmd;
  new_node->g_node->alloc_before = 5;
  new_node->g_node->before = (struct graph_node**) checked_malloc(new_node->g_node->alloc_before * sizeof(struct graph_node*));
  new_node->g_node->before[0] = NULL;
  new_node->g_node->num_before = 0;
  new_node->g_node->pid = -1;
  new_node->alloc_read = 5;
  new_node->read_list = (char**) checked_malloc(new_node->alloc_read * sizeof(char*));
  new_node->num_read = 0;
  new_node->alloc_write = 5;
  new_node->write_list = (char**) checked_malloc(new_node->alloc_write * sizeof(char*));
  new_node->num_write = 0;

  // create read_list and write_list for the new node
  process_command(new_node, cmd);

  // check for and create dependencies for the new node
  struct queue_node* current = q->head;
  while(current != new_node)
  {
    if(check_lists(current->read_list, new_node->write_list) ||  // RAW
       check_lists(current->write_list, new_node->write_list) || // WAW
       check_lists(current->write_list, new_node->read_list))    // WAR
    {
      // if here
      if(new_node->g_node->num_before == new_node->g_node->alloc_before)
      {
        (new_node->g_node->alloc_before) *= 2;
        new_node->g_node->before = (struct graph_node**)checked_realloc(new_node->g_node->before, new_node->g_node->alloc_before * sizeof(struct graph_node*));
      }

      new_node->g_node->before[new_node->g_node->num_before++] = current->g_node; 
    }
    current = current->next;
  }
 
  return new_node;
}

// Execute command trees from the head of the queue (FIFO)
struct queue_node* dequeue(struct queue* q)
{
  if(q->head == NULL && q->tail == NULL)
  {
    return NULL;
  }
        
  struct queue_node* node = q->head;
  q->head = q->head->next;
  q->head->prev = NULL;
  return node;
}

static void execute_no_dependencies(struct queue* no_dependencies)
{
  struct queue_node* current = no_dependencies->head;
  while(current != NULL)
  {
    pid_t child_pid = fork();
    if(child_pid < 0)
    {
      error(1, errno, "Failed to fork a new process in execute_no_dependencies.\n");
    }
    else if(child_pid == 0) // child process
    {
      // execute the command just like in LAB 1B
      execute_command(current->g_node->cmd, false);
      exit(1);
    }
    else // parent process
    {
      // set pid to show dependent processes this command is currently executing
      current->g_node->pid = child_pid;  
    }
    current = current->next;
  }
}

static void execute_dependencies(struct queue* dependencies)
{
  struct queue_node* current = dependencies->head;
  int i;
  int count;
  while(current != NULL)
  {
  loop:
    count = current->g_node->num_before;
    // check to make sure all dependencies ahve been run
    for(i = 0; i < count; i++)
    {
      if(current->g_node->before[i]->pid == -1)
      {
        goto loop;
      }
    }
    
    int eStatus;
    // wait for all dependencies to finish
    for(i = 0; i < count; i++)
    {
      waitpid(current->g_node->before[i]->pid, &eStatus, 0);
      current->g_node->num_before--;
    }

    pid_t child_pid = fork();
    if(child_pid < 0)
    {
      error(1, errno, "Failed to fork a new process in execute_dependencies.\n");
    }
    else if(child_pid == 0) // child process
    {
      // execute the command just like in LAB 1B
      execute_command(current->g_node->cmd, false);
      exit(1);
    }
    else // parent process
    {
      current->g_node->pid = child_pid;
    }
    
    current = current->next;
  }
}

static void execute_graph(struct dependency_graph* d_graph)
{
  // empty script or just comments
  if(d_graph->no_dependencies == NULL && d_graph->dependencies == NULL)
    return;

  execute_no_dependencies(d_graph->no_dependencies);
  execute_dependencies(d_graph->dependencies);
}

/********************************************************************************/

static char const *program_name;
static char const *script_name;

static void
usage (void)
{
  error (1, 0, "usage: %s [-pt] SCRIPT-FILE", program_name);
}

static int
get_next_byte (void *stream)
{
  return getc (stream);
}

int
main (int argc, char **argv)
{
  int command_number = 1;
  bool print_tree = false;
  bool time_travel = false;
  program_name = argv[0];

  for (;;)
    switch (getopt (argc, argv, "pt"))
      {
        case 'p': print_tree = true; break;
        case 't': time_travel = true; break;
        default: usage (); break;
        case -1: goto options_exhausted;
      }
 options_exhausted:;
  // There must be exactly one file argument.
  if (optind != argc - 1)
    usage ();

  script_name = argv[optind];
  FILE *script_stream = fopen (script_name, "r");
  if (! script_stream)
    error (1, errno, "%s: cannot open", script_name);

  
  command_stream_t command_stream =
    make_command_stream (get_next_byte, script_stream);

  // if time_travel
  if(time_travel)
  {
    // case: EOF only
    if(command_stream->head == NULL)
      return 0;

    struct queue* command_queue = (struct queue*) checked_malloc(sizeof(struct queue));
    command_queue->head = NULL;
    command_queue->tail = NULL;
    
    struct dependency_graph* dgaf = (struct dependency_graph*) checked_malloc(sizeof(struct dependency_graph));
    dgaf->no_dependencies = (struct queue*) checked_malloc(sizeof(struct queue));
    dgaf->no_dependencies->head = NULL;
    dgaf->no_dependencies->tail = NULL;
    dgaf->dependencies = (struct queue*) checked_malloc(sizeof(struct queue));
    dgaf->dependencies->head = NULL;
    dgaf->dependencies->tail = NULL;
    
    command_node_t current_command_node = command_stream->head;
    command_node_t prev_command_node = NULL;
    
    while(current_command_node != NULL)
    {
      enqueue(command_queue, current_command_node->cmd);
      prev_command_node = current_command_node;
      current_command_node = current_command_node->next;
    }
    create_dependency_graphs(dgaf, command_queue);
    execute_graph(dgaf);
    return !prev_command_node ? 0 : command_status (prev_command_node->cmd);
  }

  // not time travel
  command_t last_command = NULL;
  command_t command;
  while ((command = read_command_stream (command_stream)))
    {
      if (print_tree)
        {
          printf ("# %d\n", command_number++);
          print_command (command);
        }
      else
        {
          last_command = command;
          execute_command (command, time_travel);
        }
    }

  return print_tree || !last_command ? 0 : command_status (last_command);
  
  /*
  struct token_stream tokens = make_token_stream(get_next_byte, script_stream);
  struct token* current = tokens.head;
  while(current != NULL)
  {
    int i = 0;
    switch(current->type)
    {
      case AND:
        fprintf(stdout, "&&");
        break;
      case OR:
        fprintf(stdout, "||");
        break;
      case PIPE:
        fprintf(stdout, "|");
        break;
      case SEMICOLON:
        fprintf(stdout, ";");
        break;
      case INPUT:
        fprintf(stdout, "<"); 
        break;
      case OUTPUT:
        fprintf(stdout, ">"); 
        break;
      case LEFT_PAREN:
        fprintf(stdout, "(");
        break;
      case RIGHT_PAREN:
        fprintf(stdout, ")");
        break;
      case WORD:
      case COMMENT:
        while(current->word[i] != '\0')
        {
          fprintf(stdout, "%c", current->word[i]);
          i++;
        }
        break;
      case NEWLINE:
        fprintf(stdout, "\n");
        break;
      default:
        break;
      }
      current = current->next;
    }
    free_token_stream(tokens);
  fclose(script_stream);
  return 0;
  */
}
