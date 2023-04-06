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
#define SINGLE 0

#define MEMBUF_SIZE 1024*1024*1024  //分配的内存大小
#define NUM_FOR 1000   //循环测试次数
#define MAX_THREADS 128
static int SYNC_FLAG=0;//多线程初始完毕标志
static int INIT_C[MAX_THREADS]={0};//多线程初始化完成标志
static float sum_ths_time[MAX_THREADS];//多线程pwrite时间统计数组

struct mypara
{
    int icore;//绑核
    int offset;
    int io_size;
};


int init_sum(int ths)
{
    int sum=0;
    for(int i=0; i<=ths; i++)
    {
        sum=sum+INIT_C[i];
    }
    return sum;
}

void *new_func(void *para)
{
    //拿到传进来的结构体参数
    struct mypara *ptr;
    ptr=(struct mypara *)para;

    //初始化操作

    //初始化之 设置亲和性
    cpu_set_t mask;
    CPU_ZERO(&mask);
    int cpu_num = sysconf(_SC_NPROCESSORS_CONF);
    if (cpu_num == 0) {
        cpu_num = 1;
    }
    CPU_SET(ptr->icore, &mask);//设置icore的亲和性
    sched_setaffinity(0, sizeof(cpu_set_t), &mask);//参数0表示当前进程
    int y=sched_getcpu();
    if(DEBUG)//打印设置的CPU亲和性信息
    {
        printf("current th run on cpu:%d\n",y);
    }

    //初始化之 numa
    if(numa_available() < 0) {
        printf("Your system does not support NUMA API\n");
    }
    if(DEBUG)//打印NUMA信息
    {
        int num=numa_max_node();//查看numa node数量
        int numcpus=numa_num_task_cpus(); //查看可用CPU数量
        long num_size=numa_node_size(0,NULL);//返回对应numa节点上的空闲内存大小 以B为单位，x86工作站1个numa node 工126GB内存空闲
        printf("Your system NUMA nodes: %d\n",num);
        printf("Your numa_num_task_cpus: %d\n",numcpus);
        printf("Your node-size: %ld\n",num_size);
    }
    long long *thbuf = numa_alloc_onnode(MEMBUF_SIZE,0);//在numa node0上分配空间
    memset(thbuf,0,MEMBUF_SIZE);//初始化，避免缺页中断影响
    int th_fd;
    char th_path[50];
    sprintf(th_path, "./mnt/%d",ptr->icore+1);
    if( (th_fd = open(th_path, O_RDWR|O_DIRECT))< 0){ 
        printf("th_open %dth failed \n",ptr->icore+1);
        exit(0);
    }
    if(DEBUG) printf("%s\n", th_path);
    
    //线程初始化完成，INTT_C[ptr->icore]赋值为1
    INIT_C[ptr->icore]=1;


    int size = sizeof(int);
    struct timeval start,end;
    float th_sum=0;
    
    do{
        continue;
    }while(SYNC_FLAG != 1);

    if(DEBUG) printf("多线程开始执行write\n");
    gettimeofday(&start, NULL);
    for(int i=0;i<NUM_FOR;i++)
    {
        int ret = pread(th_fd,thbuf,ptr->io_size,0+i*ptr->io_size);//写到pagecache，用户态请求到pagecache
        if(ret<0) 
        {
            printf("th_pwrite faild\n");
        }
        //fsync(th_fd);//刷回到盘，pagecache到盘
    }
    gettimeofday(&end, NULL);
    int th_speedtime = (end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec);
    sum_ths_time[ptr->icore] = (float)th_speedtime/NUM_FOR;//统计线程ptr->icore单次时间开销
    if(DEBUG) printf("thread[%d](%d core) threads-iosize:%dK,thread-avg-time:%f\n", ptr->icore+1, ptr->icore, ptr->io_size/1024, sum_ths_time[ptr->icore]);
}

void Single_test(int threads, int io_size)
{
    //设置亲和性
    cpu_set_t mask;
    CPU_ZERO(&mask);
    int cpu_num = sysconf(_SC_NPROCESSORS_CONF);
    if (cpu_num == 0) {
        cpu_num = 1;
    }
    CPU_SET(0, &mask);//0号线程设置0core的亲和性
    sched_setaffinity(0, sizeof(cpu_set_t), &mask);
    int y = sched_getcpu();
    if(DEBUG)//打印main函数的CPU亲和性信息，以及一些调试信息
    {
        printf("main current th run on cpu:%d\n",y);
    }
    if(numa_available() < 0) {
        printf("Your system does not support NUMA API\n");
    }

    if(DEBUG)//打印NUMA信息
    {
        int num = numa_max_node();//查看numa node数量
        int numcpus = numa_num_task_cpus(); //查看可用CPU数量
        long num_size = numa_node_size(0, NULL);//返回对应numa节点上的空闲内存大小 以B为单位，x86工作站1个numa node 工126GB内存空闲
        printf("Your system NUMA nodes: %d\n",num);
        printf("Your numa_num_task_cpus: %d\n",numcpus);
        printf("Your node-size: %ld\n",num_size);
    }
    //numa分配内存
    long long *numa_membuf = numa_alloc_onnode(MEMBUF_SIZE, 0);//在numa node0上分配空间
    memset(numa_membuf, 0, MEMBUF_SIZE);//初始化，避免缺页中断影响
    int fd;
    //if( (fd=open("/dev/sdf", O_RDWR|O_DIRECT|O_SYNC))< 0){  //BLOCK测试
    if( (fd=open("./mnt/0", O_RDWR|O_DIRECT))< 0){ 
        exit(0);
    }
    struct timeval start1,end1;
    //unsigned long timer1;
    float avg_time=0;
    gettimeofday(&start1, NULL);
    for(int i=0;i<NUM_FOR;i++)
    {
        int ret = pread(fd,numa_membuf,io_size,0+i*io_size);//写到pagecache，用户态请求到pagecache
        if(ret<0) 
        {
            printf("single pwrite faild\n");
        }
        //fsync(fd);//刷回到盘，pagecache到盘
    }
    gettimeofday(&end1, NULL);
    int speedtime = (end1.tv_sec * 1000000 + end1.tv_usec) - (start1.tv_sec * 1000000 + start1.tv_usec);
    avg_time = (float)speedtime/NUM_FOR;//统计单线程，单次pwrite+fsync时间开销
    printf("Single threads-iosize:%dK,Single-avg-time:%f\n",io_size/1024,avg_time);
    close(fd);
}

int main(int argc , char **argv){
    int threads =  atoi(argv[1]);//终端输入第一个参数 设置线程数
    int io_size = 1024*atoi(argv[2]);//终端输入第二个参数 设置io-size以K为单位
    //测试单线程   对比fio，注意fio设置了qdepth的性能
    if(SINGLE){
        Single_test(threads, io_size);
        return 0;
    }
    
    //测试多线程
    pthread_t th[threads];
    struct mypara ptr[threads];
    for (int i=0 ;i<threads;i++)//创建多线程
    {
        ptr[i].icore=i;
        ptr[i].io_size=io_size;
        pthread_create(&th[i],NULL,new_func,&(ptr[i]));    
    }

    //轮询多线程是否已全部完成初始化工作
    int init_count;
    do{
        init_count = init_sum(threads);
        //printf("已完成初始化的线程数量是：%d\n",init_count);
    }while(init_count != threads);
    if(DEBUG) printf("OK now已完成初始化的线程数量是：%d, and threads is: %d\n",init_count, threads);
    SYNC_FLAG=1;//多线程全部初始化完毕，才开始执行写操作
    for (int i=0 ;i<threads;i++)
    {
        pthread_join(th[i],NULL);
    } 
    //多线程全部执行完毕，统计时间
    float total_time = 0;
    for(int i = 0; i<threads; i++)
    {
        total_time = total_time + sum_ths_time[i];
    }
    float pth_avg_time = total_time/threads;
    float IOPS=1000000/pth_avg_time;

    printf("0th 线程的延迟为:%fus,threads:%d,iosize:%d K,IOPS:%f,performance:%f MB/s,pth-avg-time:%f\n",sum_ths_time[0],threads,io_size/1024,IOPS,IOPS*io_size/1024/1024,pth_avg_time);
    //输出0线程的延迟
    return 0;
}
