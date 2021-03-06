#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include "test_mem.h"


struct option_info {
	int threads;
	unsigned long copysize;
        char cmd;
};

 /*struct para{
  *       int threads;
  *       unsigned long copysize;
};*/

static struct option_info mkoptions;
static  struct para pa;

void usage(char *name)
{
	printf
	    ("create : %s -r(run the test) -t threads(the number of theads) -b block_size(the copy size every time) -h(help)\n",name);
	    ("         datadevX can either be a path to a file or a blockdevice. No more then 16 devices are supported.\n");
	printf
	    ("         specify the fastest storage first. E.g. -f /dev/ssd:/dev/sas:/dev/sata.\n");
	printf("Detach : %s -d /dev/tier_device_name\n", name);
	exit(-1);
}


int get_opts(int argc,char *argv[])
{
        int c,ret = 0;
        while((c = getopt(argc, argv, "qrt:s:h")) != -1)
             switch(c){
             case 'r':
                 mkoptions.cmd = 'r';
                 break;
             case 'q':
                 mkoptions.cmd = 'q';
                 break;
             case 't':
                 if (optopt == 't')
		     printf
		      ("Option -%c requires threads as argument.\n",
		       optopt);
	       	 else
			sscanf(optarg, "%d",
			       &mkoptions.threads);
		 break;
             case 's':
                  if (optopt == 's')
			printf
			    ("Option -%c requires copy_size as argument.\n",
			     optopt);
	          else
			sscanf(optarg, "%lu",&mkoptions.copysize);
                        if (optarg[strlen(optarg) - 1] == 'K')
                                        mkoptions.copysize *= 1024;
                        if (optarg[strlen(optarg) - 1] == 'M')
					mkoptions.copysize *= 1024 * 1024;
		        if (optarg[strlen(optarg) - 1] == 'G')
					mkoptions.copysize *=
					    1024 * 1024 * 1024; 
		  break;
              case 'h':
                  usage(argv[0]);
                  break;
              default:
                  abort();
              }
         pa.threads = mkoptions.threads;
         pa.copysize = mkoptions.copysize;
         printf("\n");
         return ret;
}

int main(int argc,char *argv[])
{

    int fd,rc = 0;
    int mode = O_RDWR | O_NOATIME;

    if (argc < 3)
        usage(argv[0]);
        if (0 != get_opts(argc, argv))
                exit(-1);

        if ((fd = open("/dev/tmemcontrol", mode)) < 0) {
                fprintf(stderr,
                        "Failed to open /dev/tmemcontrol, is test_mem.ko loaded?\n");
                exit(-1);
        }

        if (-1 == flock(fd, LOCK_EX)) {
                fprintf(stderr, "Failed to lock /dev/tmemcontrol\n");
                exit(-1);
        }

        switch(mkoptions.cmd)
        {
           case 'r':
                 if(ioctl(fd,TMEM_RUN,&pa) < 0)
                 {
                     rc = 1;
                     fprintf(stderr,"failed to run test mem module\n");
                 } 
                 break;
            case 'q':
                  if(ioctl(fd,TMEM_STOP,0) < 0)
                  {
                     rc = 1;
                     fprintf(stderr,"failed to stop test mem module\n");  
                  }
                  break;
            default:
                  rc = 1;
                  fprintf(stderr,"unknow command\n");
        }

       	flock(fd, LOCK_UN);
	close(fd);
        exit(rc); 
}
