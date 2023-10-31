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

#ifndef __GW_MSSQL_H__
#define __GW_MSSQL_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <gfc/gfc_list.h>

#define GW_MSSQL_OK               0
#define GW_MSSQL_ERROR            1000

int
gw_mssql_connect(const char *host,
                 int port,
                 const char *database,
                 const char *username,
                 const char *password);

int
gw_mssql_query(const char *sql, gfc_list_p* result);

void
gw_mssql_disconnect();

#ifdef __cplusplus
}
#endif

#endif // __GW_MSSQL_H__