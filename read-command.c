// UCLA CS 111 Lab 1 command reading

#include "stdlib.h"
#include "stdio.h"
#include "command.h"
#include "command-internals.h"
#include "alloc.h"
#include <error.h>

int line_number = 1;

struct command_stack_node* push_command (struct command_stack *stack, struct token* tok, struct command* cmd)
{
  if((cmd == NULL && tok == NULL) || (cmd != NULL && tok != NULL))
  {
    fprintf(stderr, "Error in pushing to command stack.\n");
    exit(1);    
  }
  
  struct command_stack_node* newtop = (struct command_stack_node*) checked_malloc(sizeof(struct command_stack_node));
  
  int current_word = 0;
  
  /* Input is alread a command, just assign it to a stack node and push it
     onto the stack. */
  if(cmd != NULL && tok == NULL)
  {
    newtop->cmd = cmd;
    newtop->prev = stack->top;
    stack->top = newtop;
    // to debug, uncomment
    // fprintf(stderr, "Pushed a command.\n");
  }

  /* Input is a token and needs to be converted into a command within a
     command_stack_node */
  else if(cmd == NULL && tok != NULL)
  {
    // to debug, uncomment
    // fprintf(stderr, "Pushed a command (%d).\n", tok->type);
    int num_word = 5; // if current word = num_word, realloc
    newtop->cmd = (struct command*) checked_malloc(sizeof(struct command));
    newtop->cmd->u.word = (char **) checked_malloc(num_word * sizeof(char *));

    if(tok->type == WORD)
    {
      newtop->cmd->type = SIMPLE_COMMAND;
      newtop->cmd->status = -1;
      newtop->cmd->input = NULL;
      newtop->cmd->output = NULL;
      newtop->cmd->u.word[current_word] = tok->word;
      current_word = current_word + 1;
      newtop->prev = stack->top;
      stack->top = newtop;
    }
    tok = tok->next;
    /* Reads consecutive word commands if they exist. */
    while(tok->type == WORD)
    {
      if(current_word == num_word)
      {
        num_word *= 2;
        newtop->cmd->u.word = (char **) checked_realloc(newtop->cmd->u.word, num_word * sizeof(char *));
      }
      newtop->cmd->u.word[current_word] = tok->word;
      current_word = current_word + 1;
      tok = tok->next;
    }
    newtop->cmd->u.word[current_word] = NULL;
  }
  return stack->top;
}

struct command* pop_command (struct command_stack *stack)
{
  if(stack->top == NULL)
  {
    fprintf(stderr, "%d: Cannot pop empty command stack.\n", line_number-1);
    exit(1);
  }
  struct command_stack_node *temp = stack->top;
  struct command* cmd = temp->cmd;
  stack->top = stack->top->prev;
  free(temp);
  // to debug, uncomment
  // fprintf(stderr, "Popped a command(%d).\n", cmd->type);
  return cmd;
}

struct operator_stack_node* push_operator(struct operator_stack* stack, struct token* tok)
{
  struct operator_stack_node* newtop = (struct operator_stack_node*) malloc(sizeof(struct operator_stack_node));
  newtop->op = tok;
  newtop->prev = stack->top;
  stack->top = newtop;
  // to debug, uncomment
  // fprintf(stderr, "Pushed an operator (%d).\n", tok->type);
  return stack->top;
}

struct token* pop_operator (struct operator_stack *stack)
{
  if(stack->top == NULL)
  {
    fprintf(stderr, "%d: Cannot pop empty operator stack.\n", line_number-1);
    exit(1);
  }
  struct operator_stack_node *temp = stack->top;
  struct token* op = temp->op;
  stack->top = stack->top->prev;
  free(temp);
  // to debug, uncomment
  // fprintf(stderr, "Popped an operator (%d).\n", op->type);
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
  // to debug, uncomment
  // fprintf(stderr, "Combined a command.\n");
  return new_command;
}

int precedence(enum token_type op)
{
  switch(op)
  {
    case SEMICOLON:
    case NEWLINE:
      return 0;
    case AND:
    case OR:
      return 1;
    case PIPE:
      return 2;
    case LEFT_PAREN:
    case RIGHT_PAREN:
      return 3;
    default:
      fprintf(stderr, "Error in precedence function!\n");
      exit(1);
  }
  return 0;
}

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
        new_token->type = AND;
        new_token->next = NULL;
        new_token->word = NULL;
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
        new_token->word = NULL;
      }
      else
      {
        new_token = (struct token*)checked_malloc(sizeof(struct token));
        new_token->type = PIPE;
        new_token->next = NULL;
        new_token->word = NULL;
        ungetc(next_byte, get_next_byte_arg); // return the byte read
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
      if(next_byte != '\n' && next_byte != '>' && next_byte != '<')
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
      if(next_byte != '\n' && next_byte != '>' && next_byte != '<')
      {
        new_token = (struct token*)checked_malloc(sizeof(struct token));
        new_token->type = INPUT;
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
  // to debug, uncomment
  // int i = 0;
  
  while((current_token)->type != ENDOFFILE)
  {
    // to debug, uncomment
    // fprintf(stderr, "TOKEN %d FOUND --> number %d\n", current_token->type, i);
    // i++;
    next_token = get_next_token(get_next_byte, get_next_byte_arg);
    current_token->next = next_token;
    tokens.tail = next_token;
    current_token = next_token;
  }
  (tokens.tail)->next = NULL;

  line_number = 1;
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

void process_operator(struct token* next_operator, struct command_stack* cmd_stack, struct operator_stack* op_stack)
{
  struct token* current_operator = NULL;
  struct command* first_command = NULL;
  struct command* second_command = NULL;
  struct command* new_command = NULL;

  if(op_stack->top == NULL) // if operator stack is empty
  {
    push_operator(op_stack, next_operator);
  }
  else
  {
    if(precedence(next_operator->type) > precedence(op_stack->top->op->type))
    {
       push_operator(op_stack, next_operator);
    }
    else
    {
      while((op_stack->top->op->type) != LEFT_PAREN &&
            precedence(next_operator->type) <= precedence(op_stack->top->op->type))
      {
        current_operator = pop_operator(op_stack);
        second_command = pop_command(cmd_stack);
        first_command = pop_command(cmd_stack);
        new_command = combine_command(first_command, second_command, current_operator);
        push_command(cmd_stack, NULL, new_command);
        if(op_stack->top == NULL)
          break;
      }
      push_operator(op_stack, next_operator);
    }
  }
}

command_stream_t
make_command_stream (int (*get_next_byte) (void *),
                     void *get_next_byte_argument)
{
  // for error handling
  line_number = 1;

  struct command_stream* cmd_stream = (struct command_stream*) checked_malloc(sizeof(struct command_stream));
  cmd_stream->head = NULL;
  cmd_stream->tail = NULL;
  cmd_stream->current = cmd_stream->head;
  cmd_stream->num_commands = 0; //increment in this function
  cmd_stream->num_commands_read = 0; //only incremented in read_command_stream

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
  struct token* prev_token = NULL;
  
  if(current_token == NULL)
  {
    fprintf(stderr, "Empty command stream.\n");
    exit(1);
  }

  // initialize an empty command stream node tree
  cmd_stream->current = (struct command_node*)checked_malloc(sizeof(struct command_node));
  cmd_stream->head = cmd_stream->current;
  cmd_stream->tail = cmd_stream->current;
  cmd_stream->num_commands++;

  // when need to create new tree, pop everything off of stacks, process 

  while(current_token != NULL)
  {
    switch(current_token->type)
    {
      case AND:
      case OR:
      case PIPE:
      case SEMICOLON:
        process_operator(current_token, commands, operators);
        prev_token = current_token;
        current_token = current_token->next;
        break;
      case LEFT_PAREN:
        push_operator(operators, current_token);
        prev_token = current_token;
        current_token = current_token->next;
        break;
      case RIGHT_PAREN:
        top_operator = pop_operator(operators);
        while(top_operator->type != LEFT_PAREN) // explain this loop
        {
          second_command = pop_command(commands);
          first_command = pop_command(commands);
          new_command = combine_command(first_command, second_command, top_operator);
          push_command(commands, NULL, new_command); 
          top_operator = pop_operator(operators);
        }
        struct command* subshell_command = (struct command*)checked_malloc(sizeof(struct command));
        subshell_command->type = SUBSHELL_COMMAND;
        subshell_command->status = -1;
        subshell_command->input = NULL;
        subshell_command->output = NULL;
        subshell_command->u.subshell_command = pop_command(commands);
        push_command(commands, NULL, subshell_command);
        prev_token = current_token;
        current_token = current_token->next;
        break;
      case INPUT:
        top_command = pop_command(commands);
        if(current_token->next->type != WORD)
        {
          fprintf(stderr, "%d: Invalid input to command: not a word.\n", line_number);
          exit(1);
        }
        top_command->input = current_token->next->word; //if not a word, error
        // to debug, uncomment
        // fprintf(stderr, "Added input to command.\n");
        push_command(commands, NULL, top_command);
        if(current_token->next == NULL)
        {
          fprintf(stderr, "%d: Invalid input to command.\n", line_number);
          exit(1);
        }
        prev_token = current_token->next;
        current_token = current_token->next->next; // skip the next command
        break;
      case OUTPUT:
        top_command = pop_command(commands);
        if(current_token->next->type != WORD)
        {
          fprintf(stderr, "%d: Invalid output to command: not a word.\n", line_number);
          exit(1);
        }
        top_command->output = current_token->next->word;  //if not a word,error
        // to debug, uncomment
        // fprintf(stderr, "Added output to command.\n");
        push_command(commands, NULL, top_command);
        if(current_token->next == NULL)
        {
          fprintf(stderr, "%d: Invalid output to command.\n", line_number);
          exit(1);
        }
        prev_token = current_token->next;
        current_token = current_token->next->next; // skip the next command
        break;
      case WORD:
        push_command(commands, current_token, NULL);
        prev_token = current_token;
        current_token = current_token->next;
        while(current_token->type == WORD) // skip the words already read
        {
          prev_token = current_token;
          current_token = current_token->next;
        }
        break;
      case NEWLINE:
        // already processed one or more tokens
        if(prev_token != NULL)
        {
          if(prev_token->type == INPUT || prev_token->type == OUTPUT)
          {
            fprintf(stderr, "%d: Invalid syntax.\n", line_number);
            exit(1);
          }
          // if it separates a complete command followed by an operator, or open parenthesis
          if(prev_token->type == AND || prev_token->type == OR ||
             prev_token->type == PIPE || prev_token->type == LEFT_PAREN)
          {
            // increment current token to next non-newline token
            while(current_token->type == NEWLINE || current_token->type == COMMENT)
            {
              prev_token = current_token;
              current_token = current_token->next;
              line_number++;
            }
            // cannot have a command be EOF
            if(current_token->type == ENDOFFILE || current_token->type == AND ||
               current_token->type == OR || current_token->type == PIPE) 
            {
              fprintf(stderr, "%d: Invalid syntax.\n", line_number);
              exit(1);
            }
          }
          // is a sequence
          else if(prev_token->type == SEMICOLON || (current_token->next->type != NEWLINE && current_token->next->type != COMMENT))
          {
            // read in until next non-newline token
            // prev_token will be stored as newline

            // check if already pushed a semicolon operator
            bool isAlreadySequence = false;
            if(prev_token->type == SEMICOLON)
            {
              isAlreadySequence = true;
            }
            
            // increment to the next non-newline token
            while(current_token->type == NEWLINE || current_token->type == COMMENT)
            {
              prev_token = current_token;
              current_token = current_token->next;
              line_number++;
            }
            
            // if it is a right parentheses, DO NOT process the sequence command
            if(current_token->type == RIGHT_PAREN)
            {
              break;
            }
            
            // if end of file, process 
            if(current_token->type == ENDOFFILE)
            {
              // remove the semicolon from processing
              // since we will only process the first command in the "sequence"
              if(isAlreadySequence)
              {
                pop_operator(operators);
                // if the left hand side of semicolon has no command, error
                if(commands->top == NULL)
                {
                  fprintf(stderr, "%d: Invalid syntax, must have command prior to semicolon\n", line_number);
                  exit(1);
                }
              }
              while(operators->top != NULL)
              {
                top_operator = pop_operator(operators);
                second_command = pop_command(commands);
                first_command = pop_command(commands);
                new_command = combine_command(first_command, second_command, top_operator);
                push_command(commands, NULL, new_command);
              }
              if(commands->top != NULL) // there must be a command
              {
                cmd_stream->tail->cmd = pop_command(commands);
                cmd_stream->tail->next = NULL;
              }
              break;
            }

            // process the newline as a sequence
            if(!isAlreadySequence)
            {
              process_operator(prev_token, commands, operators);
            }

            if(current_token->type == AND || current_token->type == OR ||
               current_token->type == PIPE || current_token->type == SEMICOLON) //perhaps more cases?
            {
              fprintf(stderr, "%d: Invalid syntax.\n", line_number);
              exit(1);
            }
          }
          else  //Mutiple newlines after a complete command
          {
            while(current_token->type == NEWLINE || current_token->type == COMMENT)
            {
              // increment current pointer to next non newline
              prev_token = current_token;
              current_token = current_token->next;
              line_number++;
            }
            // end of file
            if(current_token->type == ENDOFFILE)
            {
              // combine the last commands remaining on the stack
              while(operators->top != NULL)
              {
                top_operator = pop_operator(operators);
                second_command = pop_command(commands);
                first_command = pop_command(commands);
                new_command = combine_command(first_command, second_command, top_operator);
                push_command(commands, NULL, new_command);
              }
              if(commands->top != NULL) // there is a command on the stack - not just a ton of newlines
              {
                cmd_stream->tail->cmd = pop_command(commands);
                cmd_stream->tail->next = NULL;
              }
              break;
            }

            if(current_token->type == AND || current_token->type == OR ||
               current_token->type == PIPE || current_token->type == SEMICOLON) //perhaps more cases?
            {
              fprintf(stderr, "%d: Invalid syntax. Newline cannot follow operator.\n", line_number - 1);
              exit(1);
            }
                
            // if it is a right parentheses, DO NOT process the sequence command
            if(current_token->type == RIGHT_PAREN)
            {
              break;
            }

            // combine the last commands remaining on the stacks
            while(operators->top != NULL)
            {
              top_operator = pop_operator(operators);
              second_command = pop_command(commands);
              first_command = pop_command(commands);
              new_command = combine_command(first_command, second_command, top_operator);
              push_command(commands, NULL, new_command);
            }
            
            // set current command node command pointer to top command
            cmd_stream->tail->cmd = pop_command(commands);
            
            // create new command node
            cmd_stream->tail->next = (struct command_node*)checked_malloc(sizeof(struct command_node));
            cmd_stream->tail = cmd_stream->tail->next;
            cmd_stream->tail->cmd = NULL;
            cmd_stream->tail->next = NULL;
            cmd_stream->num_commands++;
            // to debug, uncomment
            // fprintf(stderr, "Created new command node tree.\n");
          }
        }
        // case where file starts with 1 or more newlines
        else
        {
          while(current_token->type == NEWLINE || current_token->type == COMMENT)
          {
            // increment current pointer to next non newline
            prev_token = current_token;
            current_token = current_token->next;
            line_number++;
          }
        }
        break;
      case COMMENT:
        // ignore comments when processing commands
        // push_command(commands, current_token, NULL);
        // to debug, uncomment
        // fprintf(stderr, "%d: comment\n", line_number);

        // increment line number due to comment
        // line_number++;

        prev_token = current_token;
        current_token = current_token->next;

        // if followed by newline, skip over all following newlines
        while(current_token->type == NEWLINE)
        {
          prev_token = current_token;
          current_token = current_token->next;
          line_number++;
        }
        break;
      case ENDOFFILE:
        current_token = NULL;
        break;
      default:
        fprintf(stderr, "%d: Syntax error.\n", line_number);
        exit(1);
    }
  }

  // if the stacks still contain some objects, finish creating the command stream
  while(operators->top != NULL)
  {
    top_operator = pop_operator(operators);
    second_command = pop_command(commands);
    first_command = pop_command(commands);
    new_command = combine_command(first_command, second_command, top_operator);
    push_command(commands, NULL, new_command); 
  }
  cmd_stream->tail->next = NULL;
  return cmd_stream;
}

command_t
read_command_stream (command_stream_t s)
{
  if(s->num_commands_read == s->num_commands)
  {
    return NULL;
  }

  struct command *cmd = s->current->cmd;
  s->current = s->current->next;
  s->num_commands_read++;

  return cmd;
}
