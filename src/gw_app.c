/*
**                                  _________                   _____        ______
** _______ _____  ________ _______________  /___      ________ ___  /___________  /_
** __  __ `/_  / / /_  __ `/__  ___/_  __  / __ | /| / /_  __ `/_  __/_  ___/__  __ \
** _  /_/ / / /_/ / / /_/ / _  /    / /_/ /  __ |/ |/ / / /_/ / / /_  / /__  _  / / /
** _\__, /  \__,_/  \__,_/  /_/     \__,_/   ____/|__/  \__,_/  \__/  \___/  /_/ /_/
** /____/
**
** Copyright (C) 2020 doublegsoft.net
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <pthread.h>
#include <rrd.h>

#include "gw_app.h"
#include "gw_sqlite.h"
#include "gw_snmpd.h"
#include "gw_httpd.h"
#include "gw_sql.h"
#include "gw_os.h"

/*!
** global settings for application.
*/
gw_app_settings_t _gw_app_settings;

/*!
** defined in gw_snmp.c
*/
extern int _gw_snmp_daemon_running;
extern int _gw_snmp_trap_running;
extern int _gw_http_web_running;
extern int _gw_os_collect_running;

static pthread_t _gw_trap_thread;
static pthread_t _gw_daemon_thread;
static pthread_t _gw_http_thread;
static pthread_t _gw_os_thread;

extern void* gw_os_collect(void*);

void
gw_app_init(const char* work_directory)
{
  char logdir[1024] = {'\0'};
  char rrddir[1024] = {'\0'};

  strcpy(logdir, work_directory);
  strcat(logdir, "/log");
  
  strcpy(rrddir, work_directory);
  strcat(rrddir, "/rrd");
  gfc_fs_mkdirs(rrddir);

  strcpy(_gw_app_settings.cpu_rrd_file, rrddir);
  strcat(_gw_app_settings.cpu_rrd_file, "/cpu.rrd");

  strcpy(_gw_app_settings.mem_rrd_file, rrddir);
  strcat(_gw_app_settings.mem_rrd_file, "/memory.rrd");

  strcpy(_gw_app_settings.vol_rrd_file, rrddir);
  strcat(_gw_app_settings.vol_rrd_file, "/volume.rrd");

  char* cpu_rrd_argv[] = {
    "create", _gw_app_settings.cpu_rrd_file, 
    "-s", "5",
    GW_OS_CPU_RRD_DS, 
    "RRA:AVERAGE:0.5:1:600", NULL
  };
  int rc = rrd_create(6, (char**)cpu_rrd_argv);
  if (rc) 
  {
    fprintf(stderr, "error: %s\n", rrd_get_error());
  }

  char* mem_rrd_argv[] = {
    "create", _gw_app_settings.mem_rrd_file, 
    "-s", "5",
    GW_OS_MEMORY_RRD_DS, 
    "RRA:AVERAGE:0.5:1:600", NULL
  };
  rc = rrd_create(6, (char**)mem_rrd_argv);
  if (rc) 
  {
    fprintf(stderr, "error: %s\n", rrd_get_error());
  }

  char* vol_rrd_argv[] = {
    "create", _gw_app_settings.vol_rrd_file, 
    "-s", "5",
    GW_OS_VOLUME_RRD_DS, 
    "RRA:AVERAGE:0.5:1:600", NULL
  };
  rc = rrd_create(6, (char**)vol_rrd_argv);
  if (rc) 
  {
    fprintf(stderr, "error: %s\n", rrd_get_error());
  }

  gfc_gc_init();
  gfc_log_init(logdir, "guardwatch-agent");
}

void
gw_app_start(const char* work_directory, const char* resources_path)
{
  int           i;
  char*         key = NULL;
  char*         val = NULL;
  char*         sql = "select * from tn_gw_stg";

  char*         volume_result;
  char*         memory_result;
  char*         cpu_result;

  /*!
  ** local database initialization.
  */
  gw_sqlite_open(work_directory);

  /*!
  ** gtdb time series database
  */
  /*
  char dbpath[1024] = {'\0'};
  strcpy(dbpath, work_directory);
  strcat(dbpath, "./guardwatch.gtdb");
  gtdb_open(dbpath);
  */

  gfc_list_p    result;

  /*!
  ** NOT WORKING ON WIN7
  */
  extern const char* GW_SQL_SETTING_FIND;
  gw_sql_get(GW_SQL_SETTING_FIND, "{}", &sql);

  // web root
  strcpy(_gw_app_settings.server_web_root, resources_path);
  strcat(_gw_app_settings.server_web_root, "/www");

  gw_sqlite_query(sql, &result);
  for (i = 0; i < gfc_list_size(result); i++)
  {
    gfc_map_p row = gfc_list_get(result, i);
    gfc_map_get(row, "stgcd", (user_data*)&key);

    if (strcmp(key, KEY_AGENT_ACKNOWLEDGEMENT_ID) == 0)
    {
      gfc_map_get(row, "val", (user_data*)&val);
      strcpy(_gw_app_settings.acknowledgement_id, val);
    }

    if (strcmp(key, KEY_SERVER_ADDRESS) == 0)
    {
      gfc_map_get(row, "val", (user_data*)&val);
      strcpy(_gw_app_settings.server_address, val);
    }

    if (strcmp(key, KEY_SERVER_SNMP_PORT) == 0)
    {
      gfc_map_get(row, "val", (user_data*)&val);
      strcpy(_gw_app_settings.server_snmp_port, val);
    }

    if (strcmp(key, KEY_SERVER_WEB_PORT) == 0)
    {
      gfc_map_get(row, "val", (user_data*)&val);
      strcpy(_gw_app_settings.server_web_port, val);
    }

    if (strcmp(key, KEY_AGENT_SNMP_PORT) == 0)
    {
      gfc_map_get(row, "val", (user_data*)&val);
      strcpy(_gw_app_settings.agent_snmp_port, val);
    }

    if (strcmp(key, KEY_AGENT_WEB_PORT) == 0)
    {
      gfc_map_get(row, "val", (user_data*)&val);
      strcpy(_gw_app_settings.agent_web_port, val);
    }

    if (strcmp(key, KEY_AGENT_NAME) == 0)
    {
      gfc_map_get(row, "val", (user_data*)&val);
      strcpy(_gw_app_settings.agent_name, val);
    }
    gfc_map_free(row);
  }
  // free(sql);
  gfc_list_free(result);

  /*!
  ** snmp configuration.
  */
  gw_snmpd_register();

  /*!
  ** start heartbeat trap thread.
  */
  pthread_create(&_gw_trap_thread, NULL, gw_snmpd_trap, NULL);

  /*!
  ** start snmp daemon thread.
  **
  ** snmp daemon, if equals to 2, skipping agent initialization.
  */
  if (_gw_snmp_daemon_running == 0)
    _gw_snmp_daemon_running = 2;
  pthread_create(&_gw_daemon_thread, NULL, gw_snmpd_serve, (void*)work_directory);

  /*!
  ** start os performance thread
  */
  pthread_create(&_gw_os_thread, NULL, gw_os_start, NULL);

  /*!
  ** start web server thread.
  */
  pthread_create(&_gw_http_thread, NULL, gw_httpd_serve, NULL);
}

void gw_app_stop(void)
{
  _gw_snmp_daemon_running = 0;
  _gw_snmp_trap_running   = 0;
  _gw_http_web_running = 0;
  _gw_os_collect_running = 0;
}