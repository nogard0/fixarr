#include <stdio.h>
#include <string.h>
#include <jansson.h>
#include <ulfius.h>

char hr_secs[20];

#ifdef DEBUG
char * print_map(const struct _u_map * map) {
  char * line, * to_return = NULL;
  const char **keys, * value;
  int len, i;
  if (map != NULL) {
    keys = u_map_enum_keys(map);
    for (i=0; keys[i] != NULL; i++) {
      value = u_map_get(map, keys[i]);
      len = snprintf(NULL, 0, "key is %s, value is %s", keys[i], value);
      line = malloc((size_t)(len+1));
      snprintf(line, (size_t)(len+1), "key is %s, value is %s", keys[i], value);
      if (to_return != NULL) {
        len = (int)(strlen(to_return) + strlen(line) + 1);
        to_return = realloc(to_return, (size_t)(len+1));
        if (strlen(to_return) > 0) {
          strcat(to_return, "\n");
        }
      } else {
        to_return = malloc((strlen(line) + 1));
        to_return[0] = 0;
      }
      strcat(to_return, line);
      free(line);
    }
    return to_return;
  } else {
    return NULL;
  }

}
#endif

void print_response(struct _u_response * response) {
#ifndef DEBUG
  (void)(response);
#else
  if (response != NULL) {
    char * headers = print_map(response->map_header);
    char response_body[response->binary_body_length + 1];
    if (response->binary_body_length)
      strncpy(response_body, response->binary_body, response->binary_body_length);
    response_body[response->binary_body_length] = '\0';
    printf("protocol is\n%s\n\n  headers are \n%s\n\n  body is \n%s\n\n",
           response->protocol, headers, response_body);
    free(headers);
  }
#endif
}

time_t convert_iso8601(const char *time_string)
{
  struct tm actime;
  strptime(time_string, "%FT%TZ", &actime);
  return timegm(&actime);
}

char *secs_to_hrtime(time_t sec)
{
  int h = (sec/3600); 
	int m = (sec -(3600*h))/60;
	int s = (sec -(3600*h)-(m*60));
	
	sprintf(hr_secs, "%02d:%02d:%02d",h,m,s);
  return hr_secs;
}