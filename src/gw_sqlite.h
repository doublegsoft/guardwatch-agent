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

#ifndef __GW_SQLITE_H__
#define __GW_SQLITE_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <gfc.h>

#define GW_SQLITE_OK                    0
#define GW_SQLITE_ERROR                 1

/*!
** Opens a sqlite3 database and if not exists, creating one.
*/
int
gw_sqlite_open(const char* work_directory);

/*!
** Executes one-step sql.
**
** @param sql
**        the sql statement
**
** @return the sqlite result code
*/
int
gw_sqlite_execute(const char* sql);

/*!
** Executes query and fill in the result.
**
** @param sql
**        the sql statement
**
** @param result
**        the fetching result
 *
** @return the sqlite result code
*/
int
gw_sqlite_query(const char* sql, gfc_list_p* result);

/*!
** Closes sqlite3 database.
*/
void
gw_sqlite_close(void);

#ifdef __cplusplus
}
#endif

#endif // __GW_SQLITE_H__