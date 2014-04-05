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
  struct command_t *command;
  struct command_node *next;
};

struct command_stream
{
  struct command_node *head;
  struct command_node *tail;
};
