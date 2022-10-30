#include "kernel/types.h"
#include "kernel/defs.h"
#include "kernel/stat.h"
#include "user/sh_lib.h"
#include "user/user.h"

void test1() {
    struct cmd *cmd = INVALID;
    if (run_cmd(cmd) != 1) {
        fprintf(1, "Received wrong return code\n");
    }
}

void test2() {
    struct cmd *cmd = malloc(sizeof(*cmd));
    
    if (!cmd) {
        return;
    }

    // Could use a random generator here, but that would be a bit overkill
    for (int i = 4; i < 10; i++) {
        cmd->type = i;
        if (run_cmd(cmd) != 1) {
            fprintf(1, "Received wrong return code\n");
            break;
        };
    }

    free(cmd);
}

int main(void) {
    test1();
    test2();
    exit(0);
}
