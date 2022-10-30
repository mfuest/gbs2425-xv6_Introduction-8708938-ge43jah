#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

char buf[512];

int readUntil(const char delim) {
  int pos = 0;
  do {
    if (read(0, buf+pos, 1) == 1) {
      if (buf[pos] == delim) {
        buf[pos] = '\0'; // remove delim and terminate string
        return 0;
      }
      pos++;
    } else {
      return -1;
    }
  } while(1);
}

int
main(int argc, char *argv[])
{
    printf("ok\n");

    do {
      // read command
      if (read(0, buf, 2) != 2) {
        printf("error could not read command\n");
        exit(1);
      }
      buf[1] = '\0'; // replace ' ' with terminating '\0'

      if (strcmp(buf, "m") == 0) { // malloc
        // read size
        if (readUntil(' ') == -1) {
          printf("error could not read size\n");
          exit(1);
        }
        int size = atoi(buf);

        void *addr = malloc(size);
        if (addr == 0) {
          printf("error could not malloc\n");
          exit(1);
        }

        // read char to set
        if (readUntil('\n') == -1) {
          printf("error could not read char\n");
          exit(1);
        }
        int c = atoi(buf);
        memset(addr, c, size);
        printf("ok %l\n", addr);

      } else if (strcmp(buf, "f") == 0) { // free
        // read address
        if (readUntil('\n') == -1) {
          printf("error could not read address\n");
          exit(1);
        }
        void* addr = (void*)((long)atoi(buf));
        free(addr);
        printf("ok\n");

      } else if (strcmp(buf, "q") == 0) {
        // quit
        exit(0);

      } else {
        printf("error unknown command\n");
        exit(1);
      }

    } while(1);

    exit(0);
}
