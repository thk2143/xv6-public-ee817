#include "types.h"
#include "fcntl.h"
#include "user.h"

int 
main (int argc, char *argv[]) {
    mkdir("./dir");
    int fd = open("./dir/file", O_CREATE|O_WRONLY);
    close(fd);

    mknod("/console",1,1);

    exit();
}