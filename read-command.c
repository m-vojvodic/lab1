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
  INPUT,
  OUTPUT,
  WORD,
  COMMENT,
  NEWLINE,
  ENDOFFILE,
};

typedef struct token
{
  enum token_type type;
  char *word;
  struct token *next;
}token;

typedef struct token_stream
{
  token *head;
  token *tail;
}token_stream;

/*struct command_stream
{
	command_t *cmd_stream; //pointer to beginning of stream
	int num_commands; //number of command in the array
	int curr_index; //the index of the current command
	int size; //the total size of the array, changes with reallocation
}*/

typedef struct command_node
{
  struct command_t *command;
  struct command_node *next;
}command_node;

typedef struct command_stream
{
  command_node *head;
  command_node *tail;
}command_stream;

typedef struct command_stack_node
{
  command_t *cmd;
  command_stack_node* prev;
}command_stack_node;

typedef struct command_stack
{
  command_node *top;
  command_node *bottom;
}command_stack;

command_stack_node* push_command (command_stack_node *stack, command_t *cmd)
{
  command_stack_node* newtop = (command_stack_node*) checked_malloc(sizeof(command_stack_node));
  newtop->cmd = cmd;
  newtop->prev = stack->top;
  stack->top = newtop;
  return stack->top;
}

command_t* pop_command (command_stack_node *stack)
{
  command_stack_node *temp = stack->top;
  top = top->prev;
  cmd* val = temp->cmd;
  free(temp);
  return val;
}

command_node insert_command_node (command_node *tail, command_t cmd)
{
  command_node *node = (command_node*)checked_malloc(sizeof(command_node));
  node->command = &cmd;
  node->next = NULL;
  tail->next = node;
  tail = node;
}

int precedence(enum command_type op)
{
  switch(op)
  {
    case SEQUENCE_COMMAND:
      return 0;
    case AND_COMMAND:
    case OR_COMMAND:
      return 1;
    case PIPE_COMMAND:
      return 2;
    case SUBSHELL_COMMAND:
      return 3;
    default:
      return -1;
  }
}

/*static int get_byte(int (*get_next_byte) (void *), void *get_next_byte_arg)
{
	return get_next_byte(get_next_byte_arg);
}*/

static int is_word(int token)
{
  if((token >= '0' && token <= '9') || (token >= 'a' && token <= 'z') ||
     (token >= 'A' && token <= 'Z') || token == '!' || token == '%'   || 
      token == '+' || token == ','  || token == '-' || token == '.'   ||
      token == '/' || token == ':'  || token == '@' || token == '^'   || 
      token == '_')
    return 1;
  else
    return 0;
}

static char* get_word(int (*get_next_byte) (void *), void *get_next_byte_arg, int first_ch)
{
  int next_byte = get_next_byte(get_next_byte_arg);
  int size = 10;
  int current = 0;
  char* word = NULL;
  
  if(next_byte != EOF)
  {
    word = (char*) checked_malloc(size * sizeof(char));
    word[current++] = first_ch;
      
    while(is_word(next_byte))
    {
      if(current == size)
      {
	size *= 2;
	word = (char*) checked_realloc(size * sizeof(char));
      }
      word[current++] = (char) next_byte;
      next_byte = get_next_byte(get_next_byte_arg);
    }
  }
  
  if(current == size)
  {
    size += 1;
    word = (char*) checked_realloc(size * sizeof(char));
  }
  
  word[current++] = '\0';
  return word;
}


static int is_comment(int token)
{
  if(token == '#')
    return 1;
  else
    return 0;
}

static char* get_comment(int (*get_next_byte) (void *), void *get_next_byte_arg)
{
  int next_byte = get_next_byte(get_next_byte_arg);
  int size = 10;
  int current = 0;
  char* comment = NULL;

  if(next_byte != EOF)
  {
    comment = (char*) checked_malloc(size * sizeof(char));
    comment[current++] = '#';
      
    while(next_byte != '\n')
    {
      if(current == size)
      {
        size *= 2;
        comment = (char*) checked_realloc(size * sizeof(char));
      }
      comment[current++] = (char) next_byte;
      next_byte = get_next_byte(get_next_byte_arg);
    }
  }
  else // # is last comment
  {
    
  }
  return new_token; //is this correct?
}

token_stream* make_token_stream(int (*get_next_byte) (void *), void *get_next_byte_arg)
{
  token_stream* tokens;
  token* current_token = get_next_token(get_next_byte, get_next_byte_arg);
  token* next_token = NULL;
  if(current_token == NULL) // what case is this for?
    return NULL;
  tokens->head = current_token;
  tokens->tail = current_token;

  while(current_token->type != ENDOFFILE)
  {
    next_token = get_next_token(get_next_byte, get_next_byte_arg);
    current_token->next = next_token;
    tokens->tail = next_token;    
    current_token = next_token;
  }
  tail->next = NULL;

  return tokens;
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
