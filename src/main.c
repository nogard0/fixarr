#include <stdio.h>
#include <unistd.h>

#include "types.h"
#include "conf.h"
#include "stalled.h"
#include "funcs.h"

int main(int argc, char const *argv[])
{
  (void)(argc);
  (void)(argv);
  char hfj[255];

  setvbuf(stdout,NULL,_IONBF,0);
  setvbuf(stderr,NULL,_IONBF,0);

  printf("fixarr v1.00 stared!\n");
#ifdef DEBUG
  printf("DEBUG mode!\n");
#endif

  snprintf(hfj,255,"%s/fixarr.json",getenv("HOME"));
  if (load_conf(hfj,1)==-1000)
    if (load_conf("/etc/fixarr.json",1)==-1000)
      if (load_conf("fixarr.json",1)) {
        fprintf(stderr,"ERROR: No valid configuration file could be found!\n");
        return -1;
      }

  if (!conf.hosts)
    return -1;

  while (1) {
    sleep(MAX(30,process_stalled()));
  }  

  return 0;
}
