// sh_lib.h

#ifndef SH_LIB_H
#define SH_LIB_H

#include "kernel/types.h"

#define BUFFER_SIZE 100
#define EXEC 1

enum cmd_type { 
    CMD_EXEC 
};

// Basisstruktur für Befehle
struct cmd {
    enum cmd_type type;
};

// Struktur für EXEC-Befehl
struct exec_cmd {
    enum cmd_type type;
    char *path;       // Pfad zum ausführenden Programm
    char **argv;      // Argumente für das Programm
};

// Funktion zum Parsen von Befehlen (angenommen, du hast eine parsecmd-Funktion)
struct cmd* parsecmd(char *s);

// Funktion zum Erstellen eines EXEC-Befehls
struct cmd* exec_cmd_create(char *path, char **argv);

#endif // SH_LIB_H
