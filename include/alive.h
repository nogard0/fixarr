#ifndef ALIVE_H
#define ALIVE_H

#include <time.h>

time_t process_alive();
void set_host_dead(struct _host *host);

#endif /* ALIVE_H */