#ifdef __KERNEL__ 


#define TMEMERR(f, arg...) \
        printk(KERN_NOTICE "TEST_MEM : " f "\n", ## arg)
#define TMEMINFO(f,arg...)\
       printk(KERN_INFO "TEST_MEM :" f "\n",## arg)

#endif
