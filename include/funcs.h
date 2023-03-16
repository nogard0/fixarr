#ifndef FUNCS_H
#define FUNCS_H

#include <time.h>
#include <ulfius.h>

#define MIN(a,b) ({typeof(a) _a=a; typeof(b) _b=b; (_a)<(_b)?(_a):(_b);})
#define MAX(a,b) ({typeof(a) _a=a; typeof(b) _b=b; (_a)>(_b)?(_a):(_b);})

char * print_map(const struct _u_map * map);
void print_response(struct _u_response * response);
time_t convert_iso8601(const char *time_string);
char *secs_to_hrtime(time_t sec);

#endif /* FUNCS_H */