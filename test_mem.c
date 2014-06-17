#include "test_mem.h"

#include<linux/sched.h>
#include<linux/slab.h>
#include<linux/wait.h>
MODULE_LICENSE("Dual BSD/GPL");


#define TMEM_VERSION "0.0.1" 
#define THREADS 8
#define BASIC_SIZE 1024*1024
#define CPUHZ 2599998

static int start_time,end_time;
static int flag = 0;
static DECLARE_WAIT_QUEUE_HEAD(wq);
static struct task_struct *_tsk; 
static struct mutex ioctl_mutex;
static struct para pa;

 
static __inline__ __u64 rdtsc(void) 
{
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


    buf_read = vmalloc(pa.copysize);
    buf_write = vmalloc(pa.copysize);


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

static long tmem_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{
    int ret = 0,i;
    char *name;
    

    mutex_lock(&ioctl_mutex);
    switch(cmd)
    {
       case TMEM_RUN:
           name = kmalloc(20*sizeof(char),GFP_KERNEL);
           if (copy_from_user
	       (&pa,(struct para __user *)arg, sizeof(pa))) 
               {
                    ret = -EFAULT;
	       	    break;
	       }
           for(i = 0;i != pa.threads;i++)
           {
              sprintf(name,"thread_name:%d",i);
              TMEMINFO("name::%s\n",name);
              _tsk = kthread_create(test_thread,NULL,name);
              if (IS_ERR(_tsk)) 
              {
                  ret = PTR_ERR(_tsk);
                  _tsk = NULL;
                  goto out;
              }
               wake_up_process(_tsk);
             }
             schedule_timeout(2*HZ);
             flag = 1;
             wake_up_interruptible(&wq);
             break;
         case TMEM_STOP:
             TMEMINFO("COMMAND STOP!");
             break;
          default:
              TMEMERR("UNKNOW COMMAND IN KERNEL");
    }      
   
out:
    mutex_unlock(&ioctl_mutex);
    return ret;
}


static const struct file_operations _tmem_ctl_fops = {
        .open = nonseekable_open,
        .unlocked_ioctl = tmem_ioctl,
        .owner = THIS_MODULE,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
        .llseek = noop_llseek
#else
        .llseek = no_llseek
#endif
};

static struct miscdevice _tmem_misc = {
        .minor = MISC_DYNAMIC_MINOR,
        .name = "tmemcontrol",
        .nodename = "tmemcontrol",
        .fops = &_tmem_ctl_fops
};

/*static do_event(char *input,char *output,int num)
{
       memcpy(output,input,num);
}*/

static int __init t_init(void)
{
    int r;
    TMEMINFO("hello,world\n");
    r = misc_register(&_tmem_misc);
    if(r)
    {
        TMEMERR("misc_register failed for control device");
        return r;
    }
    mutex_init(&ioctl_mutex);
    return 0;
}

static void __exit t_exit(void)
{
    TMEMINFO("GOODBYE,MODULE\n");
    if(misc_deregister(&_tmem_misc) < 0)
    {
         TMEMERR("misc_deregister failed for tmem control device");
    }
    mutex_destroy(&ioctl_mutex);
}


module_init(t_init);
module_exit(t_exit);
