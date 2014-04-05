// UCLA CS 111 Lab 1 command reading

#include "stdlib.h"
#include "stdio.h"
#include "command.h"
#include "command-internals.h"
#include "alloc.h"
#include <error.h>

static int line_number = 1;

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

/*
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
*/
/*
int precedence(command_type_t op)
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
  return 0;
}
*/
/*static int get_byte(int (*get_next_byte) (void *), void *get_next_byte_arg)
{
	return get_next_byte(get_next_byte_arg);
}*/

int is_word(int token)
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

char* get_word(int (*get_next_byte) (void *), void *get_next_byte_arg, int first_ch)
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
	word = (char*) checked_realloc(word, size * sizeof(char));
      }
      word[current++] = (char) next_byte;
      next_byte = get_next_byte(get_next_byte_arg);
    }
  }
  
  ungetc(next_byte, get_next_byte_arg);

  if(current == size)
  {
    size += 1;
    word = (char*) checked_realloc(word, size * sizeof(char));
  }
  
  word[current++] = '\0';
  return word;
}


int is_comment(int token)
{
  if(token == '#')
    return 1;
  else
    return 0;
}

char* get_comment(int (*get_next_byte) (void *), void *get_next_byte_arg)
{
  int next_byte = get_next_byte(get_next_byte_arg);
  int size = 10;
  int current = 0;
  char* comment = NULL;

  comment = (char*) checked_malloc(size * sizeof(char));
  comment[current++] = '#';

  if(next_byte != EOF)
  {      
    while(next_byte != '\n')
    {
      if(current == size)
      {
        size *= 2;
        comment = (char*) checked_realloc(comment, size * sizeof(char));
      }
      comment[current++] = (char) next_byte;
      next_byte = get_next_byte(get_next_byte_arg);
    }
  }

  ungetc(next_byte, get_next_byte_arg);

  if(current == size)
  {
    size += 1;
    comment = (char*) checked_realloc(comment, size * sizeof(char));
  }
  
  comment[current++] = '\0';

  return comment;
}

struct token* get_next_token(int (*get_next_byte) (void *), void *get_next_byte_arg)
{
  int next_byte = get_next_byte(get_next_byte_arg);
  struct token* new_token;
  while(next_byte == ' ' || next_byte == '\t')
  {
    next_byte = get_next_byte(get_next_byte_arg);
  }
  
  switch(next_byte)
  {
    case '&':
      next_byte = get_next_byte(get_next_byte_arg);
      if(next_byte == '&')
      {
	new_token = (struct token*)checked_malloc(sizeof(struct token));
	(new_token)->type = AND;
	(new_token)->next = NULL;
	(new_token)->word = NULL; //do we need to store a word for special characters?
      }
      else
      {
	fprintf(stderr, "%d: Invalid syntax\n", line_number);
	exit(1);
      }
      break;
    case '|':
      next_byte = get_next_byte(get_next_byte_arg);
      if(next_byte == '|')
      {
	new_token = (struct token*)checked_malloc(sizeof(struct token));
	(new_token)->type = OR;
	(new_token)->next = NULL;
	(new_token)->word = NULL; //do we need to store a word for special characters?
      }
      else
      {
	new_token = (struct token*)checked_malloc(sizeof(struct token));
	(new_token)->type = PIPE;
	(new_token)->next = NULL;
	(new_token)->word = NULL;
	ungetc(next_byte, get_next_byte_arg); // decrement filestream ptr
      }
      break;
    case '(':
      new_token = (struct token*)checked_malloc(sizeof(struct token));
      (new_token)->type = LEFT_PAREN;
      (new_token)->next = NULL;
      (new_token)->word = NULL;
      break;
    case ')':
      new_token = (struct token*)checked_malloc(sizeof(struct token));
      (new_token)->type = RIGHT_PAREN;
      (new_token)->next = NULL;
      (new_token)->word = NULL;
      break;
    case ';':
      new_token = (struct token*)checked_malloc(sizeof(struct token));
      (new_token)->type = SEMICOLON;
      (new_token)->next = NULL;
      (new_token)->word = NULL;
      break;
    case '>':
      next_byte = get_next_byte(get_next_byte_arg);
      if(next_byte != '\n')
      {
	new_token = (struct token*)checked_malloc(sizeof(struct token));
	(new_token)->type = OUTPUT;
	(new_token)->next = NULL;
	(new_token)->word = NULL;
	ungetc(next_byte, get_next_byte_arg); // return the byte read
      }
      else // invalid syntax
      {
	fprintf(stderr, "%d: Invalid syntax\n", line_number);
	exit(1);
      }
      break;
    case '<':
      next_byte = get_next_byte(get_next_byte_arg);
      if(next_byte != '\n')
      {
	new_token = (struct token*)checked_malloc(sizeof(struct token));
	(new_token)->type = INPUT;
	(new_token)->next = NULL;
	(new_token)->word = NULL;
      }
      break;
    case '\n':
      //      next_byte = get_next_byte(get_next_byte_arg);
      //      if(next_byte == '(' || next_byte == ')' || next_byte == '\n' ||
      //	 is_word(next_byte)) //need to include whitespace and newlines?
      {
	new_token = (struct token*)checked_malloc(sizeof(struct token));
	(new_token)->type = NEWLINE;
	(new_token)->next = NULL;
	(new_token)->word = NULL;
	line_number++;
      }
      //      else
      {
	//print out error
      }
      break;
    case EOF:
      new_token = (struct token*)checked_malloc(sizeof(struct token));
      new_token->type = ENDOFFILE;
      new_token->next = NULL;
      new_token->word = NULL;
      break;
    case '#':
      //will be a comment?
      new_token = (struct token*)checked_malloc(sizeof(struct token));
      new_token->type = COMMENT;
      new_token->next = NULL;
      new_token->word = get_comment(get_next_byte, get_next_byte_arg);
      break;
    default:
      //now words and comments (?) are processed
      if(is_word(next_byte))
      {
	new_token = (struct token*)checked_malloc(sizeof(struct token));
	(new_token)->type = WORD;
	(new_token)->next = NULL;
	(new_token)->word = get_word(get_next_byte, get_next_byte_arg, next_byte);
      }
      else
      {
	fprintf(stderr, "%d: Invalid syntax\n", line_number);
	exit(1);
	// error
      }
      break;
  }
  return new_token; //is this correct?
}

struct token_stream make_token_stream(int (*get_next_byte) (void *), void *get_next_byte_arg)
{
  struct token_stream tokens;
  tokens.head = NULL;
  tokens.tail = NULL;
  struct token* current_token = get_next_token(get_next_byte, get_next_byte_arg);
  struct token* next_token;

  tokens.head = current_token;
  tokens.tail = current_token;

  while((current_token)->type != ENDOFFILE)
  {
    fprintf(stdout, "TOKEN %d FOUND\n", current_token->type);
    next_token = get_next_token(get_next_byte, get_next_byte_arg);
    (current_token)->next = next_token; // dereferencing?????
    tokens.tail = next_token;    
    current_token = next_token;
  }
  (tokens.tail)->next = NULL;

  fprintf(stdout, "Lines: %d\n", line_number-1);
  return tokens;
}
/*
command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
  // FIXME: Replace this with your implementation.  You may need to
  // add auxiliary functions and otherwise modify the source code.
  // You can also use external functions defined in the GNU C Library.  
  // need to create a combine_command function
  // command/operator stack
  error (1, 0, "command reading not yet implemented");
  return 0;
}

command_t
read_command_stream (command_stream_t s)
{
  // FIXME: Replace this with your implementation too.  
  error (1, 0, "command reading not yet implemented");
  return 0;
}
*/
