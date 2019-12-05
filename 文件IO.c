一、Linux中文件及文件描述符概述
{//Linux中文件及文件描述符概述
在Linux中对目录和设备的操作都等同于文件的操作，Linux中文件主要分为4种：普通文件、目录文件、链接文件和设备文件。
内核通过文件描述符来区分和引用特定的文件。对Linux而言，所有设备和文件的操作都是使用文件描述符来进行的。
文件描述符是一个非负的整数，它是一个索引值，并指向在内核中每个进程打开文件的记录表。
当打开一个现存文件或创建一个新文件时，内核就向进程返回一个文件描述符；当需要读写文件时，也需要把文件描述符作为参数传递给相应的函数。
通常，一个进程启动时都会打开三个文件：标准输入、标准输出和出错处理。
这三个文件分别对应文件描述法为0、1和2（也就是替换宏STDIN_FILENO、STDOUT_FILEON和STDERR_FILENO）。

}//Linux中文件及文件描述符概述

二、底层文件I/O操作
{//底层文件I/O操作
理论
{//理论
		1.基本文件操作
(1)open()函数用于打开或创建文件，在打开或创建文件时可以指定用户的属性及用户的权限等各种参数。
open()函数语法要点
#include<sys/types.h>  /*  提供类型pid_t的定义  */
#include<sys/stat.h>
#include<fcntl.h>
int open(const char *pathname,int flags,int perms)//打开谁，怎么打开，文件权限
函数传入值	
pathname:被打开的文件名（包括路径名）
flag:文件打开的方式	
		O_RDONLY：以只读方式打开文件
		O_WRONLY：以只写方式打开文件
		O_RDWR：以读写方式打开文件
		O_CREAT：如果该文件不存在，就创建一个新的文件，并用第三个参数为其设置权限
		O_EXCL：如果使用O_CREAT时文件存在，则可返回错误消息。这一参数可测试文件是否存在。此时open是原子操作，防止多个进程同时创建同一个文件
		O_NOCTTY：使用本参数时，若文件为终端，那么该终端不会成为调用open()的那个进程的控制终端
		O_TRUNC：若文件已经存在，那么会删除文件中的所有数据，并设置文件大小为0
		O_APPEND：以添加方式打开文件，在打开文件的同时，文件指针指向文件的末尾，即将写入的数据添加到文件的末尾
perms:被打开文件的存取权限
		可以用一组宏定义：S_I(R/W/X)(USR/GRP/OTH)
		其中R/W/X分别表示读/写/执行权限
		USR/GRP/OTH分别表示文件所有者/文件所属组/其他用户
		例如，S_IRUSR|S_IWUSR表示设置文件所有者的可读可写属性。八进制表示法中600也表示同样的权限
函数返回值	
成功：返回文件描述符;失败：-1
PS：在open()函数中，flag参数可通过“|”组合构成，但前3个标志常量（O_RDONLY、O_WRONLY以及O_RDWR）不能相互组合。
	perms是文件的存取权限，既可以用宏定义表示法，也可以用八进制表示法。

(2)close()函数用于关闭一个被打开的文件。当一个进程终止时，所有被它打开的文件都由内核自动关闭，很多程序都使用这一功能而不显示地关闭一个文件。
#include<unistd.h>
int close(int fd)//直接关闭指定文件
函数输入:fd：文件描述符
函数返回值:0：成功
		  -1：出错

(3)read()函数用于将从指定的文件描述符中读出数据放到缓存区中，并返回实际读入的字节数。若返回0，则表示没有数据可读，即已达到文件尾。
		读操作从文件的当前制作位置开始。当从终端设备文件中读出数据时，通常最多一次读一行。
#include<unistd.h>
ssize_t read(int fd,void *buf,size_t count)//读谁，读出来的存在哪，读多少
函数传入值	
fd：文件描述符
buf：指定存储器读出数据的缓冲区
count：指定读出的字节数
函数返回值:成功：读到的字节数
		   0：已到达文件尾
		   -1：出错

(4)write()函数用于向打开的文件写数据，写操作从文件的当前指针位置开始。对磁盘文件进行写操作，若磁盘已满或超出该文件的长度，则write()函数返回失败。
#include<unistd.h>
ssize_t write(int fd,void *buf,size_t count)//往谁写，写什么，写多少
函数传入值	
fd：文件描述符
buf：指定存储器写入数据的缓冲区
count：指定写入的字节数
函数返回值:成功：已写入的字节数
	       -1：出错

(5)lseek()函数用于在指定的文件描述符中将文件指针指定到相应的位置。它只能用在可定位（可随机访问）文件操作中。管道、套接字和大部分字符设备室不可定位的，所以在这些文件的操作中无法使用lseek()调用。
#include<unistd.h>
#include <sys/types.h>
off_t lseek(int fd,off_t offset,int whence)//在哪个文件定位，从哪开始，偏移多少
函数传入值	
fd：文件描述符
offset：偏移量，每一读写操作所需要移动的距离，单位是字节，可正可负（向前移，向后移）
whence：当前位置的基点	SEEK_SET：当前位置为头文件的开头，新位置为偏移量的大小
SEEK_CUR：当前位置为文件指针的位置，新位置为当前位置加上偏移量
SEEK_END：当前位置为文件的结尾，新位置为文件的大小加上偏移量的大小
函数返回值:成功：文件的当前位移
           -1：出错

2.文件锁
  当文件可以同时被多个进程使用时，为了避免共享资源产生竞态*****
  Linux通常采用的方法是给文件上锁——文件锁有两种：建议性锁和强制性锁。
  建议性锁要求每个上锁文件的进程都要检查是否有锁存在，并且尊重已有的锁
  强制锁是由内核执行的锁，当一个文件被上锁进行写入操作的时候，内核将阻止任何其他文件对其进行读写操作
  在Linux中，实现上锁的函数有lockf()和fcntl()，lockf()用于对文件施加建议性锁
  fcntl()可以施加建议性锁和强制锁，同时fcntl()还能对文件的某一记录上锁——记录锁
  记录锁又分为读取锁（共享锁）和写入锁（排斥锁）
  (1)读取锁可以使多个进程都在文件的统一部分建立读取锁
  (2)写入锁使在任何时刻只能有一个进程在文件的某一部分建立写入锁。
  PS：在文件的同一部分不能同时建立读取锁和写入锁

*******************fcntl()函数******************	
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
int fcntl(int fd,int cmd,struct flock *lock)
函数传入值	
fd：文件描述符
cmd:
	F_DUPFD：复制文件描述符
	F_GETFD：获得fd的close-on-exec标志，若标志未设置，则文件经过exec()函数之后仍保持打开状态
	F_SETFD：设置close-on-exec标志，该标志由参数arg的FD_CLOEXEC位决定
	F_GETFL：得到open设置的标志
	F_SETFL：改变open设置的标志
	F_GETLK：根据lock参数值，决定是否上文件锁
	F_SETLK：设置lock参数值的文件锁
	F_SETLKW：这是F_SETLK的阻塞版本（命令名中的W表示等待（wait））。在无法获取锁时，会进入睡眠状态；如果可以获取锁或者捕捉到信号则会返回
lock：结构为flock，设置记录锁的具体状态
函数返回值	0：成功
           -1：出错

struct flock
{
    short l_type;
    //F_RDLCK:读取锁（共享锁）
    //F_WRLCK：写入锁（排斥锁）
    //F_UNLCK:解锁
    off_t l_start;            //相对位移量（字节）
    short l_whence;           //相对位移量的起点
    //SEEK_SET:当前位置为文件的开头，新位置为偏移量的大小
    //SEEK_CUR:当前位置为文件指针的位置，新位置为当前位置加上偏移量
    //SEEK_END:当前位置为文件的结尾，新位置为文件的大小加上偏移量的大小
    off_t l_len;              //加锁区域的长度
    pid_t l_pid；
};

}//理论
	
例程
{//例程
	e.g设置锁函数，并测试
/*lock_set.c 判断文件是否可以上锁，根据参数上锁，输出上锁类型*/
int lock_set(int fd ,int type)
{
	struct flock lock;
	lock.l_type=type;//加锁类型
	lock.l_start=0;
	lock.l_whence=SEEK_SET;
	lock.l_len=0;//整个文件加锁
	lock.l_pid=-1;//??????
	/*判断文件是否可以上锁*/
	fcntl(fd,F_GETFL,&lock);
	/*判断不能上锁原因*/
	if(lock.l_type!=F_UNLCK)
	{
		if(lock.l_type==F_RDLCK)
		{
			printf("read lock already set by  %d\n",getpid());
		}
		else if(lock.l_type==F_WRLCK)
		{
			printf("write lock already set by %d\n",getpid());
		}
	}
	/*能上锁      */
	/*l_type可能被F_GETFL改变*/
	lock.l_type=type;
	/*根据不同type上锁或者解锁*/
	if((fcntl(fd,F_SETLKW,&lock))<0)
	{
		printf("lock failed:type =%d\n",lock.l_type);
	}
	/*上锁完毕，屏幕显示操作*/
	switch(lock.l_type)
	{
		case F_RDLCK:
		{
			printf("read lock set by %d\n",getpid());
		}
		break;
		case F_WRLCK:
		{
			printf("write lock set by %d\n",getpid());
		}
		break;
		case F_UNLCK:
		{
			printf("release lock set by %d\n",getpid());
		}
		break;
		default://任何条件都不匹配，default应放在所有case之后，加break
		break;
	}	
}
#include<sys/types.h>
#include<sys/file.h>
#include<sys/stat.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
int main(void)//测试lock_set函数
{
	int fd;
	/*打开文件*/
	fd=open("hello",O_RDWR|O_CREAT，0644);
	if(fd<0)
	{
		printf("open file failed\n");
	}
	/*给文件上锁*/
	lock_set(fd,F_WRLCK);
	lock_set(fd,F_UNLCK);
	close(fd);
	return 0;
}

}//例程

}//底层文件I/O操作

三、多路复用（select()和poll()）

多路复用就是为了使进程能够从多个阻塞I/O中获得自己想要的数据并继续执行接下来的任务
其主要的思路：同时监视多个文件描述符，如果有文件描述符的设定状态的被触发，就继续执行进程，
如果没有任何一个文件描述符的设定状态被触发，进程进入sleep
*******创建一个描述符集合，先清零，再决定监测哪些描述符；有描述符状态变化，就执行操作；操作完成把处理完的描述符清理掉*********
{//多路复用
/***********************************************************************************/
理论
{//理论
3.1 I/O处理模型有5中：
  ①阻塞I/O模型：   若所调用的I/O函数没有完成相关的功能，则会使进程挂起，直到相关数据到达才会返回。
				   对管道设备、终端设备和网络设备进行读写时经常出现这种情况。
  ②非阻塞模型：    当请求的IO操作不能完成任务时，则不让进程休眠，而且立即返回。
  ③IO多路转接模型：如果请求的IO操作阻塞，且它不是真正的阻塞IO，而是让其中的一个函数等待，在这其间IO还能进行其他操作
				   select()和poll()就属于这种类型。
  ④信号驱动IO模型：通过安装一个信号处理程序，系统可以自动捕获特定的信号的到来，从而启动IO
				   这是由内核通知用户何时可以启动一个IO操作决定的。
  ⑤异步IO模型：	   当一个描述符已准备好，可以启动IO时，进程会通知内核

3.2 select()：该函数可以设置程序中每个关系的文件描述符的条件、希望等待的时间等
			  返回时内核会通知用户已经准备好的文件描述符的数量、已经准备好的条件等。
****优缺点****select目前几乎在所有的平台上支持，其良好跨平台支持也是它的一个优点
			  select的一 个缺点在于单个进程能够监视的文件描述符的数量存在最大限制，在Linux上一般为1024
#include<sys/types.h>
#include<sys/time.h>
#include<unistd.h>
int select(int numfds,fd_set *readfds,fd_set *writefds,fd_set *exeptfds,struct timeval *timeout)
******************监测多少个文件描述符，读、写、可执行分别的集合，设置定时***********************
函数传入值	
			numfds：  ***需要监视的文件描述符的最大值加1*******,文件描述符从0开始
			readfds： 由select()监视的读文件描述符集合
			writefds：由select()监视的写文件描述符集合
			exeptfds：由select()监视的异常处理文件描述符集合
			timeout	：NULL：永远等待，直到捕捉到信号或文件描述符已准备好为止
					  具体值：struct timeval 类型的指针，若等待了timeout时间还没有检测到任何文件描述符准备好，就立即返回
					  0 ：从不等待，测试所有指定的描述符并立即返回
					  struct timeval
						{
							long tv_sec;     /* 秒 */
							long tv_unsec;   /* 微妙 */
						}
函数返回值：大于0：成功，返回准备好（读集合read不阻塞；写集合write不阻塞；异常条件集里面有异常条件）的文件描述符的数量
		    0：超时
			-1 ：出错

*******************对文件描述符进行处理的宏函数**********************

FD_ZERO(fd_set *set)	    	清除一个文件描述符集合
FD_SET(int fd,fd_set *set)		将一个文件描述符加入文件描述符集合
FD_CLR(int fd,fd_set *set)		将一个文件描述法从文件描述符集中清除
FD_ISSET(int fd,fd_set *set)	如果文件描述法fd为fd_set集合中的一个元素，则返回非零值
								可以用于调用select()之后测试文件描述符集合中的文件描述符是否有变化
***************************使用方法 *********************************
在使用select()之前，首先使用FD_ZERO()和FD_SET()来初始化文件描述符集合；
在使用select()时，循环使用FD_ISSET()来测试描述符集合；
在执行完相关描述符的操作后，使用FD_CLR()来清除描述符集合；

3.3 poll()函数
***优缺点*****pollfd并没有最大数量限制（但是数量过大后性能也是会下降）
#include<sys/types.h>
#include<poll.h>
int poll(struct pollfd fds[],int numfds,int timeout)
*******************需要设置监测文件描述符的数量，监测哪个状态，等多久***************
函数传入值	
fds[]：struct pollfd 结构,用于描述需要对哪些文件的哪种类型的操作进行监控。
					struct pollfd
					{
					     int         fd;     /* 需要监听的文件描述符 */
					     short     events;   /* 需要监听的事件 */
					     short     revents;  /* 实际发生了的事件，也就是返回结果*/
					}
					events  成员描述需要监听哪些类型的事件，可以用以下几种标志来描述。
							POLLIN：  文件中有数据可读
							POLLPRI： 文件中有紧急数据可读
							POLLOUT： 可以向文件写入数据
					revents		
							POLLERR： 文件中出现错误，只限于输出
							POLLHUP： 与文件的连接被断开了，只限于输出
							POLLNVAL：文件描述符是不合法的，即它并没有指向一个成功打开的文件
numfds： 需要监听的文件个数，即第一个参数所指向的数组中的元素数目
timeout：表示poll阻塞的超时时间（毫秒）。如果该值小于等于0，则表示无限等待
函数返回值:成功：返回大于0的值,表示事件发生的pollfd结构的个数
		   0：超时
		   -1：出错
3.4 epoll 3个函数
(1)int epoll_create(int size)
创建一个epoll的句柄，size用来告诉内核需要监听的数目一共有多大
当创建好epoll句柄后，它就是会占用一个fd值，在linux下如果查看/proc/进程id/fd/，是能够看到这个fd的
所以在使用完epoll后，必须调用close() 关闭，否则可能导致fd被耗尽
输入值：
size并不是限制了epoll所能监听的描述符最大个数，只是对内核初始分配内部数据结构的一个建议
返回：epoll描述符epfd；-1表示创建失败

(2)int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event)
对指定描述符fd执行op操作，event是与fd关联的监听事件
输入值：
epfd：epoll_create() 的返回值
op  ：EPOLL_CTL_ADD    //注册新的fd到epfd中；
	  EPOLL_CTL_MOD    //修改已经注册的fd的监听事件；
	  EPOLL_CTL_DEL    //从epfd中删除一个fd；
fd  ：需要监听的fd
*event：
            typedef union epoll_data {
                void *ptr;
                int fd;
                __uint32_t u32;
                __uint64_t u64;
            } epoll_data_t;

            struct epoll_event {
                __uint32_t events;      /* Epoll events */
							EPOLLIN     //表示对应的文件描述符可以读（包括对端SOCKET正常关闭）；
							EPOLLOUT    //表示对应的文件描述符可以写；
							EPOLLPRI    //表示对应的文件描述符有紧急的数据可读（这里应该表示有带外数据到来）；
							EPOLLERR    //表示对应的文件描述符发生错误；
							EPOLLHUP    //表示对应的文件描述符被挂断；
							EPOLLET     //将EPOLL设为边缘触发(Edge Triggered)模式，这是相对于水平触发(Level Triggered)来说的。
							EPOLLONESHOT//只监听一次事件，当监听完这次事件之后，如果还需要继续监听这个socket的话，需要再次把这个socket加入到EPOLL队列里。
                epoll_data_t data;      /* User data variable */
            };

(3)int epoll_wait(int epfd, struct epoll_event * events, int maxevents, int timeout)
等待epfd上的io事件，最多返回maxevents个事件
输入值：
epfd：epoll_create() 的返回值
events：从内核得到事件的集合
maxevents：告之内核这个events有多大
}//理论


例程
{//例程
	1.select例程
	{//select例程
	设置监测数量，得到文件描述符，初始化集合，判断集合是否改变，执行相应操作，关闭文件，清理集合
				/*multiplex_select*/
			#include<fcntl.h>
			#include<stdio.h>
			#include<unistd.h>
			#include<stdlib.h>
			#include<time.h>
			#include<errno.h>
			#define MAX_BUFFER_SIZE		1024	//缓冲区大小
			#define IN_FILES		    3		//多路复用输入文件数目
			#define TIME_DELAY			60		//超时秒数
			#define MAX(a,b)			((a>b)?(a):(b))
			int main(void)
			{
				int fds[IN_FILES];
				char buf[MAX_BUFFER_SIZE];
				int i,res,real_read,maxfd;
				struct timeval tv;
				fd_set inset,tem_inset;
				//得到文件描述符
				fds[0]=0;
				if((fds[1]=open("in1",O_RDONLY|O_NONBLOCK))<0)
				{
					printf("open in1 error\n");
				}
				if((fds[2]=open("in2",O_RDONLY|O_NONBLOCK))<0)
				{
					printf("open in2 error\n");
				}
				/*取出两个文件中描述符大的*/
				maxfd=MAX(MAX(fds[0],fds[1]),fds[2]);
				/*初始化读集合*/
				FD_ZERO(&inset);
				for(i=0;i<IN_FILES;i++)
				{
					FD_SET(0,&inset);
				}
				tv.tv_sec=TIME_DELAY;
				tv.tv_unsec=0;
				/*循环测试文件描述符是否准备就绪，使用select对相关文件描述符进行操作*/
				while(FD_ISSET(fds[0],&inset)||FD_ISSET(fds[1],&inset)||FD_ISSET(fds[2],&inset))
				{
					/*文件描述符备份，避免每次进行初始化*/
					tem_inset=inset;
					res=select(maxfd+1,&tem_inset,NULL,NULL,&tv);
					switch(res)
					{
						case -1:
						{
							printf("select error\n");
						}
						break;
						case 0:
						{
							printf("timeout\n");
						}
						break;
						default://检测描述符是否变化
						{
							for(i=0;i<IN_FILES;i++)
							{
								if(FD_ISSET(fds[i],&inset))//描述符变化
								{
									memset(buf,0,MAX_BUFFER_SIZE);
									real_read=read(fds[i],buf,MAX_BUFFER_SIZE);//读变化的描述符对应的文件
									if(real_read<0)
									{
										printf("read error\n");
										exit(1);
									}
									else if(real_read==0)//读到文件尾，视为正常
									{
										close(fds[i]);//关闭文件
										FD_CLR(fds[i],&inset);//把文件描述符从集合中删除
									}
									else//正常读
									{
										if(i==0)//主程序终端控制
										{
											if(buf[0]=='q'||buf[0]=='Q')
											{
												exit(1);
											}
										}
										else//显示管道输入字符
										{
											buf[real_read]='\0';
											printf("%s\n",buf);
										}
									}
								}
							}
						}
						
						break;
					}
				}
				return 0;
			}
	}//select例程
	2.poll例程
	{//poll例程
	设置需要监测的数量，得到文件描述符，设置每个文件描述符监测内容，有动作就执行相应操作
	#include<fcntl.h>
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<time.h>
#include<errno.h>
#include<poll.h>
#define MAX_BUFFER_SIZE		1024	//缓冲区大小
#define IN_FILES		    3		//多路复用输入文件数目
#define TIME_DELAY			60		//超时秒数
#define MAX(a,b)			((a>b)?(a):(b))
int main(void)
{
	struct pollfd fds[IN_FILES];
	char buf[MAX_BUFFER_SIZE];
	int i,res,real_read,maxfd;
	
	//得到文件描述符
	fds[0]=0;
	if((fds[1]=open("in1",O_RDONLY|O_NONBLOCK))<0)
	{
		printf("open in1 error\n");
	}
	if((fds[2]=open("in2",O_RDONLY|O_NONBLOCK))<0)
	{
		printf("open in2 error\n");
	}
	/*监听有数据可读的文件*/
	for(i=0;i<IN_FILES;i++)
	{
		fds[i].events=POLLIN;
	}
	/*初始化读集合*/
	FD_ZERO(&inset);
	for(i=0;i<IN_FILES;i++)
	{
		FD_SET(0,&inset);
	}
	tv.tv_sec=TIME_DELAY;
	tv.tv_unsec=0;
	/*循环测试文件描述符是否准备就绪，使用poll对相关文件描述符进行操作*/
	while(fds[0].events||fds[1].events||fds[2].events)
	{
		if(poll(fds,IN_FILES,0)<0)//失败
		{
			printf("poll error\n");
			exit(1);
		}	
		//poll成功
		
		for(i=0;i<IN_FILES;i++)
		{
			if(fds[i].revents)//已发生的事情
			{
				memset(buf,0,MAX_BUFFER_SIZE);
				real_read=read(fds[i].fd,buf,MAX_BUFFER_SIZE);//读变化的描述符对应的文件
				if(real_read<0)
				{
					printf("read error\n");
					exit(1);
				}
				else if(real_read==0)//读到文件尾
				{
					close(fds[i].fd);//关闭文件
					fds[i].events=0;//?????????????????????
				}
				else//正常读
				{
					if(i==0)//主程序终端控制
					{
						if(buf[0]=='q'||buf[0]=='Q')
						{
							exit(1);
						}
					}
					else//显示管道输入字符
					{
						buf[real_read]='\0';
						printf("%s\n",buf);
					}
				}//end of if real read
			}//end of if revents
		}//end of for

	}//end of while
	return 0;
}	
	}//poll例程
	
}//例程
	

/***********************************************************************************/
}//多路复用

























































