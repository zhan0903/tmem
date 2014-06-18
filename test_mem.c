#include "test_mem.h"

#include<linux/sched.h>
#include<linux/slab.h>
#include<linux/wait.h>
MODULE_LICENSE("Dual BSD/GPL");


#define TMEM_VERSION "0.0.1" 
/*#define THREADS 8*/
/*#define BASIC_SIZE 1024*1024*/ 
#define CPUHZ 2800000000 
#define TEST_TIME 10 

static int flag = 0;
static int tflag = 0;
static DECLARE_WAIT_QUEUE_HEAD(wq);
static struct task_struct *_tsk; 
static struct mutex ioctl_mutex;
static struct mutex count_mutex;
static struct para pa;
static unsigned long total_count = 0,total_nr = 0;
static unsigned long start_time,end_time;

 
static __inline__ __u64 rdtsc(void) 
{
    __u32 lo,hi;
    __asm__ __volatile__
    (
       "rdtsc":"=a"(lo),"=d"(hi)
    );
    return (__u64)hi<<32|lo;
}

/*int rdtsct()
{
   return asm("rdtsc");
}*/

static int calcu_thread(void *data)
{
    unsigned long total_copysize,total_test_time;
    int total_average_bandwidth,total_average_copy_time;
    
    while(true)
    {
	if(tflag == 0)break;
        schedule();
    }
 
    total_copysize = pa.copysize*total_count/(1024*1024);
    total_test_time = (end_time - start_time)/CPUHZ;
    total_average_bandwidth = total_copysize/total_test_time;
    total_average_copy_time = total_test_time/total_count;

    TMEMINFO("TOTAL:copy size:%lu MB test time:%lu ms copy counts:%lu\n",
             total_copysize,total_test_time,total_count);
    TMEMINFO("TOTAL:average bandwidth:%dMB/ms average copy time:%dms\n",
              total_average_bandwidth,total_average_copy_time);
    
    return 0;
}

static int test_thread(void *data)
{
    char *buf_read;
    char *buf_write;
    unsigned long int num_rd,start_t,end_t,count = 0;
    unsigned long start, intv=TEST_TIME*HZ,t_copy_time,t_bandwidth;


    buf_read = vmalloc(pa.copysize);
    buf_write = vmalloc(pa.copysize);


    if(!buf_read || !buf_write)
    {
    TMEMERR("failed to allocate memory");
    }

    wait_event_interruptible(wq,flag != 0);
    start = jiffies;
    start_t = rdtsc();

    do{
       memcpy(buf_write,buf_read,pa.copysize);
       count++;
     }while((flag != 0) && time_before(jiffies, start+intv));
     end_t = rdtsc();
     num_rd = end_t - start_t;

     mutex_lock(&count_mutex);
     total_count += count;
     total_nr += num_rd;
     tflag--;
     mutex_unlock(&count_mutex);
     if(tflag == 0)end_time = rdtsc();

     t_copy_time = num_rd/(CPUHZ*count);
     t_bandwidth = pa.copysize*count*CPUHZ/(1024*1024*num_rd);

     TMEMINFO("thread:%i total copy size:%lu number of copy times:%lu\n ",                    current->pid,pa.copysize*count,count);
     TMEMINFO("copy time for once:%lu ms average bandwidth:%lu MB/ms\n",                       t_copy_time,t_bandwidth);
     return 0;
}

static long tmem_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{
    int ret = 0,i;
    char *name;
    total_count = 0;
    total_nr = 0;
    flag = 0;
    

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
           TMEMINFO("thread run time:%ds copy size:%lu threads:%d\n",TEST_TIME,
                     pa.copysize,pa.threads);
           tflag = pa.threads;

           for(i = 0;i != pa.threads;i++)
           {
              sprintf(name,"thread_name:%d",i);
              _tsk = kthread_create(test_thread,NULL,name);
              if (IS_ERR(_tsk)) 
              {
                  ret = PTR_ERR(_tsk);
                  _tsk = NULL;
                  goto out;
              }
               wake_up_process(_tsk);
            }
            _tsk = kthread_create(calcu_thread,NULL,"calu_thread");
            if (IS_ERR(_tsk))
            {
                ret = PTR_ERR(_tsk);
                _tsk = NULL;
                goto out;
            }
            wake_up_process(_tsk);

            schedule_timeout(2*HZ);
            flag = 1;
            wake_up_interruptible(&wq);
            start_time = rdtsc();
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

static ssize_t tmem_read(struct file *file,char *buf,size_t count,
                        loff_t *f_pos)
{
    return copy_to_user(buf,buf_dev,count);
}

static const struct file_operations _tmem_ctl_fops = {
        .open = nonseekable_open,
        .unlocked_ioctl = tmem_ioctl,
        .owner = THIS_MODULE,
        .read = tmem_read,
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
    mutex_init(&count_mutex);
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
    mutex_destroy(&count_mutex);
}


module_init(t_init);
module_exit(t_exit);
