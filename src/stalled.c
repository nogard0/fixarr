#include <jansson.h>
#include <ulfius.h>
#include <string.h>
#include "types.h"
#include "conf.h"
#include "funcs.h"
#include "alive.h"

int startSearch(struct _host *host, struct _dld *dld)
{
  struct _u_response response;
  int res=0;
  struct _u_request req;
  json_t * json_res;
  json_error_t err;
  json_t * json_body, *j;

  if (dld->ids_count==1 && host->arr->search_single_auto) {
    printf("SEARCH auto-started.\n");
    return 0;
  }
  
  if (dld->ids_count>1 && !host->arr->bulk_search_command_name) {
    log_host_err("Impossible situation: Multiple targets unsuported for %s",host->name);
    exit(-1);
  }

  json_body = json_object();
  if (dld->ids_count==1) {
    json_object_set_new(json_body, "name", json_string(host->arr->search_command_name));
    j = json_array();
    json_array_append_new(j,json_integer(dld->mids[0]));
    json_object_set_new(json_body, host->arr->search_keyname, j);
  } else {
    json_object_set_new(json_body, "name", json_string(host->arr->bulk_search_command_name));
    json_array_append_new(json_body,json_integer(dld->mids[0]));
    json_object_set_new(json_body, host->arr->bulk_id1_keyname, json_integer(dld->bulk_id1));
    json_object_set_new(json_body, host->arr->bulk_id2_keyname, json_integer(dld->bulk_id2));
  }

  ulfius_init_request(&req);
  ulfius_set_request_properties(&req,
                                U_OPT_HTTP_VERB, "POST",
#if ULFIUS_CHECK_VERSION(2,7,2)
                                U_OPT_HTTP_URL, host->URL,
                                U_OPT_HTTP_URL_APPEND, "/api/v3/command",
#else
                                U_OPT_HTTP_URL, cc("%s%s",host->URL,"/api/v3/command"),
#endif
                                U_OPT_TIMEOUT, 20,
                                U_OPT_JSON_BODY, json_body,
                                U_OPT_URL_PARAMETER, "apikey", host->APIKEY,
                                U_OPT_NONE);

  ulfius_init_response(&response);
  res = ulfius_send_http_request(&req, &response);
  if (res != U_OK) {
    set_host_dead(host);
    res=-1;
    print_response(&response);
    goto out;
  }

  if (response.status==201) {
    printf("SEARCH started.\n");
    goto out;
  }

  json_res = ulfius_get_json_body_response(&response,&err);
  if (!json_res) {
    log_host_err("Error in JSON parsing: %s", err.text);
  } else {
    log_host_err("%s", json_string_value(json_object_get(json_res,"message")));
    json_decref(json_res);
  }
  
out:
  json_decref(json_body);
  ulfius_clean_response(&response);
  ulfius_clean_request(&req);
  return res;
}

int delete(struct _host *host, struct _dld *dld)
{
  struct _u_response response;
  int res;
  struct _u_request req;
  json_t * json_res;
  json_error_t err;

#if ULFIUS_CHECK_VERSION(2,7,2)
  char smID[20];
  sprintf(smID,"%d",dld->qids[0]);
#endif

  ulfius_init_request(&req);
  ulfius_set_request_properties(&req,
                                U_OPT_HTTP_VERB, "DELETE",
#if ULFIUS_CHECK_VERSION(2,7,2)
                                U_OPT_HTTP_URL, host->URL,
                                U_OPT_HTTP_URL_APPEND, "/api/v3/queue/",
                                U_OPT_HTTP_URL_APPEND, smID,
#else
                                U_OPT_HTTP_URL, cc("%s%s%d",host->URL,"/api/v3/queue/",dld->qids[0]),
#endif
                                U_OPT_TIMEOUT, 20,
                                U_OPT_URL_PARAMETER, "apikey", host->APIKEY,
                                U_OPT_URL_PARAMETER, "removeFromClient", "true",
                                U_OPT_URL_PARAMETER, "blocklist", "true",
                                U_OPT_NONE);

  ulfius_init_response(&response);
  res = ulfius_send_http_request(&req, &response);
  if (res != U_OK) {
    set_host_dead(host);
    print_response(&response);
    goto out;
  }

  if (response.status==200) {
    printf("DELETED ... ");
    goto out;
  }

  json_res = ulfius_get_json_body_response(&response,&err);
  if (!json_res) {
    log_host_err("Error in JSON parsing: %s", err.text);
  } else {
    log_host_err("%s", json_string_value(json_object_get(json_res,"message")));
    json_decref(json_res);
  }
  res=-2;
  
out:
  ulfius_clean_response(&response);
  ulfius_clean_request(&req);
  return res;
}

int get_dld_info(struct _host *host, int mID, const char *downloadId, struct _dld *dld)
{
  struct _u_response response;
  int res;
  struct _u_request req;
  json_t * json_res=NULL, *j, *jr, *j2;
  json_error_t err;
  char smID[20];

  sprintf(smID,"%d",mID);

  ulfius_init_request(&req);
  ulfius_set_request_properties(&req,
                                U_OPT_HTTP_VERB, "GET",
#if ULFIUS_CHECK_VERSION(2,7,2)
                                U_OPT_HTTP_URL, host->URL,
                                U_OPT_HTTP_URL_APPEND, "/api/v3/history",
#else
                                U_OPT_HTTP_URL, cc("%s%s",host->URL,"/api/v3/history"),
#endif
                                U_OPT_TIMEOUT, 20,
                                U_OPT_URL_PARAMETER, "apikey", host->APIKEY,
                                U_OPT_URL_PARAMETER, host->arr->id_keyname, smID,
                                U_OPT_URL_PARAMETER, "eventType", "1",
                                U_OPT_URL_PARAMETER, host->arr->history_include_target_keyname, "true",
                                U_OPT_URL_PARAMETER, "downloadId", downloadId,
                                U_OPT_NONE);
  if (host->arr->bulk_history_include_target_keyname) {
    u_map_put(req.map_url,host->arr->bulk_history_include_target_keyname,"true");
  }

  ulfius_init_response(&response);
  res = ulfius_send_http_request(&req, &response);
  if (res != U_OK) {
    set_host_dead(host);
    print_response(&response);
    res=-3;
    goto out;
  }

  json_res = ulfius_get_json_body_response(&response,&err);
  if (!json_res) {
    log_host_err("Error in JSON parsing: %s", err.text);
    res=-1;
    goto out;
  }

  jr=json_object_get(json_res,"records");

  if (!jr || !json_array_size(jr)) {
    log_host_err("Error in JSON parsing: no records");
    print_response(&response);
    res=-2;
    goto out;
  }

  j = json_array_get(jr, 0);
  dld->added=convert_iso8601(json_string_value(json_object_get(j,"date")));
  dld->sourceTitle=strdup_na(json_string_value(json_object_get(j,"sourceTitle")));
  if (host->arr->bulk_id1_keyname) {
    dld->bulk_id1=json_integer_value(json_object_get(j,host->arr->bulk_id1_keyname));
  }
  j2=json_object_get(j,host->arr->target_keyname);
  if (j2) {
    dld->title=strdup_na(json_string_value(json_object_get(j2,"title")));
    if (!host->arr->bulk_target_keyname) {
      dld->profile=json_integer_value(json_object_get(j2,"qualityProfileId"));
    }
    if (host->arr->bulk_id2_keyname) {
      dld->bulk_id2=json_integer_value(json_object_get(j2,host->arr->bulk_id2_keyname));
    }
  }
  if (host->arr->bulk_target_keyname) {
    j2=json_object_get(j,host->arr->bulk_target_keyname);
    if (j2) {
      dld->profile=json_integer_value(json_object_get(j2,"qualityProfileId"));
    }
  }

out:
  json_decref(json_res);
  ulfius_clean_response(&response);
  ulfius_clean_request(&req);
  return res;
}

void update_dld(struct _host *host)
{
  struct _u_response response;
  int res;
  time_t now;
  json_error_t err;
  struct _u_request req;
  json_t * json_res = NULL;
  json_t *j;
  int i;
  struct _dld *dld;
  
  time (&now);

  for (dld=host->dld; dld; dld=dld->next_dld) {
    free(dld->qids);
    dld->qids=NULL;
    free(dld->mids);
    dld->mids=NULL;
    dld->ids_count=0;
  }

  ulfius_init_request(&req);
  ulfius_set_request_properties(&req,
                                U_OPT_HTTP_VERB, "GET",
#if ULFIUS_CHECK_VERSION(2,7,2)
                                U_OPT_HTTP_URL, host->URL,
                                U_OPT_HTTP_URL_APPEND, "/api/v3/queue/details",
#else
                                U_OPT_HTTP_URL, cc("%s%s",host->URL,"/api/v3/queue/details"),
#endif
                                U_OPT_TIMEOUT, 20,
                                U_OPT_URL_PARAMETER, "apikey", host->APIKEY,
                                U_OPT_NONE);

  ulfius_init_response(&response);
  res = ulfius_send_http_request(&req, &response);
  if (res != U_OK) {
    set_host_dead(host);
    print_response(&response);
    goto out;
  }

  json_res = ulfius_get_json_body_response(&response,&err);
  if (!json_res) {
    log_host_err("Error in JSON parsing: %s", err.text);
    goto out;
  }
  json_array_foreach(json_res,i,j) {
    int qid, mid;
    const char *downloadId;

    if (!(downloadId=json_string_value(json_object_get(j,"downloadId")))) {
      continue;
    }
    if (!(qid=json_integer_value(json_object_get(j,"id")))) {
      continue;
    }
    if (!(mid=json_integer_value(json_object_get(j,host->arr->id_keyname)))) {
      continue;
    }
    for (dld=host->dld; dld; dld=dld->next_dld) {
      if (!strcmp(dld->downloadId,downloadId))
        break;
    }
    if (!dld) {
      int res;
      double sl=json_number_value(json_object_get(j,"sizeleft"));
      if (sl==0) {
        continue;
      }
      dld=malloc(sizeof(struct _dld));
      memset(dld,0,sizeof(struct _dld));
      dld->size=json_number_value(json_object_get(j,"size"));
      dld->sizeleft=sl;
      res=get_dld_info(host, mid, downloadId, dld);
      if (res<0) {
        free(dld);
        if (host->dead)
          goto out;
        continue;
      }
      dld->indexer=strdup_na(json_string_value(json_object_get(j,"indexer")));
      dld->last_activity=dld->added;
      dld->downloadId=strdup(downloadId);
      dld->mids=realloc(dld->mids,(1+dld->ids_count)*sizeof(*dld->mids));
      dld->qids=realloc(dld->qids,(1+dld->ids_count)*sizeof(*dld->qids));
      dld->mids[dld->ids_count]=mid;
      dld->qids[dld->ids_count]=qid;
      dld->ids_count++;
      dld->next_dld=host->dld;
      host->dld=dld;
    } else {
      double new_sizeleft=json_number_value(json_object_get(j,"sizeleft"));
      if (dld->sizeleft!=new_sizeleft) {
        dld->sizeleft = new_sizeleft;
        dld->last_activity = now;
      }
      dld->mids=realloc(dld->mids,(1+dld->ids_count)*sizeof(*dld->mids));
      dld->qids=realloc(dld->qids,(1+dld->ids_count)*sizeof(*dld->qids));
      dld->mids[dld->ids_count]=mid;
      dld->qids[dld->ids_count]=qid;
      dld->ids_count++;
    }
  }

  struct _dld **dld_holder;
  struct _dld *dld_next;
  for (dld_holder=&host->dld, dld=host->dld, dld_next=dld ? dld->next_dld : NULL ; 
       dld; 
       dld_holder=*dld_holder==dld?&dld->next_dld:dld_holder, dld=dld_next, dld_next=dld?dld->next_dld:NULL) {
    if (!dld->ids_count) {
      *dld_holder=dld_next;
      free(dld->downloadId);
      free(dld->indexer);
      free(dld->sourceTitle);
      free(dld->title);
      free(dld);      
    }
  }

out:
  json_decref(json_res);
  ulfius_clean_response(&response);
  ulfius_clean_request(&req);
}

time_t test_stalled(struct _stalled *stalled)
{
  time_t now;
  time_t nc;
  struct _dld *dld;
  const char *rnss[] = { "StartTimeout", "FinishTimeout" };

  time (&now);
  nc=now+MINnot0(stalled->StartTimeout,stalled->FinishTimeout);

  for (dld=stalled->host->dld; dld; dld=dld->next_dld) {
    if (dld->profile) {
      time_t ncd=nc;
      time_t dif=now-dld->added;
      const char *rns=NULL;

      if (stalled->StartTimeout && (dld->size==dld->sizeleft) && (dld->size!=0)) {
        if (dif>stalled->StartTimeout) {
          rns=rnss[0];
        } else {
          ncd=MIN(ncd,stalled->StartTimeout+dld->added);
        }
      }

      if (!rns && stalled->FinishTimeout) {
        if (dif>stalled->FinishTimeout) {
          rns=rnss[1];
        } else {
          ncd=MIN(ncd,stalled->FinishTimeout+dld->added);
        }
      }

      if (!rns) {
        nc=ncd;
        continue;
      }
    
      log_stalled_info_n("Stalled: [%s](%s)[%.2lf%%]: %s - %s - %s ... ",
          rns,
          secs_to_hrtime(dif), 
          (dld->size-dld->sizeleft)/dld->size*100, 
          dld->indexer,
          dld->sourceTitle,
          dld->title );
    
      if (conf.dry_run) {
        printf("DRY-RUN - no action taken\n");
      } else {
        if (!delete(stalled->host, dld)) {
          sleep(2);
          startSearch(stalled->host, dld);
        }
        if (stalled->host->dead)
          return 0;
        time (&now);
      }
    }
  }

  return nc;
}

time_t process_stalled() 
{
  int i;
  time_t tim,res=0;

  for (i=0; conf.hosts[i].name; i++) {
    if (!conf.hosts[i].dead && conf.hosts[i].used_count) {
      update_dld(&conf.hosts[i]);
    }
  }

  time(&tim);
  for (i=0; conf.stalled[i].host; i++) {
    if (conf.stalled[i].enabled && !conf.stalled[i].host->dead) {
      res=MINnot0(res,test_stalled(&conf.stalled[i]));
    }
  }

  return res;
}

