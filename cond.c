#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#define MAX_STOCK 5 //仓库容量
char g_storage[MAX_STOCK];//仓库
size_t g_stock = 0;//当前库存
pthread_mutex_t g_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t g_full = PTHREAD_COND_INITIALIZER;//满仓
pthread_cond_t g_empty = PTHREAD_COND_INITIALIZER;//空仓
//显示库存
void show(const char* who,const char* op,char prod){
    printf("%s:",who);
    size_t i;
    for(i = 0;i < g_stock;++i)
        printf("%c",g_storage[i]);
    printf("%s%c\n",op,prod);
}
//生产者线程
void* producer(void* arg){
    const char* who = (const char*)arg;
    for(;;){
        pthread_mutex_lock(&g_mtx);
        if(g_stock >= MAX_STOCK){
            printf("%s:满仓!\n",who);
            pthread_cond_wait(&g_full,&g_mtx);//睡入条件变量，同时释放互斥锁
        }
        char prod = 'A' + rand()%26;
        show(who,"<-",prod);
        g_storage[g_stock++] = prod;
        pthread_cond_signal(&g_empty);
        pthread_mutex_unlock(&g_mtx);
        usleep((rand()%100) * 1000);
    }
    return NULL;
}
//消费者线程
void* customer(void* arg){
    /**
     * 条件变量解决的问题，就是当条件不具备的时候，我就睡，当条件具备的时候，
     * 我就被唤醒。
     */
    const char* who = (const char*)arg;
    for(;;){
        pthread_mutex_lock(&g_mtx);
        if(!g_stock){
            printf("%s:空仓!\n",who);
            pthread_cond_wait(&g_empty,&g_mtx);//睡入条件变量，同时释放互斥锁
        }
        //g_stock是数组中最后一个元素的下一个元素的下标
        char prod = g_storage[--g_stock];
        show(who,"->",prod);
        /**
         * 这里的信号是线程的信号，不是进程的信号，
         * 信号发送以后，生产者的pthread_cond_wait不会立即返回，
         * 因为pthread_cond_wait返回需要两个条件：
         * 1、被唤醒 2、拿到互斥锁
         * 所以必须等到消费者执行过pthread_mutex_unlock(&g_mtx);
         * 释放互斥锁，生产者才能拿到互斥锁从pthread_cond_wait中返回
         */
        pthread_cond_signal(&g_full);
        pthread_mutex_unlock(&g_mtx);
        usleep((rand()%100) * 1000);
    }
    return NULL;
}
int main(){
    srand(time(NULL));//用系统时间初始化随机种子。
    //创建分离线程
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
    pthread_t tid;
    pthread_create(&tid,&attr,producer,"生产者");
    pthread_create(&tid,&attr,customer,"消费者");
    getchar();
    return 0;
}
#if 0
五、条件变量（内核对象）
比如现在有一个标志位，如果标志位为1，就可以继续执行，如果是0，就必须阻塞。
当然我们可以在执行自己的操作之前判断一下，如果标志位为1，执行，如果是0，就
while(ture){ sleep(1);}
然后还要轮询的去看看标志位是否变了，当其他的线程将标志位置为1，则就可以继续执行了。
这个工作在用户空间做不合适，因为会消耗cpu资源，所以才在内核空间做，内核空间做就很快内核，内核肯定不会通过循环了，它有自己的机制。条件变量。
阻塞就是让线程处于睡眠状态，也就是将线程移出调度序列。不参加调度也就不会消耗cpu时间了。
生产者消费者模型（可以是线程也可以是进程）
生产者：产生数据的线程，资源的提供者。
消费者：使用数据的线程，资源的消耗者。
通过缓冲区隔离生产者和消费者，与二者直连相比，避免互相等待，提高运行效率。
生产快于消费，缓冲区满，撑死。
消费快于生产，缓冲区空，饿死。
条件变量可以让调用线程在满足特定条件的情况下暂停。
线程的信号，不是进程的信号。
进程：IPC有自动同步机制
当消息队列为空的时候，msg_recieve会阻塞。
当另一个进程发送一个消息的时候，msg_recieve就立刻返回。
就跟网络是一样的，recieve当没有数据的时候就会阻塞，当send以后，它就会返回
线程的同步性需要自己来做。
数据采集和数据分析都会有这样的情况。
比如读取一个硬件的信息，然后把硬件的信息放在一个buffer里（队列，堆栈，数组），然后另外一个线程从buffer中读数据来分析这些数据，硬件的驱动从硬件读数据然后往buffer里放。
Int pthread_cond_init(pthread_cond_t* cond,const pthread_condattr_t* attr);
亦可：
Pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
//使调用线程睡入条件变量cond，同时释放互斥锁mutex
Int pthread_cond_wait(pthread_cond_t* cond,pthread_mutex_t* mutex);
Int pthread_cond_timedwait(pthread_cond_t* cond,pthread_mutex_t* mutex,const struct timespec* abstime);
Struct timespec{
	Time_t tv_sec;//Seconds
	Long tv_nsec;//Nanoseconds[0 - 999999999]
};
//从条件变量cond中唤出至少一个线程（可能唤出多个线程，但是唤出多少个，不能确定，唤出几个，哪几个由内核的调度算法来决定），令其重新获得原先的互斥锁
Int pthread_cond_signal(pthread_cond_t* cond);
注意：被唤出的线程此刻将从pthread_cond_wait函数中返回，但如果该线程无法获得原先的锁，则会继续阻塞在加锁上。
//从条件变量cond中唤出所有线程，全部唤醒，并不意味着全部从pthread_cond_wait函数中返回。因为还得可以获得锁才能返回。一个获得锁，返回以后，其他线程就会阻塞在获得锁上面，直到可以获得锁，才能返回。
Int pthread_cond_broadcast(pthread_cond_t* cond);
Int pthread_cond_destroy(pthread_cond_t* cond);
#endif
