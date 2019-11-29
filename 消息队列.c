消息队列提供了一种从一个进程向另一个进程发送一个数据块的方法
#include<sys/ipc.h>
#include<sys/msg.h>
#include<sys/types.h>
1、msgget()函数
创建和访问一个消息队列
int msgget(key_t, key, int msgflg);
参数：
		key：键 一般使用ftok("pathname",'id')生成
		msgflg：权限|IPC_CREAT，不存在创建一个新的消息队列
返回值：成功：key命名的消息队列的标识符（非零整数），失败-1
2、msgsnd()函数
把消息添加到消息队列中
int msgsnd(int msgid, const void *msg_ptr, size_t msg_sz, int msgflg)
参数：
		msgid：msgget函数返回的消息队列标识符
		msg_ptr：指向准备发送消息的指针，消息的数据结构却有一定的要求（一个长整型成员变量开始的结构体）	
					消息队列结构体
					struct msgstru
					{
					   long msgtype;
					   char msgtext[256];
					};
		msg_sz：msg_ptr指向的消息的长度，不是结构体长度
		msgflg：控制当前消息队列满或队列消息到达系统范围的限制时将要发生的事情
				0,表示忽略，阻塞进程，直到发送消息成功为止
				设置IPC_NOWAIT出错立即返回
				否则：进程阻塞一直到有空间容纳要发送的消息
					  删除此消息队列	返回EIDRM错误（标识符被删除）
					  捕捉一个信号，从信号处理程序返回  返回EINTR错误
返回值：成功0，失败-1				  
3、msgrcv()函数
从一个消息队列获取消息
int msgrcv(int msgid, void *msg_ptr, size_t msg_st, long int msgtype, int msgflg)
参数：
		msgid：msgget函数返回的消息队列标识符
		msg_ptr：指向准备发送消息的指针
		msg_st：msg_ptr指向的消息的长度，不是结构体长度
		msgtype：可以实现一种简单的接收优先级
				0 ，就获取队列中的第一个消息
				>0 ，将获取具有相同消息类型的第一个信息
				<0 ，就获取类型等于或小于msgtype的绝对值的第一个消息
		msgflg：控制当队列中没有相应类型的消息可以接收时将发生的事情
				0,表示忽略，阻塞进程，直到收到消息为止
				IPC_NOWAIT：msgtype无效时的处理方法。msgtype无效，设置，则立即出错返回，否则阻塞进程，直至type有效或消息队列被删除
				IPC_NOERROR：处理长度大于msg_st情况。长度大于msg_st，该位被设置，接收消息，超出部分截断，正确返回，否则不接收消息，出错返回
返回值：成功返回放到接收缓存区中的字节数，然后删除消息队列中的对应消息。失败-1	
4、msgctl()函数
int msgctl(int msgid, int command, struct msgid_ds *buf)
参数：
		msgid：msgget函数返回的消息队列标识符
		command：IPC_STAT：把msgid_ds结构中的数据设置为消息队列的当前关联值，即用消息队列的当前关联值覆盖msgid_ds的值。
				 IPC_SET：如果进程有足够的权限，就把消息列队的当前关联值设置为msgid_ds结构中给出的值
				 IPC_RMID：删除消息队列      msgid_ds为NULL，删除了不需要权限和模式 
		buf：指向msgid_ds结构的指针，它指向消息队列模式和访问权限的结构
			 struct msgid_ds
			{
				uid_t shm_perm.uid;
				uid_t shm_perm.gid;
				mode_t shm_perm.mode;
			};
返回值：成功0，失败-1	




































