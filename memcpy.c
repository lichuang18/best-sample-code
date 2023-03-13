#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#define N 1024
int main(){
    int a[N];
    int b[N];
    int i, size = sizeof(int);
    //printf("size of int :%ld, total size : %d\n",sizeof(int),N*size);
    /*
    int fd;
    if( (fd=open("/dev/sdf", O_RDWR)) < 0 ){  //以二进制方式打开 |O_DIRECT|O_SYNC
        exit(0);
    }
*/
    struct  timeval  start1;
	struct  timeval  end1;
    unsigned long timer1;

    gettimeofday(&start1, NULL);
    for(int i=0;i<1000;i++)
    {
        memcpy(b, a, size * N);
    }
    gettimeofday(&end1, NULL);
    int speedtime = (end1.tv_sec * 1000000 + end1.tv_usec) - (start1.tv_sec * 1000000 + start1.tv_usec);
    printf("exec time :%d us\n",speedtime);
   //fsync(fd);
    //close(fd);
    return 0;
}