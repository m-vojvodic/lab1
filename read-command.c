// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"
#include "alloc.h"
#include <error.h>

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

enum token_type
{
	AND,
	OR,
	PIPE,
	SEMICOLON,
	LEFT_PAREN,
	RIGHT_PAREN,
	IN,
	OUT,
	WORD,
	COMMENT,
	NEWLINE,
	ENDOFFILE,
};

struct token
{
	enum token_type type;
	char* word;
};

/*struct command_stream
{
	command_t *cmd_stream; //pointer to beginning of stream
	int num_commands; //number of command in the array
	int curr_index; //the index of the current command
	int size; //the total size of the array, changes with reallocation
}*/

typedef struct command_stream
{
	struct command_node *head;
	struct command_node *tail;
}command_stream;

typedef struct command_node
{
	struct command_t *command;
	struct command_node *next;
}command_node;

command_node insert_node (command_node *tail, command_t cmd)
{
	command_node *node = (command_node*)checked_malloc(sizeof(command_node));
	node->command = &cmd;
	node->next = NULL;
	tail->next = node;
	tail = node;
}

command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
  error (1, 0, "command reading not yet implemented");
  return 0;
}

command_t
read_command_stream (command_stream_t s)
{
  /* FIXME: Replace this with your implementation too.  */
  error (1, 0, "command reading not yet implemented");
  return 0;
}
