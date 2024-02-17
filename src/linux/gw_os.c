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
#include <stdio.h>
#include <rrd.h>
#include <json.h>
#include <math.h>
#include <gfc.h>
#include <string.h>

#include "gw_app.h"
#include "gw_os.h"

extern gw_app_settings_t  _gw_app_settings;
extern int                _gw_os_collect_running;

void gw_os_error(const char* fun, int err)
{
  
}

static void 
gw_os_cpu_monitor(void)
{
  char line[4096];
  char value[4096] = {'\0'};
  FILE* fp = fopen("/proc/stat", "r");

  while (fgets(line, sizeof(line), fp))
  {
    gfc_list_p strs = gfc_string_split(line, " ");
    long user = atol(gfc_list_get(strs, 1));
    long nice = atol(gfc_list_get(strs, 2));
    long system = atol(gfc_list_get(strs, 3));
    long idle = atol(gfc_list_get(strs, 4));
    long iowait = atol(gfc_list_get(strs, 5));
    long irq = atol(gfc_list_get(strs, 6));
    long softirq = atol(gfc_list_get(strs, 7));
    long tot = user + nice + system + idle + iowait + irq + softirq;
    double percent = 100 - (double)(idle * 100) / tot;
    sprintf(value, "N:%.2f", percent);
    gfc_list_deep_free(strs);
    break;
  }

  const char* rrdupt_argv[] = {
    "update", _gw_app_settings.cpu_rrd_file,
    value,
    NULL
  };
  int rc = rrd_update(3, (char**)rrdupt_argv);
  if (rc) 
    fprintf(stderr, "error: %s\n", rrd_get_error());
  fclose(fp);
}

static void
gw_os_memory_monitor(void)
{
  char line[4096];
  char value[4096] = {'\0'};
  FILE* fp = fopen("/proc/meminfo", "r");

  char line_total[4096] = {'\0'};
  char line_free[4096] = {'\0'};

  while (fgets(line, sizeof(line), fp))
  {
    if (line_total[0] != '\0' && line_free[0] != '\0') 
      break;
    if (strstr(line, "MemTotal") != NULL)
      strcpy(line_total, line);
    if (strstr(line, "MemFree") != NULL)
      strcpy(line_free, line);
  }
  fclose(fp);

  char* token = strstr(line_total, ":");
  token++;
  int total_mem = atoi(token);

  token = strstr(line_free, ":");
  token++;
  int free_mem = atoi(token);

  double percent = (double)(total_mem - free_mem) * 100 / total_mem;
  sprintf(value, "N:%.2f", percent);
  const char* rrdupt_argv[] = {
    "update", _gw_app_settings.mem_rrd_file,
    value,
    NULL
  };
  int rc = rrd_update(3, (char**)rrdupt_argv);
  if (rc) 
    fprintf(stderr, "error: %s\n", rrd_get_error());
}

int
gw_os_cpu(char** result)
{
  unsigned long     step = 0;
  time_t            start;
  time_t            end;
  unsigned long     ds_cnt = 0;
  char**            ds_names;
  rrd_value_t*      values;
  
  char* rrd_argv[] = {
    "fetch", _gw_app_settings.cpu_rrd_file, "AVERAGE",
    "--resolution", "5s",
    "--start", "now-60",
    "--end", "now"
  };
  
  int rc = rrd_fetch(9, rrd_argv, &start, &end, &step, &ds_cnt, &ds_names, &values);

  if (rc == 0) 
  {
    struct json_object* jobj = json_object_new_object();
    json_object_object_add(jobj, "name", json_object_new_string(ds_names[0]));

    struct json_object* jarr_times = json_object_new_array();
    struct json_object* jarr_values = json_object_new_array();

    int count = (end - start) / step - 1;

    for (int i = 0; i < count; i++) 
    {
      json_object_array_add(jarr_times, json_object_new_int(start + i * step));
      if (!isnan(values[i])) 
        json_object_array_add(jarr_values, json_object_new_double(values[i]));
      else 
        json_object_array_add(jarr_values, json_object_new_double(-1));  
    }

    json_object_object_add(jobj, "times", jarr_times);
    json_object_object_add(jobj, "values", jarr_values);

    const char* jstr = json_object_to_json_string(jobj);
    int len = strlen(jstr);
    *result = malloc(sizeof(char) * (len + 1));
    strcpy(*result, jstr);
    (*result)[len] = '\0';

    json_object_put(jobj);
  }
  
  return GW_OS_OK;
}

int
gw_os_memory(char** result)
{
  unsigned long     step = 0;
  time_t            start;
  time_t            end;
  unsigned long     ds_cnt = 0;
  char**            ds_names;
  rrd_value_t*      values;
  
  char* rrd_argv[] = {
    "fetch", _gw_app_settings.mem_rrd_file, "AVERAGE",
    "--resolution", "5s",
    "--start", "now-60",
    "--end", "now"
  };
  
  int rc = rrd_fetch(9, rrd_argv, &start, &end, &step, &ds_cnt, &ds_names, &values);

  if (rc == 0) 
  {
    struct json_object* jobj = json_object_new_object();
    json_object_object_add(jobj, "name", json_object_new_string(ds_names[0]));

    struct json_object* jarr_times = json_object_new_array();
    struct json_object* jarr_values = json_object_new_array();

    int count = (end - start) / step - 1;

    for (int i = 0; i < count; i++) 
    {
      json_object_array_add(jarr_times, json_object_new_int(start + i * step));
      if (!isnan(values[i])) 
        json_object_array_add(jarr_values, json_object_new_double(values[i]));
      else 
        json_object_array_add(jarr_values, json_object_new_double(-1));  
    }

    json_object_object_add(jobj, "times", jarr_times);
    json_object_object_add(jobj, "values", jarr_values);

    const char* jstr = json_object_to_json_string(jobj);
    int len = strlen(jstr);
    *result = malloc(sizeof(char) * (len + 1));
    strcpy(*result, jstr);
    (*result)[len] = '\0';

    json_object_put(jobj);
  }
  return GW_OS_OK;
}

int
gw_os_volumes(char** result)
{
  return GW_OS_OK;
}

int
gw_os_processes(char** result)
{

  return GW_OS_OK;
}

/*!
** Collects os performance metrics.
**
** @return always {@code NULL}
*/
void*
gw_os_start(void* data)
{
  while(1)
  {
    if (_gw_os_collect_running != 1) break;
    gw_os_cpu_monitor();
    gw_os_memory_monitor();
    sleep(5);
  }
}