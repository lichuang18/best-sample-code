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

#define DEBUG 0
#define N 1024
#define S 1024*1024*1024  //1G
#define NUM_FOR 300   //循环测试次数
#//define rand_seq 4*1024//1024*1024//4096   
/*
#define DEBUG4  1 
#define DEBUG6  1 
#define DEBUG8  1 
#define DEBUG10  1 
#define DEBUG12  1 
#define DEBUG16  1 
*/
struct mypara
{
    int fd;//文件描述符
    long long *a;//写数据的buf
    int size;//4B
    int icore;//绑核
    int offset;
    int th;//多线程数量
    int rand_seq;
};
static float sum_time[20];

void *func(void *para)
{
    struct mypara *ptr;
    ptr=(struct mypara *)para;
    //设置CPU亲和性
    cpu_set_t mask;
    CPU_ZERO(&mask);
    int cpu_num = sysconf(_SC_NPROCESSORS_CONF);
    if (cpu_num == 0) {
    	cpu_num = 1;
    }
    CPU_SET(ptr->icore, &mask);//设置要绑定的核
    sched_setaffinity(0, sizeof(cpu_set_t), &mask);//第一个参数0是指当前线程
    int y=sched_getcpu();
    //printf("current th run on cpu:%d\n",y);

    //struct  timeval  start;
    //struct  timeval  end;
    //gettimeofday(&start, NULL);
    //printf("fd = %d: a = %p:\n ",ptr->fd,ptr->a);
    for(int i=0;i<NUM_FOR;i++)
    {
        int ret = pwrite(ptr->fd,ptr->a,ptr->rand_seq,0+i*ptr->rand_seq);//*ptr->th   ptr->offset
        
        if(ret<0)
        {
            printf("pthread pwrite failed!\n");
        }
        //fsync(ptr->fd);
    }
    //gettimeofday(&end, NULL);
    //int spt = (end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec);
    //float total_time=(float)spt/NUM_FOR;
    /*
    float total_time;
    for(int i=1 ;i<=threads;i++)
    {
        total_time=total_time+sum_time[i];
    }
    */
    //printf("pthread avg time :%f us\n",total_time);
    //printf("current th run on cpu:%d,avg time :%f us,avg time :%f us \n",y,sum_time[ptr->icore],sum_time[ptr->icore]/3000);
}

int main(int argc , char **argv){
    
    int threads =  atoi(argv[1]);
    int rand_seq = 1024*atoi(argv[2]);
    cpu_set_t mask;
    CPU_ZERO(&mask);
    int cpu_num = sysconf(_SC_NPROCESSORS_CONF);
    if (cpu_num == 0) {
    	cpu_num = 1;
    }
    CPU_SET(0, &mask);//0号线程设置0core的亲和性
    sched_setaffinity(0, sizeof(cpu_set_t), &mask);
    int y=sched_getcpu();
    if(DEBUG)
    {
        printf("main current th run on cpu:%d\n",y);
        printf("argc = %d\n",argc);
        printf("argv[1] = %s\n",argv[1]);
        printf("target thread :%d\n",threads);
    }
    if(numa_available() < 0) {
   	    printf("Your system does not support NUMA API\n");
    }
    int num=numa_max_node();//查看numa node数量
    //printf("Your system NUMA nodes: %d\n",num);
    int numcpus=numa_num_task_cpus(); //查看可用CPU数量
    //printf("Your numa_num_task_cpus: %d\n",numcpus);
    long num_size=numa_node_size(0,NULL);//返回对应numa节点上的空闲内存大小 以B为单位，x86工作站1个numa node 工126GB内存空闲
   // printf("Your node-size: %ld\n",num_size);
    long long *c = numa_alloc_onnode(S,0);//在numa node0上分配空间
    long long *d = numa_alloc_onnode(S,0);//在numa node0上分配空间

    memset(c,0,S);
    memset(d,0,S);
    int fd;
    int f[16];
    //if( (fd=open("/dev/sdf", O_RDWR|O_DIRECT|O_SYNC))< 0){  //BLOCK测试
    //if( (fd=open("/dev/sdf", O_RDWR|O_DIRECT))< 0){
    //if( (fd=open("/dev/sdf", O_RDWR))< 0){ 
    //if( (fd=open("./mnt/1", O_RDWR|O_DIRECT|O_SYNC))< 0){  //带FS的测试
    if( (fd=open("./mnt/1", O_RDWR))< 0){ 
        exit(0);
    }

    char path[50];
    for(int i=2;i<35;i++)
    {
        sprintf(path, "./mnt/%d",i+1);
        if( (f[i-2]=open(path, O_RDWR))< 0){ 
            printf("failed %d\n",i);
            exit(0);
        }
        //printf("%s\n", path);
    }
    int i, size = sizeof(int);
    struct  timeval  start1,start2;
	struct  timeval  end1,end2;
    unsigned long timer1;
    float sum=0;
    float sum_total=0;
    pthread_t th[threads];

    gettimeofday(&start1, NULL);
    //printf("fd = %d: buf = %p:\n ",fd, d);
    for(int i=0;i<NUM_FOR;i++)
    {
        int ret = pwrite(fd,d,rand_seq,0+i*rand_seq);
        if(ret<0) 
        {
            printf("pwrite faild\n");
        }
        //write(fd, c, N*size);
        //fsync(fd);
    }
    gettimeofday(&end1, NULL);
    int speedtime = (end1.tv_sec * 1000000 + end1.tv_usec) - (start1.tv_sec * 1000000 + start1.tv_usec);
    //printf("exec time :%f us\n",(float)speedtime);//输出0线程的单次4K write时间开销
    sum_time[0] = (float)speedtime;
    sum = (float)speedtime/NUM_FOR+sum;//统计0线程总时间开销
    
    struct mypara ptr[threads];
    struct  timeval  start;
    struct  timeval  end;
    gettimeofday(&start, NULL);
    for (int i=0 ;i<threads;i++)
    {
        ptr[i].fd=f[i];//f[i];
        ptr[i].a=d;
        ptr[i].size=size;
        ptr[i].icore=i+1;
        ptr[i].offset=rand_seq*(i+1);
        ptr[i].th=threads;
        ptr[i].rand_seq=rand_seq;
        pthread_create(&th[i],NULL,func,&(ptr[i]));    
    }
    for (int i=0 ;i<threads;i++)
    {
        pthread_join(th[i],NULL);
    } 
    gettimeofday(&end, NULL);
    int spt = (end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec);
    float total_time=(float)spt/NUM_FOR;
    float IOPS=threads/(total_time/1000000);
    printf("threads:%d,iosize:%d K,IOPS:%f,performance:%f MB/s,pth-avg-time:%f,main-abg-time:%f\n",threads,rand_seq/1024,IOPS,IOPS*rand_seq/1024/1024,total_time,sum);
    close(fd);
    return 0;
}
