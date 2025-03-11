#include "types.h"
#include "user.h"

int
main(int argc, char *argv[]) {
    char buf[512];
    int n;

    for(;;){
        n = read(0,buf,sizeof buf);
        printf(1,"%d",n);
        if(n == 0) 
            exit();
        if(n < 0) {
            printf(2,"read error!\n");
            exit();
        }
        if(write(1,buf,n) != n) {
            printf(2,"write error!\n");
            exit();
        }
    }
    exit();
}