#include "types.h"
#include "user.h"

int
main() {
    char *argv[2];
    argv[0] = "cat";
    argv[1] = 0;
    if(fork() == 0) {
        close(0);
        open("input.txt", 0); // 0 = O_RDONLY
        exec("cat", argv);
    } else {
        wait();
    }
    exit();
}