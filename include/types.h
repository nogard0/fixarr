#ifndef TYPES_H
#define TYPES_H

#include <time.h>

struct _arr {
  char         * defname;
  char         * target_keyname;
  char         * id_keyname;
  int            search_single_auto;
  char         * search_command_name;
  char         * search_keyname;
  char         * history_include_target_keyname;
  char         * bulk_history_include_target_keyname;
  char         * bulk_target_keyname;
  char         * bulk_id1_keyname;
  char         * bulk_id2_keyname;
  char         * bulk_search_command_name;
};

struct _dld {
  char         * downloadId;
  int            bulk_id1;    //series_id
  int            bulk_id2;    //season_num
  int          * mids;        //episode_ids
  int          * qids;        //queue_ids
  int            ids_count;
  char         * indexer;
  char         * sourceTitle;
  char         * title;
  int            profile;
  time_t         added;
  double         size;
  double         sizeleft;
  time_t         last_activity;
  struct _dld  * next_dld;
};

struct _host {
  char         * URL;
  char         * APIKEY;
  char         * name;
  struct _dld  * dld;
  const struct _arr  * arr;
  int            dead;
  int            used_count;
  time_t         next_check;
};

struct _stalled {
  int            enabled;
  struct _host * host;
  unsigned int   StartTimeout;
  unsigned int   FinishTimeout;
  time_t         next_check;  
};

struct _conf {
  struct _host    * hosts;
  struct _stalled * stalled;
  int               dry_run;
  int               hosts_served;
};

#endif /* TYPES_H */