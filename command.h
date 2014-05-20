// UCLA CS 111 Lab 1 command interface

#include <stdbool.h>

typedef struct command* command_t;
typedef struct command_stream* command_stream_t;
typedef struct token* token_t;
typedef struct token_stream* token_stream_t;
typedef struct command_node* command_node_t;
typedef struct command_stack_node* command_stack_node_t;
typedef struct command_stack* command_stack_t;
typedef struct operator_stack_node* operator_stack_node_t;
typedef struct operator_stack* operator_stack_t;
typedef enum command_type* command_type_t;
typedef enum token_type* token_type_t;

/* Push commmand onto command stack and return the top command on stack.
   If push a token, creates a new command from that token. */
struct command_stack_node* push_command (struct command_stack *stack, struct token* tok, struct command* cmd);

/* Pop command off of command stack and return command. */
struct command* pop_command (struct command_stack *stack);

/* Push operator onto operator stack and return the top operator on stack. */
struct operator_stack_node* push_operator(struct operator_stack* stack, struct token* tok);

/* Pop operator off of operator stack and return operator. */
struct token* pop_operator (struct operator_stack *stack);

/* Combine two pre-existing commands with a token operator to create a new
   command and return it. */
struct command* combine_command(struct command* first, struct command* second, struct token* op);

/* Process operators to create commands. */
void process_operator(struct token* next_operator, struct command_stack* cmd_stack, struct operator_stack* op_stack);

/* Evaluates operator precedence. */
int precedence(enum token_type op);

/* Evaluates if character is valid for words. */
int is_word(int token);

/* Reads and returns the next word in command sequence as a character array. */
char* get_word(int (*get_next_byte) (void *), void *get_next_byte_arg, int first_ch);

/* Reads and returns an entire comment line as a character array. */
char* get_comment(int (*get_next_byte) (void *), void *get_next_byte_arg);

/* Reads and returns the next token of the command stream, whether it be an
   operator, word, or comment. */
struct token* get_next_token(int (*get_next_byte) (void *), void *get_next_byte_arg);

/* Create a token stream from input to be used for the creation of commands
   for the implementation of command stream. Parses input stream and stores
   operators, words, or comments in the order they appear in the input. */
struct token_stream make_token_stream(int (*get_next_byte) (void *), void *get_next_byte_arg);

/* Frees the memory allocated by the token stream objects. */
void free_token_stream(struct token_stream stream);

/* Create a command stream from GETBYTE and ARG.  A reader of
   the command stream will invoke GETBYTE (ARG) to get the next byte.
   GETBYTE will return the next input byte, or a negative number
   (setting errno) on failure.  */
command_stream_t make_command_stream (int (*getbyte) (void *), void *arg);

/* Read a command from STREAM; return it, or NULL on EOF.  If there is
   an error, report the error and exit instead of returning.  */
command_t read_command_stream (command_stream_t stream);

/* Print a command to stdout, for debugging.  */
void print_command (command_t c);

/* Execute a command.  Use "time travel" if the flag is set.  */
void execute_command (command_t c, bool time_travel);

/* Return the exit status of a command, which must have previously
   been executed.  Wait for the command, if it is not already finished.  */
int command_status (command_t c);

