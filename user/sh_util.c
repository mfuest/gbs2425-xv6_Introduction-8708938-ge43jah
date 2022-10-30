
#include "user/sh_util.h"
#include "user/sh_lib.h"
#include "kernel/defs.h"
#include "user/user.h"
#include "kernel/fcntl.h"

// Basic parsing utilities. You can take a at how the parsing works, but it's not the focus of this exercise
char whitespace[] = " \t\r\n\v";
char symbols[] = "<|>&;()";

int
strncmp(const char *p, const char *q, int n)
{
  while(n > 0 && *p && *p == *q)
    n--, p++, q++;
  
  if(n == 0)
    return 0;
  return (uchar)*p - (uchar)*q;
}

int
gettoken(char **ps, char *es, char **q, char **eq)
{
  char *s;
  int ret;

  s = *ps;
  while (s < es && strchr(whitespace, *s)) {
    s++;
  }
  if (q) {
    *q = s;
  }
  ret = *s;
  switch (*s) {
  case 0:
    break;
  case '|':
  case '(':
  case ')':
  case ';':
  case '&':
  case '<':
    s++;
    break;
  case '>':
    s++;
    if (*s == '>') {
      ret = '+';
      s++;
    }
    break;
  default:
    ret = 'a';
    while (s < es && !strchr(whitespace, *s) && !strchr(symbols, *s))
      s++;
    break;
  }
  if (eq)
    *eq = s;

  while (s < es && strchr(whitespace, *s)) {
    s++;
  }
  *ps = s;
  return ret;
}

int
peek(char **ps, char *es, char *toks)
{
  char *s;

  s = *ps;
  while (s < es && strchr(whitespace, *s)) {
    s++;
  }
  *ps = s;
  return *s && strchr(toks, *s);
}

struct cmd*
parsecmd(char *s)
{
  char *es;
  struct cmd *cmd;

  es = s + strlen(s);
  cmd = parseexec(&s, es);
  peek(&s, es, "");
  
  if (s != es) {
    fprintf(2, "leftovers: %s\n", s);
    panic("syntax");
  }
  nulterminate(cmd);
  return cmd;
}

struct cmd*
parseline(char **ps, char *es)
{
  struct cmd *cmd;

  cmd = parseexec(ps, es);

  return cmd;
}

struct cmd*
parseexec(char **ps, char *es)
{
  char *q, *eq;
  int tok, argc;
  struct exec_cmd *cmd;
  struct cmd *ret;

  ret = exec_cmd();
  cmd = (struct exec_cmd*)ret;

  argc = 0;

  while (!peek(ps, es, "|)&;")) {
    if ((tok=gettoken(ps, es, &q, &eq)) == 0) {
      break;
    }

    if (tok != 'a') {
      panic("syntax");
    }
    cmd->argv[argc] = q;
    cmd->eargv[argc] = eq;
    argc++;

    if (argc >= MAXARGS) {
      panic("too many args");
    }
  }
  cmd->argv[argc] = 0;
  cmd->eargv[argc] = 0;
  return ret;
}

// NUL-terminate all the counted strings.
struct cmd*
nulterminate(struct cmd *cmd)
{
  struct exec_cmd *ecmd = 0;

  if (cmd == INVALID) {
    return 0;
  }

  // Can be expanded later on
  switch(cmd->type) {
  case EXEC: 
    ecmd = (struct exec_cmd*)cmd;
    for (int i = 0; ecmd->argv[i]; i++) {
      *ecmd->eargv[i] = 0;
    }
    break;
  }

  return cmd;
}
