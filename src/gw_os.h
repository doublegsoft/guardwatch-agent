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

#ifndef __GW_OS_H__
#define __GW_OS_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define GW_OS_OK                                0
#define GW_OS_ERROR                             1000
#define GW_OS_ERROR_INSUFFICIENT_BUFFER         1001

#define GW_OS_CPU_RRD_DS                        "DS:cpu:GAUGE:600:0:100"
#define GW_OS_MEMORY_RRD_DS                     "DS:mem:GAUGE:600:U:U"
#define GW_OS_VOLUME_RRD_DS                     "DS:vol:GAUGE:600:U:U"


/*!
** Starts to gather machine performance metrics such as cpu, memory usages.
*/
void*
gw_os_start(void*);

/*!
** Gets the cpu result, and the format is below:
** <ul>
**   <li>name: volume name</li>
**   <li>free: the number of free bytes</li>
**   <li>total: the number of total bytes</li>
** </ul>
**
** @param result
**        the json result
**
** @return the result code
*/
int
gw_os_cpu(char**);

/*!
** Gets the memory result.
**
** @param result
**        the json result
**
** @return the result code
*/
int
gw_os_memory(char**);

/*!
** Gets the process snapshot result.
**
** @param result
**        the json result
**
** @return the result code
*/
int
gw_os_processes(char**);

/*!
** Gets the volumes result.
**
** @param result
**        the json result
**
** @return the result code
*/
int
gw_os_volumes(char**);


#ifdef __cplusplus
}
#endif

#endif // __GW_OS_H__