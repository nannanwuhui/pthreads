#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#define MAX_CONNS 5 //最大连接数
#define MAX_USERS 50 //最大用户数,50个线程代表50个用户
sem_t g_sem;//信号量其实就是一个计数器
void* user_proc(void* arg){
    pthread_t tid = pthread_self();
    int sval;
    sem_getvalue(&g_sem,&sval);
    printf("%lu线程：等待数据库连接(还剩%d个空闲连接)...\n",tid,sval);
    /**
     * 阻塞的话，是在内核空间，不是在用户空间，很省资源，把线程移出调度序列，不耗cpu
     * 只有当sem_wait返回的时候，才会将线程重新加入调度序列。
     * 但是如果你自己做一个数组，0表示没有资源，1表示有资源，去轮询的话，会一直耗费cpu资源
     */
    sem_wait(&g_sem);
    sem_getvalue(&g_sem,&sval);
    printf("%lu线程:获得数据库连接,(还剩%d个空闲连接)!\n",tid,sval);
    usleep(1000000);
    sem_post(&g_sem);
    sem_getvalue(&g_sem,&sval);
    printf("%lu线程:释放数据库连接，（还剩%d个空闲连接）\n",tid,sval);
    return NULL;
}
int main(){
    sem_init(&g_sem,0,MAX_CONNS);
    size_t i;
    pthread_t tids[MAX_USERS];
    for(i = 0;i < MAX_USERS;++i)
        pthread_create(&tids[i],NULL,user_proc,NULL);
    for(i = 0;i < MAX_USERS;++i)
        pthread_join(tids[i],NULL);
    sem_destroy(&g_sem);
    return 0;
}
#if 0
访问数据库之前需要建立一个链接，然后可以通过链接来访问数据库，在某些应用中，为了提高效率，会做一个连接池，就是先把一些连接建好，比如5个，最多这个应用就5个连接。不能建立更多的连接，再多数据库受不了了。
多个用户共享有限的资源。
连接池，线程池，内存池。
#endif
