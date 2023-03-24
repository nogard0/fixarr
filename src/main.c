#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <time.h>

#include "types.h"
#include "conf.h"
#include "stalled.h"
#include "funcs.h"
#include "alive.h"

int main(int argc, char *const *argv)
{
  int opt;
  int dry_run=0;

  setvbuf(stdout,NULL,_IONBF,0);
  setvbuf(stderr,NULL,_IONBF,0);

  printf("fixarr v1.02 stared!\n");
#ifdef DEBUG
  printf("DEBUG mode!\n");
#endif

  struct option long_options[] =
    { { "conf", required_argument, 0, 'c' },
      { "dry-run", required_argument, 0, 'd' },
      { 0, 0, 0, 0 }, };
  while ((opt = getopt_long(argc, argv, "c:d", long_options, NULL)) != -1) {
    switch (opt)
    {
    case 'c':
      if (load_conf(optarg,0))
        exit(-1);
      break;
    case 'd':
      dry_run=1;
      printf("DRY-RUN mode!\n");
      break;
    default:
      exit(-1);
    }
  }

  if (!conf.hosts) {
    char hfj[255];
    snprintf(hfj,255,"%s/fixarr.json",getenv("HOME"));
    if (load_conf(hfj,1)==-1000)
      if (load_conf("/etc/fixarr.json",1)==-1000)
        if (load_conf("fixarr.json",1)) {
          fprintf(stderr,"ERROR: No valid configuration file could be found!\n");
          exit(-1);
        }
  }

  if (!conf.hosts)
    exit(-1);
  conf.dry_run=dry_run;

  while (1) {
    time_t tim,mi;
    mi=MINnot0(process_alive(),process_stalled());
    time(&tim);
    sleep(MAX(30,mi-tim));
  }  

  return 0;
}
