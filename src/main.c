#include <stdio.h>
#include <jansson.h>
#include <string.h>
#include <ulfius.h>
#include <time.h>

#include "types.h"
#include "conf.h"
#include "stalled.h"
#include "funcs.h"

int main(int argc, char const *argv[])
{
  (void)(argc);
  (void)(argv);

  printf("fixarr v1.00 stared!\n");

  load_conf("fixarr.json",0);

  while (1) {
    sleep(MAX(30,process_stalled()));
  }  

  return 0;
}
