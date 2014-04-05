// UCLA CS 111 Lab 1 command interface

#include <stdbool.h>

typedef struct command* command_t;
typedef struct command_stream* command_stream_t;
/*typedef struct token* token_t;
typedef struct token_stream* token_stream_t;*/
typedef struct command_node* command_node_t;
typedef struct command_stream* command_stream_t;
typedef enum command_type* command_type_t;

int precedence(command_type_t op);
int is_word(int token);
char* get_word(int (*get_next_byte) (void *), void *get_next_byte_arg, int first_ch);
int is_comment(int token);
char* get_comment(int (*get_next_byte) (void *), void *get_next_byte_arg);
struct token* get_next_token(int (*get_next_byte) (void *), void *get_next_byte_arg);
struct token_stream make_token_stream(int (*get_next_byte) (void *), void *get_next_byte_arg);
command_stream_t make_command_stream (int (*get_next_byte) (void *), void *get_next_byte_argument);
command_t read_command_stream (command_stream_t s);




/* Create a command stream from GETBYTE and ARG.  A reader of
   the command stream will invoke GETBYTE (ARG) to get the next byte.
   GETBYTE will return the next input byte, or a negative number
   (setting errno) on failure.  */
command_stream_t make_command_stream (int (*getbyte) (void *), void *arg);

/* Read a command from STREAM; return it, or NULL on EOF.  If there is
   an error, report the error and exit instead of returning.  */
command_t read_command_stream (command_stream_t stream);

/* Print a command to stdout, for debugging.  */
void print_command (command_t);

/* Execute a command.  Use "time travel" if the flag is set.  */
void execute_command (command_t, bool);

/* Return the exit status of a command, which must have previously
   been executed.  Wait for the command, if it is not already finished.  */
int command_status (command_t);
