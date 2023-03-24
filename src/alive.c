#include <jansson.h>
#include <ulfius.h>
#include <string.h>
#include "types.h"
#include "conf.h"
#include "funcs.h"

void test_alive(struct _host *host)
{
  struct _u_response response;
  int res;
  struct _u_request req;
  json_error_t err;

  host->next_check=0;

  ulfius_init_request(&req);
  ulfius_set_request_properties(&req,
                                U_OPT_HTTP_VERB, "GET",
#if ULFIUS_CHECK_VERSION(2,7,2)
                                U_OPT_HTTP_URL, host->URL,
                                U_OPT_HTTP_URL_APPEND, "/api/v3/system/status",
#else
                                U_OPT_HTTP_URL, cc("%s%s",host->URL,"/api/v3/system/status"),
#endif
                                U_OPT_TIMEOUT, 5,
                                U_OPT_URL_PARAMETER, "apikey", host->APIKEY,
                                U_OPT_NONE);
  
  ulfius_init_response(&response);
  res = ulfius_send_http_request(&req, &response);
  if (res != U_OK) {
    if (host->dead==-2) {
      log_host_err("Host can not be reached. Will try again later.");
      host->dead--;
      print_response(&response);
    } else {
      if (host->dead>0)
        host->dead++;
      else 
        host->dead--;
    }
    goto out;
  }

  if (response.status!=200) {
    if (response.status==401) {
      log_host_err("Host reports Unauthorized access! Check your APIKEY!");
    } else {
      log_host_err("Host reports Error: %ld!", response.status);
    }
    log_host_err("Host disabled!");
    host->dead=-1;
    print_response(&response);
    goto out;
  }

  if (host->dead<0) {
    const char *ver;
    json_t * json_res = ulfius_get_json_body_response(&response,&err);
    if (!json_res) {
      log_host_err("Host returned invalid JSON response: %s", err.text);
      log_host_err("Host disabled!");
      host->dead=-1;
      print_response(&response);
      goto out;
    }
    ver=json_string_value(json_object_get(json_res,"version"));
    log_host_info("Host connected! Version: %s", ver);
    json_decref(json_res);
  } else {
    log_host_info("Host back online!");
  }
  host->dead=0;

out:

  if (host->dead && !(host->dead==-1)) {
    time_t tim;
    int hd=abs(host->dead);
    time(&tim);
    if (hd<6)
      tim+=60;
    else if (hd<11)
      tim+=60*5;
    else if (hd<21)
      tim+=60*30;
    else 
      tim+=60*60;
    host->next_check=tim-5;
  } else {
    host->next_check=0;
  }
  ulfius_clean_response(&response);
  ulfius_clean_request(&req);
}

void set_host_dead(struct _host *host)
{
  time_t tim;
  log_host_err("Host is unreachable. Will try again later!");
  host->dead=2;
  time(&tim);
  host->next_check=tim+60;
}

time_t process_alive() 
{
  int i;
  time_t tim,res=0;

  for (i=0; conf.hosts[i].name; i++) {
    if (conf.hosts[i].dead && !(conf.hosts[i].dead==-1)) {
      time(&tim);
      if (conf.hosts[i].next_check<=tim) {
        test_alive(&conf.hosts[i]);
      }
      if (conf.hosts[i].dead && !(conf.hosts[i].dead==-1)) {
        res=MINnot0(res,conf.hosts[i].next_check);
      }
    }
  }

  return res;
}