#include "user/sh_lib.h"
#include "user/user.h"

// Entry point: Check file descriptors and execute the shell loop
int
main(void)
{
  check_fds();
  run_shell();
  exit(0);
}
