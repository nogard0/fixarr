#include <jansson.h>
#include <ulfius.h>
#include <string.h>
#include "types.h"
#include "conf.h"
#include "funcs.h"

#define log_err(x,p...) fprintf(stderr,"ERROR (%s): " x "\n", stalled->host->name, ## p);

char _cc[1024];

#if ! ULFIUS_CHECK_VERSION(2,7,2)
#define cc(s,p...) ({ snprintf(_cc,1024,s,## p); _cc; })
#endif

void startSearch(struct _stalled *stalled, int mID)
{
  struct _u_response response;
  int res;
  struct _u_request req;
  json_t * json_res;
  json_error_t err;
  json_t * json_body, *j;

  if (!mID) {
    log_err("Invalid ID: 0");
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
#if ULFIUS_CHECK_VERSION(2,7,2)
                                U_OPT_HTTP_URL, stalled->host->URL,
                                U_OPT_HTTP_URL_APPEND, "/api/v3/command",
#else
                                U_OPT_HTTP_URL, cc("%s%s",stalled->host->URL,"/api/v3/command"),
#endif
                                U_OPT_TIMEOUT, 20,
                                U_OPT_JSON_BODY, json_body,
                                U_OPT_URL_PARAMETER, "apikey", stalled->host->APIKEY,
                                U_OPT_NONE);

  ulfius_init_response(&response);
  res = ulfius_send_http_request(&req, &response);
  if (res != U_OK) {
    log_err("Error in http request: %d", res);
    print_response(&response);
    goto out;
  }

  if (response.status==201) {
    printf("SEARCH started.\n");
    goto out;
  }

  json_res = ulfius_get_json_body_response(&response,&err);
  if (!json_res) {
    log_err("Error in JSON parsing: %s", err.text);
  } else {
    log_err("%s", json_string_value(json_object_get(json_res,"message")));
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
  json_t * json_res;
  json_error_t err;

  if (!mID) {
    log_err("Invalid ID: 0");
    return -1;
  }

#if ULFIUS_CHECK_VERSION(2,7,2)
  char smID[20];
  sprintf(smID,"%d",mID);
#endif

  ulfius_init_request(&req);
  ulfius_set_request_properties(&req,
                                U_OPT_HTTP_VERB, "DELETE",
#if ULFIUS_CHECK_VERSION(2,7,2)
                                U_OPT_HTTP_URL, stalled->host->URL,
                                U_OPT_HTTP_URL_APPEND, "/api/v3/queue/",
                                U_OPT_HTTP_URL_APPEND, smID,
#else
                                U_OPT_HTTP_URL, cc("%s%s%d",stalled->host->URL,"/api/v3/queue/",mID),
#endif
                                U_OPT_TIMEOUT, 20,
                                U_OPT_URL_PARAMETER, "apikey", stalled->host->APIKEY,
                                U_OPT_URL_PARAMETER, "removeFromClient", "true",
                                U_OPT_URL_PARAMETER, "blocklist", "true",
                                U_OPT_NONE);

  ulfius_init_response(&response);
  res = ulfius_send_http_request(&req, &response);
  if (res != U_OK) {
    log_err("Error in http request: %d", res);
    print_response(&response);
    goto out;
  }

  if (response.status==200) {
    printf("DELETED ... ");
    goto out;
  }

  json_res = ulfius_get_json_body_response(&response,&err);
  if (!json_res) {
    log_err("Error in JSON parsing: %s", err.text);
  } else {
    log_err("%s", json_string_value(json_object_get(json_res,"message")));
    json_decref(json_res);
  }
  res=-2;
  
out:
  ulfius_clean_response(&response);
  ulfius_clean_request(&req);
  return res;
}

int get_time_and_title(struct _stalled *stalled, int mID, const char *downloadId, time_t *dif, char **sourceTitle)
{
  struct _u_response response;
  int res;
  struct _u_request req;
  json_t * json_res, *j, *jr;
  json_error_t err;
  time_t now;
  time_t added;
  char smID[20];
  int i;

  if (!mID || !downloadId) {
    return -4;
  }

  sprintf(smID,"%d",mID);

  ulfius_init_request(&req);
  ulfius_set_request_properties(&req,
                                U_OPT_HTTP_VERB, "GET",
#if ULFIUS_CHECK_VERSION(2,7,2)
                                U_OPT_HTTP_URL, stalled->host->URL,
                                U_OPT_HTTP_URL_APPEND, "/api/v3/history",
                                U_OPT_HTTP_URL_APPEND, stalled->host->arr->history_endpoint,
#else
                                U_OPT_HTTP_URL, cc("%s%s%s",stalled->host->URL,"/api/v3/history",stalled->host->arr->history_endpoint),
#endif
                                U_OPT_TIMEOUT, 20,
                                U_OPT_URL_PARAMETER, "apikey", stalled->host->APIKEY,
                                U_OPT_URL_PARAMETER, stalled->host->arr->id_keyname, smID,
                                U_OPT_NONE);

  ulfius_init_response(&response);
  res = ulfius_send_http_request(&req, &response);
  if (res != U_OK) {
    log_err("Error in http request: %d", res);
    print_response(&response);
    res=-3;
    goto out;
  }

  json_res = ulfius_get_json_body_response(&response,&err);
  if (!json_res) {
    log_err("Error in JSON parsing: %s", err.text);
    res=-1;
    goto out;
  }

  if (strlen(stalled->host->arr->history_endpoint) == 0) {
    jr=json_object_get(json_res,"records");

    if (!jr || !json_array_size(jr)) {
      log_err("Error in JSON parsing: no records");
      print_response(&response);
      res=-2;
      goto out2;
    }
  } else {
    jr=json_res;
  }

  json_array_foreach(jr,i,j) {
    const char *dId=json_string_value(json_object_get(j,"downloadId"));
    if (dId && !strcmp(dId,downloadId)) {
      time (&now);
      added=convert_iso8601(json_string_value(json_object_get(j,"date")));
      *dif=(now-added);
      dId=json_string_value(json_object_get(j,"sourceTitle"));
      if (dId)
        *sourceTitle=strdup(dId);
      else
        *sourceTitle=strdup("N/A");
      goto out2;
    }
  }
  res=-5;
out2:
  json_decref(json_res);
out:
  ulfius_clean_response(&response);
  ulfius_clean_request(&req);
  return res;
}

void find_stalled(struct _stalled *stalled)
{
  struct _u_response response;
  int res;
  time_t now;
  json_error_t err;
  struct _u_request req;
  json_t * json_res;
  json_t *jarr, *j;
  int i;
  const char *downloadId;

  time (&now);
  if (stalled->zeroStartTimeout && stalled->stalledTimeout) {
    stalled->next_check=now+MIN(stalled->zeroStartTimeout,stalled->stalledTimeout);
  } else if (stalled->zeroStartTimeout) {
    stalled->next_check=now+stalled->zeroStartTimeout;
  } else if (stalled->stalledTimeout) {
    stalled->next_check=now+stalled->stalledTimeout;
  }
  
  ulfius_init_request(&req);
  ulfius_set_request_properties(&req,
                                U_OPT_HTTP_VERB, "GET",
#if ULFIUS_CHECK_VERSION(2,7,2)
                                U_OPT_HTTP_URL, stalled->host->URL,
                                U_OPT_HTTP_URL_APPEND, "/api/v3/queue",
#else
                                U_OPT_HTTP_URL, cc("%s%s",stalled->host->URL,"/api/v3/queue"),
#endif
                                U_OPT_TIMEOUT, 20,
                                U_OPT_URL_PARAMETER, "apikey", stalled->host->APIKEY,
                                U_OPT_URL_PARAMETER, "pageSize", "1000",
                                U_OPT_NONE);

  ulfius_init_response(&response);
  res = ulfius_send_http_request(&req, &response);
  if (res != U_OK) {
    log_err("Error in http request: %d", res);
    print_response(&response);
    goto out;
  }

  json_res = ulfius_get_json_body_response(&response,&err);
  if (!json_res) {
    log_err("Error in JSON parsing: %s", err.text);
    goto out;
  }
  jarr=json_object_get(json_res,"records");
  json_array_foreach(jarr,i,j) {
    double so,sl,pro;
    time_t dif;
    int id, res;
    char *sourceTitle;
    so=json_number_value(json_object_get(j,"size"));
    sl=json_number_value(json_object_get(j,"sizeleft"));
    pro=(so-sl)/so*100;
    id=json_integer_value(json_object_get(j,stalled->host->arr->id_keyname));
    downloadId=json_string_value(json_object_get(j,"downloadId"));
    res=get_time_and_title(stalled, id, downloadId, &dif, &sourceTitle);
    if (res<0) {
      continue;
    }
    if (! ((stalled->zeroStartTimeout && (so==sl) && (so!=0) && (dif>stalled->zeroStartTimeout))
        || (stalled->stalledTimeout && (dif>stalled->stalledTimeout))) ) {
      
      time (&now);
      if (stalled->zeroStartTimeout && (so==sl) && (so!=0)) {
        stalled->next_check=MIN(stalled->next_check,now+stalled->zeroStartTimeout-dif);
      }
      if (stalled->stalledTimeout) {
        stalled->next_check=MIN(stalled->next_check,now+stalled->stalledTimeout-dif);
      }
      if (sourceTitle)
        free(sourceTitle);
      continue;
    }
    
    printf("%s stalled: (%s)[%.2lf%%]: %s - %s - %s ... ", 
          stalled->host->name, 
          secs_to_hrtime(dif), 
          pro, 
          json_string_value(json_object_get(j,"indexer")),
          sourceTitle,
          json_string_value(json_object_get(j,"title")) );
    
    if (conf.dry_run) {
      printf("DRY-RUN - no action taken\n");
    } else {
      if (delete(stalled, json_integer_value(json_object_get(j,"id"))) == 0) {
        startSearch(stalled, id);
      }
    }
    if (sourceTitle)
      free(sourceTitle);
  }

  json_decref(json_res);

out:
  ulfius_clean_response(&response);
  ulfius_clean_request(&req);
}

time_t process_stalled() 
{
  int i;
  time_t tim,res=0;

  time(&tim);
  for (i=0; conf.stalled[i].host; i++) {
    if (conf.stalled[i].enabled) {
      if  (conf.stalled[i].next_check<=tim) {
        find_stalled(&conf.stalled[i]);
      }
      if (res) {
        res=MIN(res,conf.stalled[i].next_check);
      } else {
        res=conf.stalled[i].next_check;
      }
    }
  }

  time(&tim);
  return res-tim;
}

