// cp.c

#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include <string.h>

int main(int argc, char *argv[]) {
    // Überprüfen der Argumentanzahl
    if (argc != 3) {
        fprintf(2, "Usage: cp <source> <destination>\n");
        exit(1); // EXIT_FAILURE durch 1 ersetzt
    }

    char *source = argv[1];
    char *destination = argv[2];

    // Öffnen der Quelldatei
    int src_fd = open(source, O_RDONLY);
    if (src_fd < 0) {
        fprintf(2, "Error: Cannot open source file %s\n", source);
        exit(1); // EXIT_FAILURE durch 1 ersetzt
    }

    // Erstellen/Öffnen der Zieldatei
    int dest_fd = open(destination, O_CREATE | O_WRONLY);
    if (dest_fd < 0) {
        fprintf(2, "Error: Cannot open destination file %s\n", destination);
        close(src_fd);
        exit(1); // EXIT_FAILURE durch 1 ersetzt
    }

    // Kopieren der Daten
    char buffer[512];
    int n;

    while ((n = read(src_fd, buffer, sizeof(buffer))) > 0) {
        if (write(dest_fd, buffer, n) != n) {
            fprintf(2, "Error: Write to destination file failed\n");
            close(src_fd);
            close(dest_fd);
            exit(1); // EXIT_FAILURE durch 1 ersetzt
        }
    }

    if (n < 0) {
        fprintf(2, "Error: Read from source file failed\n");
        close(src_fd);
        close(dest_fd);
        exit(1); // EXIT_FAILURE durch 1 ersetzt
    }

    // Schließen der Dateien und Beenden
    close(src_fd);
    close(dest_fd);
    exit(0); // EXIT_SUCCESS durch 0 ersetzt
}