
只能有一个执行线程访问代码的临界区域，调协进程对共享资源的访问的
信号量是个计数器，为获得共享资源，测试控制该资源的信号量，为正，可以使用，使用后-1 ；为0，休眠，直至大于0唤醒
#include<sys/ipc.h>
#include<sys/sem.h>
#include<sys/types.h>
1、semget()函数
创建一个新信号量或取得一个已有信号量
int semget(key_t key, int num_sems, int sem_flags)
参数：
		key：键 一般使用ftok("pathname",'id')生成
		num_sems：需要的信号量数目，它的值几乎总是1；引用现有的信号量，为0
		sem_flags：权限|IPC_CREAT，不存在创建一个新的信号量，存在没关系；IPC_CREAT | IPC_EXCL则可以创建一个新的，唯一的信号量，如果信号量已存在，返回一个错误
返回值：成功返回一个相应信号标识符（非零），失败-1
2、semop()函数
改变信号量的值
int semop(int sem_id, struct sembuf *sem_opa, size_t num_sem_ops)
参数：
		sem_id：semget()返回的信号量标识符
		sembuf：
				struct sembuf
				{
					short sem_num; // 除非使用一组信号量，否则它为0
					short sem_op;  // 信号量在一次操作中需要改变的数据，通常是两个数，一个是-1，即P（等待）操作，
								   // 一个是+1，即V（发送信号）操作。
					short sem_flg; // 通常为SEM_UNDO,使操作系统跟踪信号，
								   // 并在进程没有释放该信号量而终止时，操作系统释放信号量
				};
		num_sem_ops：进行操作信号量的个数
返回值：成功0，失败-1
3、semctl()函数
直接控制信号量信息				
int semctl(int sem_id, int sem_num, int command, union semun arg)			
参数：				
	sem_id： semget()返回的信号量标识符			
	sem_num：信号量集合中的成员，在0和信号量数量-1 之间，表示第几个
	command：SETVAL：用来把信号量初始化为一个已知的值，这个值通过union semun中的val成员设置，其作用是在信号量第一次使用前对它进行设置
			 IPC_RMID：用于删除一个已经无需继续使用的信号量标识符	
	union semun： union semun 
					{
					int val;          //信号量初始值
					struct semid_ds *buf;
					unsigned short *arry;
					};
返回值：成功0，失败-1
/*************分隔线****************/				
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short int *array;
    struct seminfo *__buf;
};
/*封装好的信号量操作函数，初始化某个信号量*/
int init_sem(int sem_id, int init_value)
{
    union semun sem_union;
    sem_union.val = init_value;
    if(semctl(sem_id,0,SETVAL,sem_union) == -1) //初始化一个信号量，该信号量ID是sem_id，初始值是init_value  SETVAL把信号量初始化为一个已知的值  
    {
        perror("Initialize semaphore");
        return -1;
    }
    return 0;
}
/*封装好的信号量操作函数，删除某个信号量*/
int del_sem(int sem_id)
{
    union semun sem_union;
     if(semctl(sem_id,0,IPC_RMID,sem_union) == -1)//IPC_RMID用于删除一个已经无需继续使用的信号量标识符
     {
         perror("Delete semaphore");
         return -1;
     }
     return 0;
}
/*封装好的信号量操作函数，对某个信号量执行P操作*/
int sem_p(int sem_id)
{
    struct sembuf sem_b;
    sem_b.sem_num = 0;// 指定要操作的信号，0是第一个。除非使用一组信号量，否则它为0
    sem_b.sem_op = -1;// 信号量在一次操作中需要改变的数据，通常是两个数，一个是-1，即P（等待）操作， 一个是+1，即V（发送信号）操作
    sem_b.sem_flg = 0;//0 设置信号量的默认操作；通常为SEM_UNDO,如果该进程崩溃，则根据这个进程的UNDO记录自动恢复相应信号量的计数值；IPC_NOWAIT设置信号量操作不等待
    if (semop(sem_id,&sem_b,1) == -1)//1为进行操作信号量的个数
    {
        perror("P operation");
        return -1;
    }
    return 0;
}
/*封装好的信号量操作函数，对某个信号量执行V操作*/
int sem_v(int sem_id)
{
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op = 1;
    sem_b.sem_flg = 0;
    if (semop(sem_id,&sem_b,1) == -1)
    {
        perror("V operation");
        return -1;
    }
    return 0;
}










































