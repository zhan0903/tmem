#ifdef __KERNEL__
#include <linux/bio.h>
#include <linux/blkdev.h>
#include <linux/spinlock.h>
#include <linux/mutex.h>
#include <linux/file.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/vmalloc.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/hdreg.h>
#include <linux/crypto.h>
#include <linux/err.h>
#include <linux/scatterlist.h>
#include <linux/workqueue.h>
#include <linux/rbtree.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <linux/falloc.h>
#include <linux/kthread.h>
#include <linux/version.h>
#include <linux/sysfs.h>
#include <linux/device.h>
#include <linux/socket.h>
#include <linux/in.h>
#include <linux/net.h>
#include <linux/inet.h>
#include <asm/div64.h>
#endif

#define TMEM_RUN        0xFE00
#define TMEM_STOP       0xFE03


struct para{
       int threads;
       unsigned long copysize;
};

#define TMEMERR(f, arg...) \
        printk(KERN_NOTICE "TEST_MEM : " f "\n", ## arg)
#define TMEMINFO(f,arg...)\
       printk(KERN_INFO "TEST_MEM :" f "\n",## arg)

