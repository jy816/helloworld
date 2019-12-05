Linux C网络编程基础

一、TCP/IP概述
{//TCP/IP概述
1. TCP/IP四层模型，从上到下
		应用层		负责应用程序的网络访问，通过端口号识别不同进程
					telnet ftp
		传输层		连接与建立端对端之间通信，根据数据传输方式选择通信协议
					TCP    UDP
		网络层		将数据帧封装成IP数据包，进行路由算法（寻找最优路径）
					ICMP   IGMP  IPV4  IPV6：IP在主机和网络之间寻址和路由数据包
		网络接口层  二进制流转换成数据帧，并进行发送和接收，数据帧是独立的网络信息传输单元
					APR获得同一物理网络中硬件主机地址  MPLS
其中，网络编程主要涉及TCP/UDP
2. TCP
2.1 概述
	TCP实现程序之间的数据传递，程序通过socket来使用TCP
	IP可以区分网络中两个设备，socket可以区分网络中两个应用程序
2.2 三次握手协议
	TCP通过三次握手进行初始化
	目的：使数据段的发送和接收同步，告诉其他主机其一次可接收的数据量，并建立虚拟连接
	过程：
		  (1)初始化主机（客户端）使用同步标志置位的数据段发出会话请求
		  (2)接收主机（服务器）发回数据段表示回复（同步标志置位，即将发送数据段的起始字节顺序号，应答并带有将收到下一个数据段的字节顺序号）
		  (3)请求主机（客户端）回送一个数据段，带有确认顺序号和字节顺序号
2.3 协议选择
	可靠性 TCP；实时性 UDP
Linux套接字结构定义

一个socket{协议，本地地址，本地端口}
一个完整套接字{协议，本地地址，本地端口，远程地址，远程端口}
类型：
		(1)流式socket（SOCK_STREAM）,使用TCP协议
		(2)数据报socket（SOCK_DGRAM）,使用UDP协议
		(3)原始socket
}//TCP/IP概述


二、Linux C的网络基础操作函数
{//Linux C的网络基础操作函数
1. 地址结构相关处理
<netinet/in.h>
定义一种结构类型sockaddr_in,通常在涉及TCP/IP的编程协议中使用
		struct sockaddr_in
		{
		int sa_len;                     //长度单位，不必设置，通常情况下固定长度为16字节
		short int sa_family;            //地址族
		unsigned short int sin_port;    //端口号（网络字节序）
		struct in_addr sin_addr;        //IP地址（网络字节序）
		unsigned char sin_zero[8];      //填充0以保持与struct sockaddr同样大小
		};
		struct sin_addr
		{
		in_addr_t s_addr;    //32位IPv4地址，网络字节序
		};
		sa_family可选值：
						AF_INET		IPv4协议
						AF_INET6	IPv6协议
						AF_LOCAL	UNIX协议
						AF_LINK		链路地址协议
						AF_KEY		密钥套接字

2. 字节顺序转换函数族
计算机的数据存储有两种形式：大端格式和小端格式
·大端格式：字数据的高位存储在低地址中
·小端格式：字数据的高位存储在高地址中
网络中的数据传输采用的是大端格式，而计算机操作系统既有大端格式（大型机）也有小端格式（一般PC机）。
<arpa/inet.h>
提供了4个函数用于处理大端格式和小端格式的数据转换
/*
    函数名中，h代表host，n代表network，s代表short，l代表long
    32位的long数据通常存放IP地址，16位的short数据通常存放端口号
    这些函数调用成功后返回处理后的值，调用失败则返回-1
*/
uint32_t htonl(uint32_t hostlong);    //将32位PC机数据（小端格式）转换为32位网络传输数据（大端格式）
uint16_t htons(uint16_t hostshort);   //将16位PC机数据（小端格式）转换为16位网络传输数据（大端格式）
uint32_t ntohl(uint32_t netlong);     //将32位网络传输数据转换为32位PC机数据
uint16_t ntohs(uint16_t netshort);    //将16位网络传输数据转换为16位PC机数据

3. IP地址转换函数族
<arpa/inet.h>
“点分十进制”表示的IP地址与二进制表示的IP地址相互转换的函数族
用户表达地址一般用十进制，socket编程用二进制
p 十进制；n二进制
3.1 inet_pton(int family,const char *strptr,void *addrptr)
family:		AF_INET		IPv4协议
			AF_INET6	IPv6协议
strptr:要转化的值
addrptr：转化后的值
返回值：0；-1
3.2 inet_ntop(int family,void *addrptr,char *strptr,size_t len)
family:		AF_INET		IPv4协议
			AF_INET6	IPv6协议
addrptr:转化后的地址
strptr:要转化的值
len：转化后的值
返回值：0；-1

4. 域名转换函数
<netdb.h>
struct hostent
{
    char *h_name;          //主机的正式名称
    char *h_aliases;       //主机的别名
    int h_addrtype;        //主机的地址类型，IPv4为AF_INET
    int h_length;          //主机的地址长度，对于IPv4是4字节，即32位
    char **h_addr_list;    //主机的IP地址列表
}；

4.1 
struct hostent *gethostbyname(const char *name)
name：指向存放域名或主机名的字符串
返回值：成功  hostent类型指针；失败  NULL
4.2
struct hostent *gethostbyaddr(const void *addr, socklen_t len, int type)
ddr：一个指向含有地址结构（in_addr）的指针
len：此结构的大小，对于IPv4而言其值为4，对于IPv6则是16
type：协议类型
返回值：成功  hostent类型指针；失败  NULL	
}//Linux C的网络基础操作函数


三、Linux网络套接字操作函数
{//Linux网络套接字操作函数
理论
{
1. 创建套接字描述符函数，建立一个socket连接
#include <sys/types.h>
#include <sys/socket.h>
int socket(int domain, int type, int protocol);    //创建成功的返回值为整型的套接字描述符
domain：套接字的协议类型
		AF_UNIX, AF_LOCAL	本地交互协议
		AF_INET				IPv4协议
		AF_INET6			IPv6协议
		AF_ROUTE			路由套接字
		AF_KEY				密钥套接字
		AF_NETLINK			内核接口设备协议
		AF_IPX				IPX-Novell协议
		AF_X25				ITU-T X.25/ISO-8208协议
		AF_AX25				业余无线电AX.25协议
		AF_ATMPVC			原始ATM接入协议
		AF_APPLETALK		苹果公司的Appletalk协议
		AF_PACKET			底层数据包接口
type：用于指定当前套接字类型
		SOCK_STREAM（字节流）、SOCK_DGRAM（数据报））、SOCK_RAW（原始套接字）
protocol：除了在使用原始套接字以外，通常情况下设置为0，以表示使用默认的协议
返回值：成功   套接字描述符；失败  -1

2. 绑定套接字函数，本地IP绑定到端口号
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
sockfd： 创建套接字时得到的套接字描述符
addr：   本地地址（端口号、地址；不指定地址，内核随意分配一个临时端口给应用程序）
addrlen：套接字对应的地址结构长度
返回值：0；-1
在C/S模式下服务器和客户端都可以使用bind函数来设置套接字地址，通常来说有以下5种模式
		C/S			IP		 port		说明
		服务器	INADDR_ANY	非0值	指定服务器的公认端口号（服务器最经常使用的绑定方式）
		服务器	本地IP地址	非0值	指定服务器的IP地址和公认端口号（服务器只接收来自对应于这个IP地址的特定网络设备端口的客户端连接）
		客户端	INADDR_ANY	非0值	指定客户端的连接端口号（系统会自动为它选择一个未使用的端口号，并且用本地的IP地址来填充套接字地址中的相应项）
		客户端	本地IP地址	非0值	指定客户端的IP地址和连接端口号（客户端使用指定的网络设备接口和端口号进行通信）
		客户端	本地IP地址	0		指定客户端的IP地址（客户端使用指定的网络设备接口进行通信，系统自动为客户端选择一个未使用的端口号。
														一般情况下，只有在主机有多个网络设备接口时使用）
在编写客户端程序时，通常不适用固定的端口号，除非是在必须使用特定端口的情况下
服务器执行主动关闭操作：
服务器最后进入TIME_WAIT状态。当客户机再次与这个服务器进行连接时，仍使用相同的客户机端口号，于是这个连接与前一次连接的套接字对完全相同
这是因为前一次连接处于TIME_WAIT状态，并未完全消失，所以这次连接请求被拒绝，函数connect以错误返回。
客户端执行手动关闭操作：
客户端最后进入TIME_WAIT状态，当立刻再次执行这个客户端程序时，客户机将继续与这个固定客户机端口号绑定
但因为前一次连接处于TIME_WAIT状态，并未消失，系统会发现这个端口号仍被占用，所以这次绑定操作失败，函数bind以错误返回。

3. 倾听套接字切换函数
int listen(int sockfd, int backlog)
sockfd： 套接字描述符
backlog：请求队列的最大长度
返回值：0；-1
socket函数创建的套接字是主动套接字，可以用它来进行主动连接（调用connect函数），但不能接受连接请求，而用于服务器的套接字必须能够接收客户端请求。
listen函数将一个尚未连接的主动套接字转换成为一个被动套接字，被动套接字可以接收请求。

若要创建一个倾听套接字，
(1)调用socket函数创建一个主动套接字
(2)调用bind函数将套接字与服务器套接字地址绑定在一起
(3)调用listen函数进行转换。这3个操作是所有TCP服务器必须执行的操作
而对于参数backlog的作用，TCP协议为每个倾听套接字维护两个队列：

·未完成连接队列：每个尚未完成3次握手操作的TCP连接在这个队列中占有一项。
TCP协议在接收到一个客户机的SYN数据报后，在这个队列中创建一个新条目，然后发送确认消息以及自己的SYN数据报（ACK+SYN），等待客户端对自己的SYN进行确认。
此时，套接字处于SYN_RCVD状态，这个条目保存在队列中，直到客户端返回对自己SYN消息的确认，或者连接超时。

·已完成连接队列：每个已经完成3次握手操作，但尚未被应用程序接收（调用accept函数）的TCP连接在这个队列中占有一项。

参数backlog指定倾听套接字的完成连接对了的最大长度，表示这个套接字能够接收的最大数目的未接收（unaccepted）连接。
如果当一个客户端的SYN消息到达，但这个队列已满，TCP协议将会忽略这个SYN，并且不会发送RST数据报

4. 建立连接函数
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
返回值：0；-1
客户端调用函数connect函数来主动建立连接时，连接过程中可能有以下几种错误情况：
如果客户端TCP协议没有接收到对它的SYN确认信息，则函数以错误返回，错误类型为ETIMEOUT。

如果远程TCP协议返回一个RST数据报，则函数立即以错误返回，错误类型为ECONNREFUSED。
当在SYN数据报指定的目的端口号没有服务器进程在等待连接时，会发送RST数据报，向客户机报告这个错误。

如果客户机的SYN数据报导致某个路由器产生“目的地不可达”类型的ICMP消息，则函数以错误返回，错误类型为EHOSTUNREACH或ENETUNREACH
通常情况下，TCP协议在接收到ICMP消息后回记录下这个消息并继续发送几次SYN数据报，如果都宣告失败，函数才会以错误返回。

如果调用函数connect失败，应该用函数close关闭这个套接字，不能再次用这个套接字来调用函数connect

5. 发送和接收函数
ssize_t send(int sockfd, const void *buf, size_t len, int flags)
ssize_t recv(int sockfd, void *buf, size_t len, int flags)
sockfd:套接字描述符
buf:发送数据的指针
len:数据长度
flags:  0 
返回值:成功，代表接收/发送成功的字节数（函数发送的实际长度可能小于其指定的长度）;失败则返回-1

6. 关闭连接函数
int close(int fd)
	
}
例程
{
e.g服务器和客户端
/*server.c*/
#include<sys/types.h>
#include<sys/socket.h>
#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>
#include<unistd.h>
#include<netinet/in.h>
#define PORT			 4321
#define BUFFER_SIZE		 1024
#define MAX_QUE_CONN_NM  5
int main()
{
	struct sockadd_in server_sockaddr,client_sockaddr;
	int sin_size,recvbytes;
	int sockfd,client_fd;
	char buf[MAX_QUE_CONN_NM];
	
	/*建立socket连接*/
	if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1)//IPV4 字节流套接字
	{
		perror("socket");
	}
	printf("socket id =%d\n",sockfd);
	/*设置socket_in结构体参数*/
	server_sockaddr.sin_family=AF_INET;//IPV4
	server_sockaddr.sin_port=htons(PORT);//端口号  主机字节序————网络字节序
	server_sockaddr.sin_addr.s_addr=INADD_ANY;//IP地址     INADD_ANY指定地址为0.0.0.0的地址
	bzero(&(server_sockaddr.sin_zero),8);
	
	int i=1;//允许重复使用本地地址和套接字进行绑定
	setsockopt(sockfd,SOL_SOCKET,SO_REFUSEADDR,&i,sizeof(i));
	/*int setsockopt(int sockfd, int level, int optname,const void *optval, socklen_t optlen)
	sockfd：标识一个套接口的描述字
	level：选项定义的层次；支持SOL_SOCKET基本套接口、IPPROTO_TCPTCP套接口、IPPROTO_IPIPv4套接口和IPPROTO_IPV6 IPv6套接口
	optname：需设置的选项     SO_REUSERADDR 允许重用本地地址和端口 int
	optval：指针，指向存放选项待设置的新值的缓冲区
	optlen：optval缓冲区长度
	*/
	/*绑定函数bind()*/
	if(bind(sockfd,(struct sockaddr *)&server_sockaddr,sizeof(struct sockaddr))==-1)
	{
		perror("bind");
	}
	printf("bind success\n");
	
	/*listen()函数，创建未处理请求的队列*/
	if(listen(sockfd,MAX_QUE_CONN_NM)==-1)
	{
		perror("listen");
	}
	printf("listening..........");
	
	/*accept()函数，等待客户端连接*/
	if(client_fd=accept(sockfd,(struct sockaddr *)&client_sockaddr,&sin_size)==-1)
	{
		perror("accept");
	}
	/*recv()函数，接收客户端请求*/
	memset(buf,0,sizeof(buf));
	if(recvbytes=recv(client_fd,buf,BUFFER_SIZE,0)==-1)
	{
		perror("recv");
	}
	printf("receive a message: %s\n",buf);
	close(sockfd);
	return 0;
}

/*client.c*/
#include<sys/types.h>
#include<sys/socket.h>
#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>
#include<netdb.h>
#include<unistd.h>
#include<netinet/in.h>
#define PORT			 4321
#define BUFFER_SIZE		 1024
int main(int argc,char *argv[])
{
	int sockfd,sendbytes;
	char buf[BUFFER_SIZE];
	struct hostent *host;
	struct sockaddr_in serv_addr;
	
	if(argc<3)
	{
		fprintf(stderr,"USAGE:./client Hostname(or ip address)Text\n");
	}
	/*地址解析函数*/
	if(host=gethostbyname(argv[1])==NULL)
	{
		perror("gethostbyname");
	}
	memset(buf,0,sizeof(buf));
	sprintf(buf,"%s",argv[2]);
	
	
	/*创建socket*/
	if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1)
	{
		perror("socket");
	}
	printf("socket id =%d\n",sockfd);
	/*设置socket_in结构体参数*/
	server_sockaddr.sin_family=AF_INET;
	server_sockaddr.sin_port=htons(PORT);
	server_sockaddr.sin_addr=*((struct in_add *)host->h_addr);
	bzero(&(server_sockaddr.sin_zero),8);
	
	/*conect函数发起对服务器的连接*/
	if(connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr))==-1)
	{
		perror("connect");
	}
	
	/*发送消息给服务器*/
	if((sendbytes=sned(sockfd,buf,strlen(buf),0))==-1)
	{
		perror("send");
	}
	close(sockfd);
	return 0;

}
	
}
}//Linux网络套接字操作函数














































