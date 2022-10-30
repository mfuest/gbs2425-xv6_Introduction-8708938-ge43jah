int gettoken(char **ps, char *es, char **q, char **eq);
int peek(char **ps, char *es, char *toks);
int strncmp(const char *p, const char *q, int n);

struct cmd *parseline(char **ps, char *es);
struct cmd *parsecmd(char*); // Builds a struct containing all the input data from the user input
struct cmd *parseexec(char**, char*); // Parses a command that can be used to execute a command
struct cmd *nulterminate(struct cmd*);
