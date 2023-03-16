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
//#include <numa.h>
#include "/usr/include/numa.h"

#define N 1024//128M
#define S 128*1024*1024  //128M
int main(){
    if(numa_available() < 0) {
   	    printf("Your system does not support NUMA API\n");
    }
    int num=numa_max_node();//查看numa node数量
    printf("Your system NUMA nodes: %d\n",num);
    int numcpus=numa_num_task_cpus(); //查看可用CPU数量
    printf("Your numa_num_task_cpus: %d\n",numcpus);

    long num_size=numa_node_size(0,NULL);//返回对应numa节点上的空闲内存大小 以B为单位，x86工作站1个numa node 工126GB内存空闲
    printf("Your node-size: %ld\n",num_size);

    int *c = numa_alloc_onnode(S,0);//在numa node0上分配128M空间
    int *d = numa_alloc_onnode(S,0);//在numa node0上分配128M空间

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

    memset(a,0,S);
    memset(b,0,S);
    memset(c,0,S);
    memset(d,0,S);
    int i, size = sizeof(int);
    struct  timeval  start1;
	struct  timeval  end1;
    unsigned long timer1;
    int sum=0;
    for(int j=0;j<20;j++)//每次4M，下一次从下一个4M开始  共80M
    {
        gettimeofday(&start1, NULL);
        for(int i=0;i<1000;i++)//每次4K，下次从下一个4k开始访问  共4M
        {
            memcpy(d+j*1024*1024+i*1024, c+j*1024*1024+i*1024, size * N); //4K粒度cpy  
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