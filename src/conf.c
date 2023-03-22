#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <jansson.h>
#include <string.h>

#include "types.h"

struct _conf conf;

const struct _arr arrs[]={{"RADARR","movieId","MoviesSearch","movieIds","/movie"},
                          {"SONARR","episodeId","EpisodeSearch","episodeIds",""}};

#define json_integer_value_def(j,n,def) ({ int _r; json_t *_j=json_object_get(j,n); if (json_is_integer(_j)) _r=json_integer_value(_j); else _r=def; _r; })

int load_conf(char *fn, int silent)
{
  char *buf;
  json_t * json, * jarr, * j, * jarr2, * j2;
  json_error_t err;
  int res=0;
  int i, l, n, hc;
  const char * str;
  struct _host *hosts=NULL;
  struct _stalled *stalled=NULL;

  conf.hosts=NULL;
  conf.stalled=NULL;
  conf.dry_run=0;

  FILE *f = fopen(fn, "rb");

  if (!f) {
    if (silent)
      return -1000;
    fprintf(stderr,"ERROR: Invalid configuration file specified: %s!\n",fn);
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
    fprintf(stderr,"Error in configuration parsing: %s\n", err.text);
    return -1;
  }

  #define go_out(s,p...) { fprintf(stderr,"%s: "s "!\n",fn,## p); res=-2; goto out; }

  jarr=json_object_get(json,"hosts");
  if (json_array_size(jarr)==0) 
    go_out("No hosts configured!");
  hc = json_array_size(jarr);
  hosts=malloc(sizeof(struct _host)*(hc+1));
  memset(hosts,0,sizeof(struct _host)*(hc+1));
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

  jarr=json_object_get(json,"stalled");
  l = 1;
  json_array_foreach(jarr,i,j) {
    l += json_array_size(json_object_get(j,"hosts"));
  }
  stalled = malloc(sizeof(struct _stalled)*l);
  memset(stalled,0,sizeof(struct _stalled)*l);

  n = 0;
  json_array_foreach(jarr,i,j) {
    jarr2 = json_object_get(j,"hosts");
    json_array_foreach(jarr2,l,j2) {
      int ho;
      const char *host;
      host=json_string_value(j2);
      if (!host)
        go_out("Invalid host specified");
      for (ho=0; ho<hc; ho++) {
        if (!strcmp(hosts[ho].name,host)) {
          stalled[n].host = &hosts[ho];
          break;
        }
      }
      if (!stalled[n].host)
        go_out("Invalid host specified - %s",host);
      stalled[n].enabled = !json_is_false(json_object_get(j,"enabled"));
      stalled[n].zeroStartTimeout = json_integer_value_def(j,"zeroStartTimeout",15)*60;
      stalled[n].stalledTimeout = json_integer_value_def(j,"stalledTimeout",7200)*60;
      if (!stalled[n].zeroStartTimeout && !stalled[n].stalledTimeout)
        stalled[n].enabled = 0;
      n++;
    }
  }

  for (i=0, l=0; stalled[i].host; i++) {
    if (stalled[i].enabled) {
      l++;
      break;
    }
  }

  if (!l) {
    go_out("No valid enabled stalled watchers configured");
  }

  conf.hosts=hosts;
  conf.stalled=stalled;

  printf("Configuration loaded successfully: %s\n", fn);

  json_decref(json);
  return 0;

out:
  json_decref(json);
  if (hosts)
    for (i=0;hosts[i].name!=NULL;i++) {
      free(hosts[i].name);
      free(hosts[i].URL);
      free(hosts[i].APIKEY);
    }
  if (stalled)
    free(stalled);
  free(hosts);
  return res;
}