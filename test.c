#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h> 
#include <numa.h>
#include <sched.h>
#include <string.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    char path[50];
    for(int i=0;i<10;i++)
    {
        sprintf(path, "./tmp/%d",i+1);
        printf("%s\n", path);
    }
    return 0;
}
