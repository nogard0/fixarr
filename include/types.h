#ifndef TYPES_H
#define TYPES_H

#include <time.h>

struct _arr {
  char         * defname;
  char         * id_keyname;
  char         * search_command_name;
  char         * search_key_name;
  char         * history_endpoint;
};

struct _host {
  char         * URL;
  char         * APIKEY;
  char         * name;
  const struct _arr  * arr;
  int            dead;
  int            used_count;
  time_t         next_check; 
};

struct _stalled {
  int            enabled;
  struct _host * host;
  unsigned int   zeroStartTimeout;
  unsigned int   stalledTimeout;
  time_t         next_check;  
};

struct _conf {
  struct _host    * hosts;
  struct _stalled * stalled;
  int               dry_run;
  int               hosts_served;
};

#endif /* TYPES_H */