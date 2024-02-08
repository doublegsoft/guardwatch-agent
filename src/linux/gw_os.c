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
#include <json.h>
#include <rrd.h>

#include "gw_os.h"

extern int _gw_os_collect_running;

void gw_os_error(const char* fun, int err)
{
  
}

int
gw_os_cpu(char** result)
{
  
  return GW_OS_OK;
}

int
gw_os_memory(char** result)
{
  
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
gw_os_collect(void* data)
{
  time_t raw;
  char* res = NULL;

  while(1)
  {
    if (_gw_os_collect_running != 1) break;

    time (&raw);

    /*!
    ** CPU
    */
    gw_os_cpu(&res);
    // gtdb_insert("cpu", raw, os_result);
    if (res != NULL)
      free(res);
    res = NULL;

    /*!
    ** MEMORY
    */
    gw_os_memory(&res);
    // gtdb_insert("memory", raw, os_result);
    if (res != NULL) 
      free(res);
    res = NULL;

    sleep(1);
  }
}