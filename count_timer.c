//原文链接：https://blog.csdn.net/lucky_zbaby/article/details/110945309
 
 
//在Linux 环境下，计算一个循环或者处理事件类事情花费多长时间计算C程序运行的时间，可以通过以下三个函数来实现：clock(),time(),gettimeofday()

long i = 100000000L;
	/* clock函数持续的时间*/
	clock_t start, finish;
	double duration;	
	start = clock();
	cout << i << "loops is";
	while(i--);
	finish = clock();
	duration = ((double)(finish - start) / CLOCKS_PER_SEC);
	    
	cout << "Clock()  " << duration << "seconds"<<endl;

	/* gettimeofday函数持续的时间*/
	 i = 100000000L;
	struct  timeval  start1;
	struct  timeval  end1;
	unsigned long timer1;	
	gettimeofday(&start1, NULL);
	while (i--);
	gettimeofday(&end1, NULL);
	int speedtime = (end1.tv_sec * 1000000 + end1.tv_usec) - (start1.tv_sec * 1000000 + start1.tv_usec);	
	cout << "gettimeofday = "<< speedtime << endl;
		 
	/* time函数持续的时间*/
	 i = 100000000L;
	 time_t t1, t2;
	 time(&t1);
	 while (i--);
	 time(&t2);
	 cout << "time()  " << t2-t1 << endl;