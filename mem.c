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


#define N 1024//128M
#define S 128*1024*1024
int main(){
    int *a;
    int ret;
    //int *a=(int*)malloc(2)
    ret = posix_memalign(&a,4096,S);
    if(ret)
    {
        printf ("mem error\n");
        return -1;
    } 
    int ret2;
    int *b;
    ret2 = posix_memalign(&b,4096,S);
    if(ret2)
    {
        printf ("mem error\n");
        return -1;
    } 

    int i, size = sizeof(int);
    struct  timeval  start1;
	struct  timeval  end1;
    unsigned long timer1;
    
    memset(a,0,S);
    memset(b,0,S);
    int sum=0;
    for(int j=0;j<20;j++)
    {
        gettimeofday(&start1, NULL);
        for(int i=0;i<1000;i++)
        {
            memcpy(b+i*1024, a+i*1024, size * N);
        }
        gettimeofday(&end1, NULL);
        int speedtime = (end1.tv_sec * 1000000 + end1.tv_usec) - (start1.tv_sec * 1000000 + start1.tv_usec);
        printf("%dth exec time :%d us\n",j,speedtime);
        sum=sum+speedtime;
    }
    printf("avg: exec time :%d us\n",sum/20);
   //fsync(fd);
    //close(fd);
    return 0;
}