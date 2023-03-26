#ifndef CONF_H
#define CONF_H

#include "types.h"

#define FIXARR_VERSION "1.1.1"

extern struct _conf conf;

int load_conf(char *fn, int silent);

#endif /* CONF_H */