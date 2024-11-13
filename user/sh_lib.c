// sh_lib.c

#include "user/sh_lib.h"
#include "kernel/defs.h"
#include "kernel/fcntl.h"
#include "user/sh_util.h"
#include "user/user.h"
#include <string.h>
#include <stdlib.h>

// Proxy-Funktion zum Ausführen von Befehlen
void run_proxy(struct cmd *cmd)
{
    if (run_cmd(cmd) != 0) {
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}

// Implementierung der run_cmd Funktion
int run_cmd(struct cmd *cmd)
{
    // Überprüfen, ob der cmd-Zeiger gültig ist
    if (cmd == NULL) {
        fprintf(STD_ERR, "Received empty command\n");
        return EXIT_FAILURE;
    }

    // EXEC ist der einzige Befehlstyp
    if (cmd->type == EXEC) {
        struct exec_cmd *exec_cmd = (struct exec_cmd *)cmd;

        // Überprüfen, ob der Pfad zum Programm vorhanden ist
        if (exec_cmd->path == NULL || exec_cmd->path[0] == '\0') {
            fprintf(STD_ERR, "No program to execute\n");
            return EXIT_FAILURE;
        }

        // Aufruf von exec
        if (exec(exec_cmd->path, exec_cmd->argv) < 0) {
            fprintf(STD_ERR, "exec failed for %s\n", exec_cmd->path);
            return EXIT_FAILURE;
        }

        // exec kehrt nicht zurück, wenn erfolgreich
        return EXIT_FAILURE;
    }

    fprintf(STD_ERR, "Unknown command type\n");
    return EXIT_FAILURE;
}

// "Constructor" für einen EXEC-Befehl
struct cmd* exec_cmd_create(char *path, char **argv)
{
    struct exec_cmd* cmd = malloc(sizeof(*cmd));

    if (!cmd) {
        return NULL;
    }

    memset(cmd, 0, sizeof(*cmd));
    cmd->type = EXEC;
    cmd->path = path;
    cmd->argv = argv;
    return (struct cmd*) cmd;
}

// Führt die Shell endlos aus
void run_shell()
{
    static char buf[BUFFER_SIZE];
    
    // Liest und führt Eingabebefehle aus
    while (getcmd(buf, sizeof(buf)) >= 0) {
        // Überprüfen auf Shell-eigene Befehle wie 'cd'
        if (!strncmp(buf, "cd ", 3)) {
            // Chdir muss vom Elternprozess aufgerufen werden, nicht vom Kind
            buf[strlen(buf) - 1] = 0;  // '\n' abschneiden

            if (chdir(buf + 3) < 0) {
                fprintf(STD_ERR, "cannot cd %s\n", buf + 3);
            }
            continue;
        }

        if (fork1() == 0) {
            // Andernfalls erstellen wir ein Kind und führen den Befehl aus
            run_proxy(parsecmd(buf));
        }

        wait(0);
    }
}

// Gibt das Prompt aus und wartet auf Benutzereingabe
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

// Stellt sicher, dass drei Dateideskriptoren geöffnet sind
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

// Fork mit Panic im Fehlerfall
int fork1(void)
{
    int pid;

    pid = fork();

    if (pid == -1) {
        panic("fork");
    }
    return pid;
}

// Panic-Funktion für Fehlerausgaben
void panic(char* s)
{
    fprintf(STD_ERR, "%s\n", s);
    exit(EXIT_FAILURE);
}