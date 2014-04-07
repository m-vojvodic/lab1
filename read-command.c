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

  /* Input is alread a command, just assign it to a stack node and push it
     onto the stack. */
  if(cmd != NULL && tok == NULL)
  {
    newtop->cmd = cmd;
    newtop->prev = stack->top;
    stack->top = newtop;
  }

  /* Input is a token and needs to be converted into a command within a
     command_stack_node */
  else if(cmd == NULL && tok != NULL)
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
    tok = tok->next;          // Pass by reference?
    /* Reads consecutive word commands if they exist. */
    while(tok->type == WORD)
    {
      newtop->cmd->u.word[current_word++] = tok->word;
      tok = tok->next;
    }
    newtop->cmd->u.word[current_word++] = '\0';
  }
  return stack->top;
}

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

struct command* combine_command(struct command* first, struct command* second, struct token* op)
{
  if(first == NULL || second == NULL || op == NULL)
  {
    fprintf(stderr, "Error in combine_command: Invalid input to function.\n");
    exit(1);
  }

  struct command* new_command = (struct command*)checked_malloc(sizeof(struct command));

  switch(op->type)
  {
    case AND:
      new_command->type = AND_COMMAND;
      new_command->u.command[0] = first;
      new_command->u.command[1] = second;
      break;
    case OR:
      new_command->type = OR_COMMAND;
      new_command->u.command[0] = first;
      new_command->u.command[1] = second;
      break;
    case PIPE:
      new_command->type = PIPE_COMMAND;
      new_command->u.command[0] = first;
      new_command->u.command[1] = second;
      break;
    case SEMICOLON:
      /* In the case of a single newline */
    case NEWLINE:
      new_command->type = SEQUENCE_COMMAND;
      new_command->u.command[0] = first;
      new_command->u.command[1] = second;
      break;
    default:
      fprintf(stderr, "Error in combine_command: Invalid input operator.\n");
      exit(1);
      break;
  }
  return new_command;
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
    case '&': // AND
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
    case '|': // PIPE or OR
      next_byte = get_next_byte(get_next_byte_arg);
      if(next_byte == '|')
      {
	new_token = (struct token*)checked_malloc(sizeof(struct token));
	new_token->type = OR;
	new_token->next = NULL;
	new_token->word = NULL; //do we need to store a word for special characters?
      }
      else
      {
	new_token = (struct token*)checked_malloc(sizeof(struct token));
	new_token->type = PIPE;
	new_token->next = NULL;
	new_token->word = NULL;
	ungetc(next_byte, get_next_byte_arg); // decrement filestream ptr
      }
      break;
    case '(': // LEFT_PAREN
      new_token = (struct token*)checked_malloc(sizeof(struct token));
      new_token->type = LEFT_PAREN;
      new_token->next = NULL;
      new_token->word = NULL;
      break;
    case ')': // RIGHT_PAREN
      new_token = (struct token*)checked_malloc(sizeof(struct token));
      new_token->type = RIGHT_PAREN;
      new_token->next = NULL;
      new_token->word = NULL;
      break;
    case ';': // SEMICOLON
      new_token = (struct token*)checked_malloc(sizeof(struct token));
      new_token->type = SEMICOLON;
      new_token->next = NULL;
      new_token->word = NULL;
      break;
    case '>': // OUTPUT
      next_byte = get_next_byte(get_next_byte_arg);
      if(next_byte != '\n')
      {
	new_token = (struct token*)checked_malloc(sizeof(struct token));
	new_token->type = OUTPUT;
	new_token->next = NULL;
	new_token->word = NULL;
	ungetc(next_byte, get_next_byte_arg); // return the byte read
      }
      else // invalid syntax
      {
	fprintf(stderr, "%d: Invalid syntax\n", line_number);
	exit(1);
      }
      break;
    case '<': // INPUT
      next_byte = get_next_byte(get_next_byte_arg);
      if(next_byte != '\n')
      {
	new_token = (struct token*)checked_malloc(sizeof(struct token));
	new_token->type = INPUT;
	new_token->next = NULL;
	new_token->word = NULL;
      }
      break;
    case '\n': // NEWLINE
	new_token = (struct token*)checked_malloc(sizeof(struct token));
	new_token->type = NEWLINE;
	new_token->next = NULL;
	new_token->word = NULL;
	line_number++;
      break;
    case EOF: // ENDOFFILE
      new_token = (struct token*)checked_malloc(sizeof(struct token));
      new_token->type = ENDOFFILE;
      new_token->next = NULL;
      new_token->word = NULL;
      break;
    case '#': // COMMENT
      new_token = (struct token*)checked_malloc(sizeof(struct token));
      new_token->type = COMMENT;
      new_token->next = NULL;
      new_token->word = get_comment(get_next_byte, get_next_byte_arg);
      break;
    default: //now WORDs are processed
      if(is_word(next_byte))
      {
	new_token = (struct token*)checked_malloc(sizeof(struct token));
	new_token->type = WORD;
	new_token->next = NULL;
	new_token->word = get_word(get_next_byte, get_next_byte_arg, next_byte);
      }
      else
      {
	fprintf(stderr, "%d: Invalid syntax\n", line_number);
	exit(1);
      }
      break;
  }
  return new_token;
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
    fprintf(stderr, "TOKEN %d FOUND\n", current_token->type); // for debugging
    next_token = get_next_token(get_next_byte, get_next_byte_arg);
    (current_token)->next = next_token;
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


command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
  // FIXME: Replace this with your implementation.  You may need to
  // add auxiliary functions and otherwise modify the source code.
  // You can also use external functions defined in the GNU C Library.  
  // need to create a combine_command function
  // command/operator stack

  struct command_stream* cmd_stream = (struct command_stream*) checked_malloc(sizeof(struct command_stream));
  cmd_stream->head = NULL;
  cmd_stream->tail = NULL;

  struct token_stream tokens = make_token_stream(get_next_byte, get_next_byte_argument);

  struct command_stack* commands = (struct command_stack*) checked_malloc(sizeof(struct command_stack));
  commands->top = NULL;
  commands->bottom = NULL;

  struct operator_stack* operators = (struct operator_stack*) checked_malloc(sizeof(struct operator_stack));
  operators->top = NULL;
  operators->bottom = NULL;

  struct token* top_operator;
  struct command* top_command;
  struct command* first_command;
  struct command* second_command;
  struct command* new_command;

  struct token* current_token = tokens.head;
  //struct token* next_token = current_token->next;
  
  while(current_token != NULL)
  {
	switch(current_token->type)
	{
	  case AND:
	  case OR:
      case PIPE:
	  case SEMICOLON:
	    //implement operator logic
		break;
	  case LEFT_PAREN:
		push_operator(operators, current_token);
		break;
	  case RIGHT_PAREN:
		//implement parenthesis logic
		top_operator = pop_operator(operators);
		while(top_operator->type != LEFT_PAREN)
		{
		  second_command = pop_command(commands);
		  first_command = pop_command(commands);
		  new_command = combine_command(first_command, second_command, top_operator);
		  push_command(commands, NULL, new_command); 
		}
		struct command* subshell_command = (struct command*)checked_malloc(sizeof(struct command));
		subshell_command->type = SUBSHELL_COMMAND;
		subshell_command->status = -1;
		subshell_command->input = NULL;
		subshell_command->output = NULL;
		subshell_command->u.subshell_command = pop_command(commands);
		push_command(commands, NULL, subshell_command);
		break;
	  case INPUT:
		top_command = pop_command(commands);
		top_command->input = current_token->next->word; //if not a word, error
		push_command(commands, NULL, top_command);
		break;
	  case OUTPUT:
		//implement redirect logic
		top_command = pop_command(commands);
		top_command->output = current_token->next->word;  //if not a word, error
		push_command(commands, NULL, top_command);
		break;
	  case WORD:
		//implement word logic
		push_command(commands, current_token, NULL);
		break;
	  case NEWLINE:
		//newline logic
		break;
	  case COMMENT:
		//????
		break;
	  case ENDOFFILE:
		//treat as its own command
	  default:
		//error at line number
		exit(1);
	}
  }
  return cmd_stream;
}

/*
command_t  //need to change to struct command
read_command_stream (command_stream_t s)
{
  // FIXME: Replace this with your implementation too.  
  error (1, 0, "command reading not yet implemented");
  return 0;
}
*/
