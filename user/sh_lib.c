#include "user/sh_lib.h"
#include "kernel/defs.h"
#include "kernel/fcntl.h"
#include "user/sh_util.h"
#include "user/user.h"

void run_proxy(struct cmd *cmd)
{
    if (run_cmd(cmd) != 0) {
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}

// Do magic ╰( ͡° ͜ʖ ͡° )つ──☆*:
int run_cmd(struct cmd *cmd)
{
    // TODO: Pointer to struct could be invalid, perform a check with output according to the problem statement.
	
  	// EXEC is the only command type right now.
    if (cmd->type == EXEC) {
        // TODO: Cast the command to exec_cmd.
        // TODO: Call exec and handle errors to the problem statement. Keep in mind that exec does not return if it is successful.
        // Hint: the struct exec_cmd defined in sh_lib.h already contains all the necessary arguments for calling exec.
      	return EXIT_FAILURE;
    }
    
    fprintf(STD_ERR, "Unknown command type\n");
    // Note: run_cmd should return 0 (EXIT_SUCCESS) for success, and 1 (EXIT_FAILURE) for failure.
    return EXIT_FAILURE;
}

// "Constructor" for a cmd of type EXEC
struct cmd*
exec_cmd(void)
{
    struct exec_cmd* cmd = malloc(sizeof(*cmd));

    if (!cmd) {
        return NULL;
    }

    memset(cmd, 0, sizeof(*cmd));
    cmd->type = EXEC;
    return (struct cmd*) cmd;
}

// Executes the shell forever
void run_shell()
{
    static char buf[BUFFER_SIZE];
    
    // Read and run input commands.
    while (getcmd(buf, sizeof(buf)) >= 0) {
        // Before calling run_cmd we check for commands that the shell itself can execute
        // If the user enters chdir, switch directories,
        if (!strncmp(buf, "cd ", 3)) {
            // Chdir must be called by the parent, not the child.
            buf[strlen(buf) - 1] = 0;  // chop \n

            if (chdir(buf + 3) < 0) {
                fprintf(STD_ERR, "cannot cd %s\n", buf + 3);
            }
            continue;
        }

        if (fork1() == 0) {
            // Otherwise we create a child and use it to execute the given command
            run_proxy(parsecmd(buf));
        }

        wait(0);
    }
}

// Prints the prompt and waits for user input
int getcmd(char* buf, int nbuf)
{
    fprintf(STD_ERR, "$ ");
    memset(buf, 0, nbuf);
    gets(buf, nbuf);

    if (buf[0] == 0) {  // EOF
        return -1;
    }

    return EXIT_SUCCESS;
}

// Ensure that three file descriptors are open
void check_fds()
{
    int fd;

    while ((fd = open("console", O_RDWR)) >= 0) {
        if (fd >= 3) {
            close(fd);
            break;
        }
    }
}

// Fork with panic in case of an error
int fork1(void)
{
    int pid;

    pid = fork();

    if (pid == -1) {
        panic("fork");
    }
    return pid;
}

void panic(char* s)
{
    fprintf(STD_ERR, "%s\n", s);
    exit(EXIT_FAILURE);
}
