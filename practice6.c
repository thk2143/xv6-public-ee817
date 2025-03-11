#include "types.h"
#include "user.h"

int
main() {
    int fd = dup(1);
    write(1, "hello ", 6);
    write(fd, "world\n", 6);
    exit();
}