#ifndef FUNCS_H
#define FUNCS_H

#include <time.h>
#include <ulfius.h>

#define MIN(a,b) ({typeof(a) _a=a; typeof(b) _b=b; (_a)<(_b)?(_a):(_b);})
#define MAX(a,b) ({typeof(a) _a=a; typeof(b) _b=b; (_a)>(_b)?(_a):(_b);})
#define MINnot0(a,b) ({typeof(a) _a=a; typeof(b) _b=b; _a?_b?_a<_b?_a:_b:_a:0;})

char * print_map(const struct _u_map * map);
void print_response(struct _u_response * response);
time_t convert_iso8601(const char *time_string);
char *secs_to_hrtime(time_t sec);

#define log_stalled_err(x,p...) fprintf(stderr,"ERROR (%s): " x "\n", stalled->host->name, ## p);
#define log_stalled_info(x,p...) fprintf(stdout,"INFO (%s): " x "\n", stalled->host->name, ## p);
#define log_host_err(x,p...) fprintf(stderr,"ERROR (%s)[%s]: " x "\n", host->name, host->URL, ## p);
#define log_host_info(x,p...) fprintf(stdout,"INFO (%s): " x "\n", host->name, ## p);

#ifndef ULFIUS_CHECK_VERSION
#define ULFIUS_CHECK_VERSION(major,minor,patch)                        \
  (ULFIUS_VERSION_MAJOR > (major) ||                                    \
   (ULFIUS_VERSION_MAJOR == (major) && ULFIUS_VERSION_MINOR > (minor)) || \
   (ULFIUS_VERSION_MAJOR == (major) && ULFIUS_VERSION_MINOR == (minor) && \
    ULFIUS_VERSION_PATCH >= (patch)))
#endif

#if ! ULFIUS_CHECK_VERSION(2,7,2)
char _cc[1024];

#define cc(s,p...) ({ snprintf(_cc,1024,s,## p); _cc; })
#endif

#endif /* FUNCS_H */