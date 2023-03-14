#ifndef TYPES_H
#define TYPES_H

struct _arr {
  char         * defname;
  char         * id_keyname;
  char         * search_command_name;
  char         * search_key_name;
};

struct _host {
  char         * URL;
  char         * APIKEY;
  char         * name;
  const struct _arr  * arr;
};

struct _stalled {
  int            enabled;
  struct _host * host;
  unsigned int   minRefreshTime;
  unsigned int   zeroStartTimeout;
  unsigned int   stalledTimeout;
};

struct _conf {
  struct _host * hosts;
  struct _stalled * stalled;
};

#endif /* TYPES_H */