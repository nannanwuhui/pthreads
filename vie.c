#include <stdio.h>
#include <string.h>
#include <pthread.h>
unsigned int g_cn = 0;
void* thread_proc(void* arg){
    unsigned int i;
    for(i = 0;i < 100000000;++i)
        ++g_cn;
    return NULL;
}
int main(void){
    pthread_t tid1,tid2;
    pthread_create(&tid1,NULL,thread_proc,NULL);
//    pthread_join(tid1,NULL);
    pthread_create(&tid2,NULL,thread_proc,NULL);
    pthread_join(tid1,NULL);
    pthread_join(tid2,NULL);
    printf("g_cn = %u\n",g_cn);
    return 0;
}
#if 0
当多个线程同时访问其所共享的进程资源时，需要相互协调，以防止出现数据不一致，不完整的问题，这就叫线程同步。

读内存是将数据从内存读到cpu的寄存器。
理想中的原子++
　　线程1		线程2			内存
指令　寄存器	　　　指令　寄存器		g_cn
读内存　０					0
算加法　１					0
写内存　１					1
			读内存　１		1
			算加法　２		1
			写内存　２		2
现实中的非原子++:
线程1	　　　　线程2　　　　内存
      线程1		线程2                  内存
指令	寄存器	    指令　　寄存器　　　g_cn
读内存　０	　　　			　０
		　读内存　　０　　　　０
算加法　１　　 　　    　　　　　　　０
		　算加法　　１　　　　０
写内存　１	　　　　　　　　　　　１
		　写内存　　１　　　　１
每个线程的栈是独立的，对应的寄存器也是独立的。
++是简单的操作，要是做更复杂的操作，像计算表达式，还混合有赋值，函数调用。下标的计算，指针的计算，都有可能因为线程的切换而带来混乱，因为所有的我们在高级语言里看到的一条指令，一句话，真正被编译完了以后，很可能都是多条指令，这些指令在执行的过程当中很可能因为线程的并发与竞争而产生一种未确定的后果。
进程中就没有这种问题，因为进程中你没法访问同一个变量。除非你用共享内存。
如果你用共享内存的话，进程之间同样也会有这种问题。
用IPC机制的时候也不会有这种并发的问题，因为它是数据的传输。
#endif
