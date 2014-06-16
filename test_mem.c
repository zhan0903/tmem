#include<linux/init.h>
#include<linux/sched.h>
#include<linux/err.h>
#include<linux/module.h>
#include<linux/types.h>
#include <linux/kthread.h>
#include<linux/slab.h>
#include<linux/wait.h>
MODULE_LICENSE("Dual BSD/GPL");

#include "test_mem.h"

#define TMEM_VERSION "0.0.1" 
#define THREADS 8
#define BASIC_SIZE 1024*1024
#define CPUHZ 2599998

static int start_time,end_time;
static int flag = 0;
static DECLARE_WAIT_QUEUE_HEAD(wq);
static struct task_struct *_tsk; 

 
static __inline__ __u64 rdtsc(void) {
        __u32 lo,hi;

        __asm__ __volatile__
        (
         "rdtsc":"=a"(lo),"=d"(hi)
        );
        return (__u64)hi<<32|lo;
}

static int test_thread(void *data)
{
   char *buf_read;
   char *buf_write;
   int num_rd,count = 0;
   unsigned long start, intv=10*HZ;


   buf_read = vmalloc(BASIC_SIZE);
   buf_write = vmalloc(BASIC_SIZE);


   if(!buf_read || !buf_write)
    {
    TMEMERR("failed to allocate memory");
    }
     
    wait_event_interruptible(wq,flag != 0);
    TMEMERR("in thread:%i\n",current->pid);
    start = jiffies;
    start_time = rdtsc();

    do{
       memcpy(buf_write,buf_read,BASIC_SIZE);
       count++;
     }while((flag != 0) && time_before(jiffies, start+intv));
    
     end_time = rdtsc();
     num_rd = end_time - start_time;
     TMEMINFO("number of cpuT:%d copy times:%d  BASIC_SIZE:%d\n ",num_rd,count,BASIC_SIZE);
     
     return 0;
}

/*static do_event(char *input,char *output,int num)
{
       memcpy(output,input,num);
}*/

static int __init t_init(void)
{
     int ret,i;
     char *name;
     TMEMINFO("hello,world\n");

     name = kmalloc(20*sizeof(char),GFP_KERNEL);

     for(i = 0;i != THREADS;i++)
     {
        sprintf(name,"thread_name:%d",i);
        TMEMINFO("name::%s\n",name);
        _tsk = kthread_create(test_thread,NULL,name);
        if (IS_ERR(_tsk)) {
            ret = PTR_ERR(_tsk);
            _tsk = NULL;
            goto out;
         }   
        wake_up_process(_tsk);
     }
    
    schedule_timeout(2*HZ); 
    flag = 1;
    wake_up_interruptible(&wq);

    return 0;
out:
    return ret;
}

static void __exit t_exit(void)
{
   TMEMINFO("GOODBYE,MODULE\n");
}


module_init(t_init);
module_exit(t_exit);
