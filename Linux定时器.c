
struct sigevent//定时器到期产生的异步通知
{
	int sigev_notify;   //设置定时器到期后的行为
	int sigev_signo;    //设置产生信号的信号码
	union sigval   sigev_value; //设置产生信号的值，标识定时器；有几个定时器，可以用sigev_value区分哪个定时器产生的信号
	void (*sigev_notify_function)(union sigval);//定时器到期，从该地址启动一个线程
	pthread_attr_t *sigev_notify_attributes;    //创建线程的属性
}
/*sigev_notify取值:
SIGEV_NONE：定时器到期后什么都不做，只提供通过timer_gettime和timer_getoverrun查询超时信息。
SIGEV_SIGNAL：定时器到期后，内核会将sigev_signo所指定的信号，传送给进程，在信号处理程序中，si_value会被设定为sigev_value的值。
SIGEV_THREAD：定时器到期后，内核会以sigev_notification_attributes为线程属性创建一个线程，线程的入口地址为sigev_notify_function，传入sigev_value作为一个参数*/
union sigval
{
	int sival_int;  //integer value
	void *sival_ptr; //pointer value
}

struct itimerspec
{
    struct timespec it_interval;    // 时间间隔
    struct timespec it_value;       // 首次到期时间
};
 
struct timespec
{
    time_t  tv_sec    //Seconds.
    long    tv_nsec   //Nanoseconds.
};
//定时器工作时，先将it_value的时间值减到0，发送一个信号，
//再将it_interval的值赋给it_value，重新开始定时，如此反复。
//如果it_value值被设置为0，则定时器停止定时；
//如果it_interval等于0，那么表示该定时器不是一个时间间隔定时器，一旦it_value到期后定时器就回到未启动状态



1. int timer_create(clockid_t clockid, struct sigevent *sevp,timer_t timerid)
功能： 创建定时器
描述： 函数 timer_create 会创建一个timer(每进程), 返回的timer id 在调用进程中是唯一的, 创建后的timer处于停止(disarmed)状态.
参数：  clock_id说明定时器是基于哪个时钟的
			1. CLOCK_REALTIME :Systemwide realtime clock.(系统实时时间，即日历时间)
			2. CLOCK_MONOTONIC:Represents monotonic time. Cannot be set.(从系统启动开始到现在为止的时间)
			3. CLOCK_PROCESS_CPUTIME_ID :High resolution per-process timer(本进程启动到执行到当前代码，系统CPU花费的时间).
			4. CLOCK_THREAD_CPUTIME_ID :Thread-specific timer(本线程启动到执行到当前代码，系统CPU花费的时间).
			5. CLOCK_REALTIME_HR :High resolution version of CLOCK_REALTIME(CLOCK_REALTIME的细粒度（高精度）版本).
			6. CLOCK_MONOTONIC_HR :High resolution version of CLOCK_MONOTONIC(CLOCK_MONOTONIC的细粒度版本)
		*sevp  设置了定时器到期时的通知方式和处理方式，使用前要初始化(memset等), 否则可能出现timer到期无动作的异常情况
				sevp传入NULL，那么定时器到期会产生默认的信号，对CLOCK_REALTIMER来说，默认信号就是SIGALRM，
				如果要产生除默认信号之外的其他信号，程序必须将evp->sigev_signo设置为期望的信号码
		timerid  创建的timer的id 通过这个指针返回
返回值：成功0，失败-1	

2.int timer_delete (timer_t timerid)
功能： 通过timder id删除指定的 timer
描述： 如果调用这个函数时,timer处于激活(armed)状态, 删除前会先将这个timer状态变更为未激活(disarmed)状态.
	   挂起等待timer(删除的)产生信号的行为是未定义的
返回值：成功0，失败-1	

3. int timer_settime(timer_t timerid, int flags,const struct itimerspec *new_value,struct itimerspec *old_value)
功能： 启动/停止或重置定时器
参数：
timerid		指定的timer
flags		0 ：new_value->it_value 表示希望timer首次到期时的时间与启动timer的时间间隔.
			TIMER_ABSTIME：new_value->it_value 表示希望timer首次到期的绝对时间.
				（如果new_value->it_value 设定的绝对时间 早于 当前的绝对时间, 那么timer会立即到期.）
				（如果时钟 CLOCK_REALTIME 被调整了,那么timer的首次过期时间也会适当调整.）
new_value	new_value 有2个子域: it_value 和 it_interval
			⑴ it_value : 用于设置首次timer到期的时间, 也是 启动/停止 timer的控制域;
			⑵ it_interval : 用于设置timer循环的时间间隔, 如果其值不为0(秒和纳秒至少有一个不为0),每次timer到期时,timer会使用new_value->it_interval的值重新加载timer;
			如果其值为0, timer只会在it_value指定的时间到期一次,之后不会重新加载timer.
			①启动timer：
			前提:timer处于停止(disarmed)状态,否则就是重置timer.
			设置:new_value->it_value的值是非0值(秒和纳秒都不等于0或者其中一个不等于0).
			结果:timer变为启动(armed)状态.
			② 停止timer：
			设置:new_value->it_value的的值为0(秒和纳秒都为0).
			结果:timer变为停止(disarmed)状态.
			③重置timer：
			前提:timer处于已启动(armed)状态,否则就是启动timer
			设置:new_value->it_value的的值不为0(秒和纳秒至少有一个不为0)
			结果:timer仍为(armed)状态.之前的参数(即new_value(it_value 和 it_interval))设置会被覆盖.
old_value	取得上一次的设置的new_value
返回值：成功0，失败-1	

4. int timer_gettime(timer_t timerid, struct itimerspec *curr_value)
功能： 获得定时器的到期时间和间隔
timerid	true	IN	指定的timer
curr_value	true	OUT	curr_value->it_value : 离timer到期的剩余时间
curr_value->it_interval : timer的循环时间间隔
返回值：成功0，失败-1	

5.int timer_getoverrun(timer_t timerid)
功能： 获得定时器超限的次数
描述：当一个timer到期并且上一次到期时产生的信号还处于挂起状态时,不会产生新的信号(即丢弃一个信号),这就是定时器超限(overrun), 丢弃的信号数量就是 overrun count。
对于一个给定的timer, 在任何时间点只能有一个信号在进程中排队, 这是POSIX.1-2001中指定的, 因为不这样做,排队信号的数量很容易达到系统的上限.
因为系统调度延迟或者信号被暂时阻塞都会造成信号产生到信号被发送( (e.g., caught by a signal handler))或者接收((e.g., using sigwaitinfo(2)))之间有一个延迟的时间段,在这个时间段中可能会有多次的timer到期.
程序可以通过调用timer_getoverrun来确定一个指定的定时器出现这种超限的次数, 从而精确能精确的计算出在给定时间内timer到期了多少次。
定时器超限只能发生在同一个定时器产生的信号上。多个定时器，甚至是那些使用相同的时钟和信号的定时器，所产生的信号都会排队而不会丢失。
如果超限运行的次数等于或大于 {DELAYTIMER_MAX}，则此调用会返回











































