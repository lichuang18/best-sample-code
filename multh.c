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
#define S 1024*1024*1024  //1G
#define NUM_FOR 4   //循环测试次数
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
    for(int i=0;i<3000;i++)
    {
        int ret = pwrite(ptr->fd,ptr->a,4096,ptr->offset+i*4096*ptr->th);//ptr->offset+i*4096
        if(ret<0)
        {
            printf("pthread pwrite failed!\n");
        }
        fsync(ptr->fd);
    }
    //gettimeofday(&end, NULL);
    //int speedt = (end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec);
    //sum_time[ptr->icore] = (float)speedt;
    //printf("current th run on cpu:%d,avg time :%f us,avg time :%f us \n",y,sum_time[ptr->icore],sum_time[ptr->icore]/3000);
    //write(ptr->fd, ptr->a, N*ptr->size);
}

int main(int argc , char **argv){
    
    int threads =  atoi(argv[1]);
    printf("argc = %d\n",argc);

    printf("argv[1] = %s\n",argv[1]);

    printf("target thread :%d\n",threads);
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
    if( (f[0]=open("./mnt/2", O_RDWR))< 0){ 
        exit(0);
    }
    if( (f[1]=open("./mnt/3", O_RDWR))< 0){ 
        exit(0);
    }
    if( (f[2]=open("./mnt/4", O_RDWR))< 0){ 
        exit(0);
    }
    if( (f[3]=open("./mnt/5", O_RDWR))< 0){ 
        exit(0);
    }
    if( (f[4]=open("./mnt/6", O_RDWR))< 0){ 
        exit(0);
    }
    if( (f[5]=open("./mnt/7", O_RDWR))< 0){ 
        exit(0);
    }
    if( (f[6]=open("./mnt/8", O_RDWR))< 0){ 
        exit(0);
    }
    if( (f[7]=open("./mnt/9", O_RDWR))< 0){ 
        exit(0);
    }
    if( (f[8]=open("./mnt/10", O_RDWR))< 0){ 
        exit(0);
    }
    if( (f[9]=open("./mnt/11", O_RDWR))< 0){ 
        exit(0);
    }
    if( (f[10]=open("./mnt/12", O_RDWR))< 0){ 
        exit(0);
    }
    if( (f[11]=open("./mnt/13", O_RDWR))< 0){ 
        exit(0);
    }
    if( (f[12]=open("./mnt/14", O_RDWR))< 0){ 
        exit(0);
    }
    if( (f[13]=open("./mnt/15", O_RDWR))< 0){ 
        exit(0);
    }
    if( (f[14]=open("./mnt/16", O_RDWR))< 0){ 
        exit(0);
    }
    if( (f[15]=open("./mnt/17", O_RDWR))< 0){ 
        exit(0);
    }


    int i, size = sizeof(int);
    struct  timeval  start1,start2;
	struct  timeval  end1,end2;
    unsigned long timer1;
    float sum=0;
    float sum_total=0;
    pthread_t th[threads];
    gettimeofday(&start1, NULL);
    for(int i=0;i<3000;i++)
    {
        int ret = pwrite(fd,c,N*size,0+i*4096);
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
    sum = (float)speedtime/3000+sum;//统计0线程总时间开销
    printf("avg time :%f us\n",sum);
    struct mypara ptr[threads];

    struct  timeval  start;
    struct  timeval  end;
    gettimeofday(&start, NULL);
    for (int i=0 ;i<threads;i++)
    {
        ptr[i].fd=f[i];
        ptr[i].a=&d;
        ptr[i].size=size;
        ptr[i].icore=i+1;
        ptr[i].offset=4096*(i+1);
        ptr[i].th=threads;
        pthread_create(&th[i],NULL,func,&(ptr[i]));    
    }
    for (int i=0 ;i<threads;i++)
    {
        pthread_join(th[i],NULL);
    } 
    gettimeofday(&end, NULL);
    int spt = (end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec);
    float total_time=(float)spt;
    /*
    float total_time;
    for(int i=1 ;i<=threads;i++)
    {
        total_time=total_time+sum_time[i];
    }
    */
    printf("pthread total time :%f us\n",total_time);

    float IOPS=3000*threads/(total_time/1000000);
    printf("IOPS : %f\n",IOPS);
    close(fd);
    return 0;
}
