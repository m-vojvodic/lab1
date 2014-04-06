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

struct command_stack_node* push_command (struct command_stack *stack, struct token* tok, struct command* cmd)
{
  if((cmd == NULL && tok == NULL) || (cmd != NULL && tok != NULL))
  {
	fprintf(stderr, "Error in pushing to command stack.\n");
	exit(1);	
  }

  struct command_stack_node* newtop = (struct command_stack_node*) checked_malloc(sizeof(struct command_stack_node));

  if(cmd != NULL && tok == NULL) //input is alread a command, just assign it to a stack node and push it onto the stack
  {
	newtop->cmd = cmd;
	newtop->prev = stack->top;
	stack->top = newtop;
  }
  else if(cmd == NULL && tok != NULL)  //input is a token and needs to be converted into a command within a command_stack_node
  {	
	int current_word = 0;
	if(tok->type == WORD)
	{
  	  newtop->cmd = (struct command*) checked_malloc(sizeof(struct command));
	  newtop->cmd->type = SIMPLE_COMMAND;
	  newtop->cmd->status = -1;
	  newtop->cmd->input = NULL;
	  newtop->cmd->output = NULL;
	  newtop->cmd->u.word[current_word++] = tok->word;
  	  newtop->prev = stack->top;
  	  stack->top = newtop;
	}
	tok = tok->next;
	while(tok->type == WORD)  //reads consecutive word commands if they exist
	{
	  newtop->cmd->u.word[current_word++] = tok->word;
	  tok = tok->next;
	}
	newtop->cmd->u.word[current_word++] = '\0';
  }
  return stack->top;
}

//pop the top command off of the command stack
struct command* pop_command (struct command_stack *stack)
{
  struct command_stack_node *temp = stack->top;
  struct command* cmd = temp->cmd;
  stack->top = stack->top->prev;
  free(temp);
  return cmd;
}

struct operator_stack_node* push_operator(struct operator_stack* stack, struct token* tok)
{
  struct operator_stack_node* newtop = (struct operator_stack_node*) malloc(sizeof(struct operator_stack_node));
  newtop->op = tok;
  newtop->prev = stack->top;
  stack->top = newtop;
  return stack->top;
}

struct token* pop_operator (struct operator_stack *stack)
{
  struct operator_stack_node *temp = stack->top;
  struct token* op = temp->op;
  stack->top = stack->top->prev;
  free(temp);
  return op;
}


/*struct command_node insert_command_node (struct command_node *tail, struct command cmd)
{
  struct command_node *node = (struct command_node*)checked_malloc(sizeof(struct command_node));
  node->command = &cmd;
  node->next = NULL;
  tail->next = node;
  tail = node;
}*/

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
  return 0;
}

//checks if the next byte is a valid character for a word
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

//creates and returns a char array to be used as a token for commands or file names
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

//creates and returns a char array comprised of an entire line following '#'
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

void free_token_stream(struct token_stream stream)
{
	if(stream.head == NULL) //empty list
	{
		return;
	}
	struct token* current = stream.head;
	if(current->next == NULL) //one token in list
	{
		if(current->word != NULL)
		{
			free(current->word);
		}
		free(current);
		return;
	}
	struct token* next_token = current->next;
	while(next_token != NULL) //regular case
	{
		if(current->word != NULL)
		{
			free(current->word);
		}
		free(current);
		current = next_token;
		next_token = current->next;
	}
	if(current->word != NULL) //removes last token
	{
		free(current->word);
	}
	free(current);
	return;
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

command_t  //need to change to struct command
read_command_stream (command_stream_t s)
{
  // FIXME: Replace this with your implementation too.  
  error (1, 0, "command reading not yet implemented");
  return 0;
}
*/
