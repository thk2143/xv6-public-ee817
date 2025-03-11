#include "types.h"
#include "user.h"

int 
main() {
    char *argv[3];
    argv[0] = "echo";
    argv[1] = "hello";
    argv[2] = 0; 

    exec("./echo",argv);
    printf(1,"exec error\n");
    exit();
}