#include <stdio.h>
#include <jansson.h>
#include <string.h>
#include <ulfius.h>
#include <time.h>

#include <types.h>
#include <conf.h>

// struct _host {
//   int      enabled;
//   char   * URL;
//   char   * APIKEY;
//   char   * name;
//   char   * id_keyname;
//   char   * search_command_name;
//   char   * search_key_name;
// };

// struct _host hosts[]={{1,"http://192.168.40.3:7878","92578a090c8f437bb7a9eba399c8034d","RADARR","movieId","MoviesSearch","movieIds"},
//                       {1,"http://192.168.40.3:8989","beca31b2bc2f4430abf6bb3d05bce0a3","SONARR","episodeId","EpisodeSearch","episodeIds"}};

/**
 * decode a u_map into a string
 */
char * print_map(const struct _u_map * map) {
  char * line, * to_return = NULL;
  const char **keys, * value;
  int len, i;
  if (map != NULL) {
    keys = u_map_enum_keys(map);
    for (i=0; keys[i] != NULL; i++) {
      value = u_map_get(map, keys[i]);
      len = snprintf(NULL, 0, "key is %s, value is %s", keys[i], value);
      line = o_malloc((size_t)(len+1));
      snprintf(line, (size_t)(len+1), "key is %s, value is %s", keys[i], value);
      if (to_return != NULL) {
        len = (int)(o_strlen(to_return) + o_strlen(line) + 1);
        to_return = o_realloc(to_return, (size_t)(len+1));
        if (o_strlen(to_return) > 0) {
          strcat(to_return, "\n");
        }
      } else {
        to_return = o_malloc((o_strlen(line) + 1));
        to_return[0] = 0;
      }
      strcat(to_return, line);
      o_free(line);
    }
    return to_return;
  } else {
    return NULL;
  }
}

void print_response(struct _u_response * response) {
  if (response != NULL) {
    char * headers = print_map(response->map_header);
    char response_body[response->binary_body_length + 1];
    o_strncpy(response_body, response->binary_body, response->binary_body_length);
    response_body[response->binary_body_length] = '\0';
    printf("protocol is\n%s\n\n  headers are \n%s\n\n  body is \n%s\n\n",
           response->protocol, headers, response_body);
    o_free(headers);
  }
}

time_t convert_iso8601(const char *time_string)
{
  struct tm actime;
//  time_t t;
  //memset(&ctime, 0, sizeof(struct tm));
  strptime(time_string, "%FT%TZ", &actime);
//  actime.__tm_zone="GMT";
//  actime.__tm_gmtoff=3600;
//  printf ("The current local time is: %s, %ld", asctime (&actime),actime.__tm_gmtoff);
//t=timegm(&actime);
  return timegm(&actime);
  //return t + localtime( &t )->__tm_gmtoff;
}

void startSearch(struct _stalled *stalled, int mID)
{
  struct _u_response response;
  int res;
  struct _u_request req;
  json_t * json_res;
  json_error_t err;
  json_t * json_body, *j;

  if (!mID) {
    printf("Invalid ID: 0\n");
    return;
  }

  json_body = json_object();
  json_object_set_new(json_body, "name", json_string(stalled->host->arr->search_command_name));
  j = json_array();
  json_array_append_new(j,json_integer(mID));
  json_object_set_new(json_body, stalled->host->arr->search_key_name, j);

  ulfius_init_request(&req);
  ulfius_set_request_properties(&req,
                                U_OPT_HTTP_VERB, "POST",
                                U_OPT_HTTP_URL, stalled->host->URL,
                                U_OPT_HTTP_URL_APPEND, "/api/v3/command",
                                U_OPT_TIMEOUT, 20,
                                U_OPT_JSON_BODY, json_body,
                                U_OPT_URL_PARAMETER, "apikey", stalled->host->APIKEY,
                                U_OPT_NONE); // Required to close the parameters list

  ulfius_init_response(&response);
  res = ulfius_send_http_request(&req, &response);
  if (res != U_OK) {
    printf("Error in http request: %d\n", res);
    print_response(&response);
    goto out;
  }

  if (response.status==201) {
    printf("SEARCH started.\n");
    goto out;
  }

  json_res = ulfius_get_json_body_response(&response,&err);
  if (!json_res) {
    printf("Error in JSON parsing: %s\n", err.text);
  } else {
    printf("%s\n", json_string_value(json_object_get(json_res,"message")));
    json_decref(json_res);
  }
  
out:
  json_decref(json_body);
  ulfius_clean_response(&response);
  ulfius_clean_request(&req);
}

int delete(struct _stalled *stalled, int mID)
{
  struct _u_response response;
  int res;
  struct _u_request req;
  char smID[20];
  json_t * json_res;
  json_error_t err;

  if (!mID) {
    printf("Invalid ID: 0\n");
    return -1;
  }

  sprintf(smID,"%d",mID);

  ulfius_init_request(&req);
  ulfius_set_request_properties(&req,
                                U_OPT_HTTP_VERB, "DELETE",
                                U_OPT_HTTP_URL, stalled->host->URL,
                                U_OPT_HTTP_URL_APPEND, "/api/v3/queue/",
                                U_OPT_HTTP_URL_APPEND, smID,
                                U_OPT_TIMEOUT, 20,
                                U_OPT_URL_PARAMETER, "apikey", stalled->host->APIKEY,
                                U_OPT_URL_PARAMETER, "removeFromClient", "true",
                                U_OPT_URL_PARAMETER, "blocklist", "true",
                                U_OPT_NONE); // Required to close the parameters list

  ulfius_init_response(&response);
  res = ulfius_send_http_request(&req, &response);
  if (res != U_OK) {
    printf("Error in http request: %d\n", res);
    print_response(&response);
    goto out;
  }

  if (response.status==200) {
    printf("DELETED ... ");
    goto out;
  }

  json_res = ulfius_get_json_body_response(&response,&err);
  if (!json_res) {
    printf("Error in JSON parsing: %s\n", err.text);
  } else {
    printf("%s\n", json_string_value(json_object_get(json_res,"message")));
    json_decref(json_res);
  }
  res=-2;
  
out:
  ulfius_clean_response(&response);
  ulfius_clean_request(&req);
  return res;
}

int get_mins_added(struct _stalled *stalled, int mID)
{
  struct _u_response response;
  int res;
  struct _u_request req;
  json_t * json_res, *j;
  json_error_t err;
  time_t now;
  time_t added, dif=0;
  char smID[20];

  if (!mID) {
    return 0;
  }

  sprintf(smID,"%d",mID);

  ulfius_init_request(&req);
  ulfius_set_request_properties(&req,
                                U_OPT_HTTP_VERB, "GET",
                                U_OPT_HTTP_URL, stalled->host->URL,
                                U_OPT_HTTP_URL_APPEND, "/api/v3/history",
                                U_OPT_TIMEOUT, 20,
                                U_OPT_URL_PARAMETER, "apikey", stalled->host->APIKEY,
                                U_OPT_URL_PARAMETER, stalled->host->arr->id_keyname, smID,
                                U_OPT_NONE); // Required to close the parameters list

  ulfius_init_response(&response);
  res = ulfius_send_http_request(&req, &response);
  if (res != U_OK) {
    printf("Error in http request: %d\n", res);
    print_response(&response);
    goto out;
  }

  json_res = ulfius_get_json_body_response(&response,&err);
  if (!json_res) {
    printf("Error in JSON parsing: %s\n", err.text);
    res=-1;
    goto out;
  }

  j=json_object_get(json_res,"records");

  if (!j || !json_array_size(j)) {
    printf("Error in JSON parsing: no records\n");
    res=-2;
    json_decref(json_res);
    goto out;
  }

  j=json_array_get(j, 0);

  time (&now);
  added=convert_iso8601(json_string_value(json_object_get(j,"date")));
  dif=(now-added)/60;

  json_decref(json_res);
out:
  ulfius_clean_response(&response);
  ulfius_clean_request(&req);
  return dif;
}

int find_stalled(struct _stalled *stalled)
{
  struct _u_response response;
  int res;
  json_error_t err;
  struct _u_request req;
  json_t * json_res;
  json_t *jarr, *j;
  int i;

  ulfius_init_request(&req);
  ulfius_set_request_properties(&req,
                                U_OPT_HTTP_VERB, "GET",
                                U_OPT_HTTP_URL, stalled->host->URL,
                                U_OPT_HTTP_URL_APPEND, "/api/v3/queue",
                                U_OPT_TIMEOUT, 20,
                                U_OPT_URL_PARAMETER, "apikey", stalled->host->APIKEY,
                                U_OPT_NONE);

  ulfius_init_response(&response);
  res = ulfius_send_http_request(&req, &response);
  if (res != U_OK) {
    printf("Error in http request: %d\n", res);
    print_response(&response);
    goto out;
  }
//print_response(&response);
  json_res = ulfius_get_json_body_response(&response,&err);
  if (!json_res) {
    printf("Error in JSON parsing: %s\n", err.text);
    res=-1;
    goto out;
  }
  jarr=json_object_get(json_res,"records");
  json_array_foreach(jarr,i,j) {
    double so,sl;
    int dif;
    int id;
    so=json_real_value(json_object_get(j,"size"));
    sl=json_real_value(json_object_get(j,"sizeleft"));
    //printf("TEST: %lld,%lld\n",so,sl);
    id=json_integer_value(json_object_get(j,stalled->host->arr->id_keyname));
    if ((so!=sl) || (so==0)) {
      continue;
    }
    dif=get_mins_added(stalled, id);
    if (dif>15) {
      printf("%s stalled: (%d min): %s - %s ... ", stalled->host->name, dif,json_string_value(json_object_get(j,"indexer")),
          json_string_value(json_object_get(j,"title")));
      if (delete(stalled, json_integer_value(json_object_get(j,"id"))) == 0) {
        startSearch(stalled, id);
      }
    }
  }

  json_decref(json_res);

out:
  ulfius_clean_response(&response);
  ulfius_clean_request(&req);
  return res;
}

int main(int argc, char const *argv[])
{
  (void)(argc);
  (void)(argv);

  int i;

  //setvbuf(stdout,NULL,_IONBF,0);
  printf("fixarr v1.00 stared!\n");

  load_conf("fixarr.json");

  while (1) {
    for (i=0; i<2; i++) {
      find_stalled(&conf.stalled[i]);
    }
    sleep(60*5);
  }  

  return 0;
}
