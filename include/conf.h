#ifndef CONF_H
#define CONF_H

#include "types.h"

extern struct _conf conf;

int load_conf(char *fn);

#endif /* CONF_H */