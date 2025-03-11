#include "types.h"
#include "user.h"

int
main() {
    if(fork() == 0) {
        write(1, "hello", 6);
        exit();
    } else {
        wait();
        write(1, "world\n", 6);
    }
    exit();
}