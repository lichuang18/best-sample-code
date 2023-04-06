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


#define N 1024
#define S 256*1024*1024  //256M

#define NUM_FOR 4   //循环测试次数

#define THREADS  4
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
    int *a;//写数据的buf
    int size;//4B
    int icore;//绑核
    int offset;
};
static float sum_time[20];

void *func(void *para)
{
    struct  timeval  start;
    struct  timeval  end;
    struct mypara *ptr;
    ptr=(struct mypara *)para;
    cpu_set_t mask;
    CPU_ZERO(&mask);
    int cpu_num = sysconf(_SC_NPROCESSORS_CONF);
    if (cpu_num == 0) {
    	cpu_num = 1;
    }
    CPU_SET(ptr->icore, &mask);//设置要绑定的核
    sched_setaffinity(0, sizeof(cpu_set_t), &mask);//第一个参数0是指当前线程
    gettimeofday(&start, NULL);
    for(int i=0;i<300;i++)
    {
        int ret = pwrite(ptr->fd,ptr->a,N*ptr->size,ptr->offset+i*THREADS*4096);
        if(ret<0)
        {
            printf("pwrite failed!\n");
        }
        fsync(ptr->fd);
    }
    gettimeofday(&end, NULL);
    int speedt = (end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec);
    sum_time[ptr->icore] = (float)speedt;
    //write(ptr->fd, ptr->a, N*ptr->size);
    int y=sched_getcpu();
    //printf("current th run on cpu:%d\n",y);
}

int main(){
    cpu_set_t mask;
    CPU_ZERO(&mask);
    int cpu_num = sysconf(_SC_NPROCESSORS_CONF);
    if (cpu_num == 0) {
    	cpu_num = 1;
    }
    CPU_SET(0, &mask);//0号线程设置0core的亲和性
    sched_setaffinity(0, sizeof(cpu_set_t), &mask);
    int y=sched_getcpu();
    printf("main current th run on cpu:%d\n",y);

    if(numa_available() < 0) {
   	    printf("Your system does not support NUMA API\n");
    }
    int num=numa_max_node();//查看numa node数量
    printf("Your system NUMA nodes: %d\n",num);
    int numcpus=numa_num_task_cpus(); //查看可用CPU数量
    printf("Your numa_num_task_cpus: %d\n",numcpus);

    long num_size=numa_node_size(0,NULL);//返回对应numa节点上的空闲内存大小 以B为单位，x86工作站1个numa node 工126GB内存空闲
    printf("Your node-size: %ld\n",num_size);

    int *c = numa_alloc_onnode(S,0);//在numa node0上分配256M空间
    int *d = numa_alloc_onnode(S,0);//在numa node0上分配256M空间

    memset(c,0,S);
    memset(d,0,S);
    int fd;
    //if( (fd=open("/dev/sdf", O_RDWR|O_DIRECT|O_SYNC))< 0){  //BLOCK测试
    //if( (fd=open("/dev/sdf", O_RDWR|O_DIRECT))< 0){
    //if( (fd=open("/dev/sdf", O_RDWR))< 0){ 
    //if( (fd=open("./mnt/1", O_RDWR|O_DIRECT|O_SYNC))< 0){  //带FS的测试
    if( (fd=open("./mnt/1", O_RDWR))< 0){ 
        exit(0);
    }
    int i, size = sizeof(int);
    struct  timeval  start1,start2;
	struct  timeval  end1,end2;
    unsigned long timer1;
    float sum=0;
    float sum_total=0;
    pthread_t th[THREADS];
    for(int j=0;j<NUM_FOR;j++)
    {
        gettimeofday(&start2, NULL);
        gettimeofday(&start1, NULL);
        for(int i=0;i<300;i++)
        {
            int ret = pwrite(fd,c,N*size,0+i*THREADS*4096);
            if(ret<0) 
            {
                printf("pwrite faild\n");
            }
            //write(fd, c, N*size);
            fsync(fd);
        }
        gettimeofday(&end1, NULL);
        int speedtime = (end1.tv_sec * 1000000 + end1.tv_usec) - (start1.tv_sec * 1000000 + start1.tv_usec);
        //printf("exec time :%f us\n",(float)speedtime);//输出0线程的单次4K write时间开销
        sum_time[0] = (float)speedtime;
        sum = (float)speedtime/300+sum;//统计0线程总时间开销
        
        struct mypara ptr[THREADS];
        for (int i=0 ;i<THREADS-1;i++)
        {
            ptr[i].fd=fd;
            ptr[i].a=&d;
            ptr[i].size=size;
            ptr[i].icore=i+1;
            ptr[i].offset=4096*(i+1);
            pthread_create(&th[i],NULL,func,&(ptr[i]));    
        }
    }
    printf("avg time :%f us\n",sum/NUM_FOR);
    for (int i=0 ;i<THREADS-1;i++)
    {
        pthread_join(th[i],NULL);
    } 
    float he;
    for(int i=0 ;i<THREADS;i++)
    {
        he=he+sum_time[i];
    }
    printf("total time :%f us\n",he);
    close(fd);
    return 0;
}
