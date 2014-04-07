// UCLA CS 111 Lab 1 command internals

enum command_type
{
  AND_COMMAND,         // A && B
  SEQUENCE_COMMAND,    // A ; B
  OR_COMMAND,          // A || B
  PIPE_COMMAND,        // A | B
  SIMPLE_COMMAND,      // a simple command
  SUBSHELL_COMMAND,    // ( A )
};

enum token_type
{
  AND,         // 0   &&
  OR,          // 1   ||
  PIPE,        // 2   |
  SEMICOLON,   // 3   ;
  LEFT_PAREN,  // 4   (
  RIGHT_PAREN, // 5   )
  INPUT,       // 6   <
  OUTPUT,      // 7   >
  WORD,        // 8   a word
  COMMENT,     // 9   # followed by zero or more words
  NEWLINE,     // 10  \n
  ENDOFFILE,   // 11  EOF
};

// Data associated with a command.
struct command
{
  enum command_type type;

  // Exit status, or -1 if not known (e.g., because it has not exited yet).
  int status;

  // I/O redirections, or null if none.
  char *input;
  char *output;

  union
  {
    // for AND_COMMAND, SEQUENCE_COMMAND, OR_COMMAND, PIPE_COMMAND:
    struct command *command[2];

    // for SIMPLE_COMMAND:
    char **word;

    // for SUBSHELL_COMMAND:
    struct command *subshell_command;
  } u;
};

struct token
{
  enum token_type type;
  char* word;
  struct token *next;
};

struct token_stream
{
  struct token *head;
  struct token *tail;
};

/*struct command_stream
{
  command_t *cmd_stream; //pointer to beginning of stream
  int num_commands; //number of command in the array
  int curr_index; //the index of the current command
  int size; //the total size of the array, changes with reallocation
}*/

struct command_node
{
  struct command *command;
  struct command_node *next;
};

struct command_stream
{
  struct command_node *head;
  struct command_node *tail;
};

struct command_stack_node
{
  struct command* cmd;
  struct command_stack_node* prev;
};

struct command_stack
{
  struct command_stack_node *top;
  struct command_stack_node *bottom;
};

struct operator_stack_node
{
  struct token* op;
  struct operator_stack_node* prev;
};

struct operator_stack
{
  struct operator_stack_node *top;
  struct operator_stack_node *bottom;
};

