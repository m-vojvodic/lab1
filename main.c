// UCLA CS 111 Lab 1 main program

#include <errno.h>
#include <error.h>
#include <getopt.h>
#include <stdio.h>
#include "alloc.h"
#include "command.h"
#include "command-internals.h"

/* LAB 1C */

static
void process_command(struct queue_node* q_node,struct command* cmd)
{
	if(cmd->type == SIMPLE_COMMAND)
	{
		if(cmd->output != NULL)
			q_node->write_list[q_node->num_write++] = cmd->output;
		if(cmd->input != NULL)
			q_node->read_list[q_node->num_read++] = cmd->input;
		int i = 0;
		while(cmd->u.word[i] != NULL)
		{
			q_node->read_list[q_node->num_read++] = cmd->u.word[i];
			i++;
		}
		q_node->read_list[q_node->num_read] = NULL;
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

// Add command trees to the tail of the queue
struct queue_node* enqueue(struct queue* q, struct command_node* cmd_node)
{
	struct queue_node* new_node = (struct queue_node*) checked_malloc(sizeof(struct queue_node));

	if(q->tail == NULL && q->head == NULL) //empty queue
	{
		q->head = new_node;
		q->tail = new_node;
		new_node->next = NULL;
		new_node->prev = NULL;
		new_node->g_node = (struct graph_node*) checked_malloc(sizeof(struct graph_node));
	}
	else
	{
		q->tail->next = new_node;
		new_node->prev = q->tail;
		new_node->next = NULL;
		q->tail = new_node;
		new_node->g_node = (struct graph_node*) checked_malloc(sizeof(struct graph_node));
	}
	new_node->g_node->cmd = cmd_node->cmd;
	new_node->g_node->before = NULL;
	new_node->g_node->num_before = 0;
	new_node->g_node->pid = -1;
	new_node->num_read = 0;
	new_node->num_write = 0;
	process_command(new_node, cmd_node->cmd);
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
    free_token_stream(tokens); */
  	fclose(script_stream);
	return 0;
}
