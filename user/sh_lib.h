#pragma once

#include "kernel/types.h"

#define MAXARGS 10
#define BUFFER_SIZE 100 // Maximum input length

// A command can only have the types EXEC or INVALID
#define INVALID 0
#define EXEC    1

// Parsed command representation
struct cmd {
  int type;
};

struct exec_cmd {
  int type;
  char *argv[MAXARGS];
  char *eargv[MAXARGS];
};

int getcmd(char *buf, int nbuf);
int fork1(void);  // Fork but panics on failure.
int run_cmd(struct cmd *cmd); // Performs an action for a given command

void run_proxy(struct cmd* cmd);
void run_shell();
void check_fds(); // Checks availability of file descriptors
void panic(char*);

struct cmd *exec_cmd(void); // Builds an exec command
