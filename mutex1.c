#include <stdio.h>
#include <string.h>
#include <pthread.h>
unsigned int g_cn = 0;
//用这种方法就可以不用destroy了。
pthread_mutex_t g_mtx = PTHREAD_MUTEX_INITIALIZER;
void* thread_proc(void* arg){
    unsigned int i;
    for(i = 0;i < 100000000;++i){
        pthread_mutex_lock(&g_mtx);
        ++g_cn;
        pthread_mutex_unlock(&g_mtx);
    }
    return NULL;
}
int main(void){
    pthread_t tid1,tid2;
    pthread_create(&tid1,NULL,thread_proc,NULL);
    pthread_create(&tid2,NULL,thread_proc,NULL);
    pthread_join(tid1,NULL);
    pthread_join(tid2,NULL);
    printf("g_cn = %u\n",g_cn);
//    pthread_mutex_destroy(&g_mtx);//释放资源
    return 0;
}
