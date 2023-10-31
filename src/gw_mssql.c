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
#include "config.h"

#include <freetds/tds.h>
#include <freetds/iconv.h>
#include <freetds/utils/string.h>
#include <freetds/convert.h>
#include <freetds/data.h>
#include <freetds/utils.h>

#include <locale.h>
#include <assert.h>

#include "gw_mssql.h"

static TDSSOCKET*       tds;
static TDSLOGIN*        login;
static TDSCONTEXT*      context;
static TDSLOGIN*        connection;

int
gw_mssql_connect(const char* host,
                 int port,
                 const char* database,
                 const char* username,
                 const char* password)
{
  TDSRET rc;
  setlocale(LC_ALL, "");

  login = tds_alloc_login(1);
  context = tds_alloc_context(NULL);

  tds_set_library(login, "TDS-Library");
  tds_set_app(login, "TSQL");
  // tds_set_language(login, "us_english");
  tds_set_server(login, host);
  tds_set_port(login, port);
  tds_set_user(login, username);
  tds_set_passwd(login, password);

  tds = tds_alloc_socket(context, 512);
  assert(tds);
  tds_set_parent(tds, NULL);
  connection = tds_read_config_info(tds, login, context->locale);

  tds_dstr_copy(&connection->database, database);

  rc = tds_connect_and_login(tds, connection);
  if (TDS_FAILED(rc)) return -1;

  return 0;
}

int
gw_mssql_query(const char* sql, gfc_list_p* result)
{
  *result = gfc_list_new();
  int rows = 0;
  TDSRET rc;

  rc = tds_submit_query(tds, sql);
  if (TDS_FAILED(rc))
    return -1;

  return 0;
}

void
gw_mssql_disconnect()
{
  free(login);
  free(context);
  free(connection);
  free(tds);

  tds = NULL;
  login = NULL;
  context = NULL;
}