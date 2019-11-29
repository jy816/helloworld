#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/types.h>
1、shmget()函数
创建共享内存
int shmget(key_t key, size_t size, int shmflg)
参数：
	key：键 一般使用ftok("pathname",'id')生成
	size：以字节为单位指定需要共享的内存容量
	shmflg：权限标志，可以与IPC_CREAT做或操作，0666|IPC_CREAT
返回值：成功：一个与key相关的共享内存标识符（非负整数）；失败：-1
2、shmat()函数    -- at：attach
把共享内存连接到当前进程的地址空间
void *shmat(int shm_id, const void *shm_addr, int shmflg);
参数：
	shm_id:shmget()函数返回的共享内存标识   ；shmflg：标志位
	shm_addr:共享内存连接到当前进程中的地址位置
			0(推荐)系统自动查找进程地址空间，将共享内存附加到第一块有效内存区域，此时flag无效
			非0且flag未指定SHM_RND，连接到shm_addr指定地址                SHM_RND取整
			非0且flag指定SHM_RND，连接到shm_addr-(shm_addr mod SHMLBA)    SHMLBA低边界地址倍数
	shmflg：指定为SHM_RDONLY，只读方式连接，其他为读写方式
返回值：成功：指向共享内存第一个字节的指针；失败：-1
3、shmdt()函数    -- dt：detach
将共享内存从当前进程中分离，不是删除，只是当前进程不可用
int shmdt(const void *shmaddr)
参数：shmaddr为shmat()函数返回的地址指针
返回值：成功0，失败-1
4、shmctl()函数    -- ctl：control
控制共享内存
int shmctl(int shm_id, int command, struct shmid_ds *buf)
参数：
	shm_id：shmget()函数返回的共享内存标识符
	command：
			IPC_STAT：把shmid_ds结构中的数据设置为共享内存的当前关联值，即用共享内存的当前关联值覆盖shmid_ds的值。
			IPC_SET：如果进程有足够的权限，就把共享内存的当前关联值设置为shmid_ds结构中给出的值
			IPC_RMID：删除共享内存段，buf为0（删除了要权限没用了）
	buf：指向共享内存模式和访问权限的结构
			struct shmid_ds
			{
				uid_t shm_perm.uid;
				uid_t shm_perm.gid;
				mode_t shm_perm.mode;
			};
返回值：成功0，失败-1
















