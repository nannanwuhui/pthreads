#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>//posix线程引入以后才加进来的。
unsigned int g_cn = 0;
sem_t g_sem;

void* add_proc(void* arg){
    unsigned int i;
    sem_wait(&g_sem);
    for(i = 0;i < 100000000;++i){
        ++g_cn;
    }
    sem_post(&g_sem);
    return NULL;
}
void* sub_proc(void* arg){
    unsigned int i;
    sem_wait(&g_sem);
    for(i = 0;i < 100000000;++i){
        --g_cn;
    }
    sem_post(&g_sem);
    return NULL;
}
int main(void){
    sem_init(&g_sem,0,1);
    pthread_t tid1,tid2;
    pthread_create(&tid1,NULL,add_proc,NULL);
    pthread_create(&tid2,NULL,sub_proc,NULL);
    pthread_join(tid1,NULL);
    pthread_join(tid2,NULL);
    printf("g_cn = %u\n",g_cn);
    sem_destroy(&g_sem);//释放资源
    return 0;
}
#if 0
三、信号量
信号量是一个计数器，用于控制访问有限资源的线程数。
#include <semaphore.h>
//创建信号量
Int sem_init(sem_t* sem,int pshared,unsigned int value);
Sem 　信号量ID，输出。
pshared 一般取０，表示调用进程的信号量。
非０表示该信号量可以共享内存的方式为多个进程所共享（linux暂不支持）。
value　－信号量初值。
／／信号量减１，不够减即阻塞。
Int sem_wait(sem_t* sem);（永远阻塞）
／／信号量减１，不够减即返回-1，errno为EAGAIN
Int sem_trywait(sem_t* sem); 够减返回0，非阻塞模式（不够减即返回-1，errno为EAGAIN）（永远不阻塞）
／／信号量减１，不够减即阻塞，直到abs_timeout超时返回-1，errno为ETIMEDOUT
Int sem_timedwait(sem_t* sem,const struct timespec* abs_timeout);（阻塞一段时间，超过返回-1）
Struct timespec{
	time_t tv_sec; //Seconds
	long tv_nsec;//Nanoseconds[0 -999999999]
};
//信号量加1
Int sem_post(sem_t* sem);（不会阻塞）
//销毁信号量,在init的时候会创建一些资源，这里是销毁这些资源。
Int sem_destroy(sem_t* sem);
范例:sem.c
注意：
1）信号量APIs没有声明在pthread.h中，而是声明在semaphore.h中，失败也不返回错误码，而是返回-1，同时设置errno.
2）互斥量任何时候都只允许一个线程访问共享资源，而信号量则允许最多value个线程同时访问共享资源。
#endif
