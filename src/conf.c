#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <jansson.h>
#include <string.h>

#include "types.h"

struct _conf conf;

const struct _arr arrs[]={{"RADARR","movieId","MoviesSearch","movieIds"},
                          {"SONARR","episodeId","EpisodeSearch","episodeIds"}};

int load_conf(char *fn)
{
  char *buf;
  json_t * json, * jarr, * j;
  json_error_t err;
  int res=0;
  int i;
  const char * str;
  struct _host *hosts;

  FILE *f = fopen(fn, "rb");

  if (!f) {
    printf("ERROR: Invalid configuration file specified: %s!\n",fn);
    return errno;
  }

  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET);  

  buf = malloc(fsize + 1);
  fread(buf, fsize, 1, f);
  fclose(f);

  buf[fsize] = 0;

  json = json_loads(buf, 0, &err);
  if (!json) {
    printf("Error in configuration parsing: %s\n", err.text);
    return -1;
  }

  #define go_out(s,p...) { printf(s "!\n",## p); res=-2; goto out; }

  jarr=json_object_get(json,"hosts");
  if (json_array_size(jarr)==0) 
    go_out("No hosts configured!");
  hosts=malloc(sizeof(struct _host)*(json_array_size(jarr)+1));
  memset(hosts,0,sizeof(struct _host)*(json_array_size(jarr)+1));
  json_array_foreach(jarr,i,j) {
    str=json_string_value(json_object_get(j,"type"));
    if (!str)
      go_out("Unspecified host type");
    if (strcmp(str,"radarr") == 0)
      hosts[i].arr=&arrs[0];
    else if (strcmp(str,"sonarr") == 0)
      hosts[i].arr=&arrs[1];
    else 
      go_out("Invalid host type: %s",str);
    str=json_string_value(json_object_get(j,"name"));
    if (str) {
      hosts[i].name=strdup(str);
    } else {
      hosts[i].name=strdup(hosts[i].arr->defname);
    }
    str=json_string_value(json_object_get(j,"url"));
    if (!str)
      go_out("Unspecified url");
    hosts[i].URL=strdup(str);
    str=json_string_value(json_object_get(j,"apikey"));
    if (!str)
      go_out("Unspecified apikey");
    hosts[i].APIKEY=strdup(str);
  }

out:
  json_decref(json);
  return res;
}