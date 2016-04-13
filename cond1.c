#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
//范例:cond.c->cond1.c(if->while)
//生产者1:<-Z
//消费者1:->Z
//消费者1:空仓!
//生产者2:<-B
//消费者2:->B
//消费者1:BWGYKSegmentation fault (core dumped)
/**
  * 出现这种错误的原因是当消费者1打印出空仓以后，消费者1和消费者2同时在睡入条件变量，
  *
  * 生产者2生产了一件产品，消费者1和消费者2都被唤醒了，但是消费者2获得了锁(消费者1
  * 无法获得锁，阻塞)，并消费了这件产品，这个时候g_stock已经是
  * 0了，等消费者2释放了锁以后，消费者1就可以拿到锁了，然后继续从仓库中拿东西，
  * 所以就出现了段错误。
  */
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
//        if(g_stock >= MAX_STOCK){
        //第二个生产者线程获得锁以后，也要判断g_stock >= MAX_STOCK，如果
        //为真，就继续睡入条件变量
        while(g_stock >= MAX_STOCK){
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
//        if(!g_stock){
        //当第二个消费者线程获得锁以后，也要判断g_stock是否为0，如果
        //是，就继续睡入条件变量
        while(!g_stock){
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
    pthread_create(&tid,&attr,producer,"生产者1");
    pthread_create(&tid,&attr,producer,"生产者2");
    pthread_create(&tid,&attr,customer,"消费者1");
    pthread_create(&tid,&attr,customer,"消费者2");
    getchar();
    return 0;
}
#if 0
注意：当一个线程被从条件变量中唤出以后，导致其睡入条件变量的条件可能还需要再判断一次，因其随时有可能被其他线程修改。
因为从醒来到获得锁之间有一个时间间隙，这个时间间隙可能会有其他的线程改变导致其睡入条件变量的条件
当你的操作依赖于某个条件，而这个条件是另外一个线程提供的的时候，就要使用条件变量了。
#endif
