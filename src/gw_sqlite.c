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

#include <string.h>
#include <sqlite3.h>

#include <gfc.h>

#include "gw_sqlite.h"
#include "gw_app.h"

/*!
** the create-table statement.
*/
const char* GW_SQL_SETTING_CREATE =
    "create table tn_gw_stg (\n"
    "  stgcd           text not null primary key,\n"
    "  stgnm           text,\n"
    "  stgtyp          text,\n"
    "  val             text,\n"
    "  opt             text,\n"
    "  nt              text,\n"
    "  sta             text,\n"
    "  lmt             text\n"
    ");";

/*!
** the insert sql to create a setting entity object.
*/
const char* GW_SQL_SETTING_INSERT =
    "insert into tn_gw_stg ("
    "  stgcd, stgnm, stgtyp, val, opt, nt, sta, lmt"
    ") values ("
    "  {{{stgcd}}}, {{{stgnm}}}, {{{stgtyp}}}, {{{val}}}, {{{opt}}}, {{{nt}}}, {{{sta}}}, {{{lmt}}} "
    ")";

/*!
** the update sql to update a setting entity object.
*/
const char* GW_SQL_SETTING_UPDATE =
    "update tn_gw_stg "
    "set    stgcd = {{{stgcd}}}, stgnm = {{{stgnm}}}, stgtyp = {{{stgtyp}}}, val = {{{val}}}, opt = {{{opt}}}, nt = {{{nt}}}, sta = {{{sta}}}, lmt = {{{lmt}}} "
    "where  stgcd = {{{stgcd}}}";

/*!
** the delete sql to delete a setting entity object.
*/
const char* GW_SQL_SETTING_DELETE =
    "delete from tn_gw_stg \n"
    "where  stgcd = {{{stgcd}}}\n";

/*!
** the select sql to find setting entity objects.
*/
const char* GW_SQL_SETTING_FIND =
    "select stgcd, stgnm, stgtyp, val, opt, nt, sta, lmt "
    "from   tn_gw_stg "
    "where  1 = 1 "
    "{{#stgcd}}"
    "and    stgcd = {{{stgcd}}} "
    "{{/stgcd}}";

const char* GW_SQL_SETTING_INSERT_ACKNOWLEDGEMENT_ID =
    "insert into tn_gw_stg (stgcd, stgnm, stgtyp, sta, lmt, val)\n"
    "values ('" KEY_AGENT_ACKNOWLEDGEMENT_ID "', '服务器识别编码', 'AGENT', 'E', CURRENT_TIMESTAMP, '')";

const char* GW_SQL_SETTING_INSERT_SERVER_ADDRESS =
    "insert into tn_gw_stg (stgcd, stgnm, stgtyp, sta, lmt, val)\n"
    "values ('" KEY_SERVER_ADDRESS "', '网管中心服务地址', 'AGENT', 'E', CURRENT_TIMESTAMP, '192.168.0.207')";

const char* GW_SQL_SETTING_INSERT_SERVER_SNMP_PORT =
    "insert into tn_gw_stg (stgcd, stgnm, stgtyp, sta, lmt, val)\n"
    "values ('" KEY_SERVER_SNMP_PORT "', '网管中心网管端口', 'AGENT', 'E', CURRENT_TIMESTAMP, '6162')";

const char* GW_SQL_SETTING_INSERT_SERVER_WEB_PORT =
    "insert into tn_gw_stg (stgcd, stgnm, stgtyp, sta, lmt, val)\n"
    "values ('" KEY_SERVER_WEB_PORT "', '网管中心应用端口', 'AGENT', 'E', CURRENT_TIMESTAMP, '8088')";

const char* GW_SQL_SETTING_INSERT_AGENT_SNMP_PORT =
    "insert into tn_gw_stg (stgcd, stgnm, stgtyp, sta, lmt, val)\n"
    "values ('" KEY_AGENT_SNMP_PORT "', '本地网管端口', 'AGENT', 'E', CURRENT_TIMESTAMP, '6161')";

const char* GW_SQL_SETTING_INSERT_AGENT_WEB_PORT =
    "insert into tn_gw_stg (stgcd, stgnm, stgtyp, sta, lmt, val)\n"
    "values ('" KEY_AGENT_WEB_PORT "', '本地应用端口', 'AGENT', 'E', CURRENT_TIMESTAMP, '10086')";

const char* GW_SQL_SETTING_INSERT_AGENT_NAME =
    "insert into tn_gw_stg (stgcd, stgnm, stgtyp, sta, lmt, val)\n"
    "values ('" KEY_AGENT_NAME "', '本地机器名称', 'AGENT', 'E', CURRENT_TIMESTAMP, '机器名称')";

const char* GW_SQL_SETTING_INSERT_AGENT_MSSQL_ADDRESS =
    "insert into tn_gw_stg (stgcd, stgnm, stgtyp, sta, lmt, val)\n"
    "values ('AGENT.MSSQL.ADDRESS', 'MSSQL数据库地址', 'AGENT', 'E', CURRENT_TIMESTAMP, 'localhost')";

const char* GW_SQL_SETTING_INSERT_AGENT_MSSQL_PORT =
    "insert into tn_gw_stg (stgcd, stgnm, stgtyp, sta, lmt, val)\n"
    "values ('AGENT.MSSQL.PORT', 'MSSQL数据库端口', 'AGENT', 'E', CURRENT_TIMESTAMP, '1433')";

const char* GW_SQL_SETTING_INSERT_AGENT_MSSQL_USERNAME =
    "insert into tn_gw_stg (stgcd, stgnm, stgtyp, sta, lmt, val)\n"
    "values ('AGENT.MSSQL.USERNAME', 'MSSQL数据库用户', 'AGENT', 'E', CURRENT_TIMESTAMP, 'sa')";

const char* GW_SQL_SETTING_INSERT_AGENT_MSSQL_PASSWORD =
    "insert into tn_gw_stg (stgcd, stgnm, stgtyp, sta, lmt, val)\n"
    "values ('AGENT.MSSQL.PASSWORD', 'MSSQL数据库密码', 'AGENT', 'E', CURRENT_TIMESTAMP, '')";

const char* GW_SQL_SETTING_INSERT_MSSQL_SLOW_QUERY =
    "insert into tn_gw_stg (stgcd, stgnm, stgtyp, sta, lmt, val)\n"
    "values ('MSSQL.SLOW_QUERY', '慢查询查询', 'MSSQL', 'E', CURRENT_TIMESTAMP, '\n"
    "select top 100\n"
    "  creation_time,\n"
    "  last_execution_time,\n"
    "  total_worker_time,\n"
    "  total_elapsed_time,\n"
    "  substring(st.text, qs.statement_start_offset / 2 + 1, (\n"
    "    case statement_end_offset\n"
    "    when -1 THEN DATALENGTH(st.text)\n"
    "    else qs.statement_end_offset\n"
    "    end - qs.statement_start_offset) / 2 + 1\n"
    "  ) as statement_text\n"
    "from sys.dm_exec_query_stats as qs\n"
    "cross apply sys.dm_exec_sql_text(qs.sql_handle) as st\n"
    "where 1 = 1\n"
    "and last_execution_time > {{{{lastExecutionTime}}}}\n"
    "order by total_worker_time desc;\n"
    "');";

const char* GW_SQL_SETTING_INSERT_MSSQL_LOCK =
    "insert into tn_gw_stg (stgcd, stgnm, stgtyp, sta, lmt, val)\n"
    "values ('MSSQL.LOCK', '死锁查询', 'MSSQL', 'E', CURRENT_TIMESTAMP, '\n"
    "SELECT  L.request_session_id AS SPID, \n"
    "    DB_NAME(L.resource_database_id) AS DatabaseName,\n"
    "    O.Name AS LockedObjectName, \n"
    "    P.object_id AS LockedObjectId, \n"
    "    L.resource_type AS LockedResource, \n"
    "    L.request_mode AS LockType,\n"
    "    ST.text AS SqlStatementText,        \n"
    "    ES.login_name AS LoginName,\n"
    "    ES.host_name AS HostName,\n"
    "    TST.is_user_transaction as IsUserTransaction,\n"
    "    AT.name as TransactionName,\n"
    "    CN.auth_scheme as AuthenticationMethod\n"
    "FROM    sys.dm_tran_locks L\n"
    "    JOIN sys.partitions P ON P.hobt_id = L.resource_associated_entity_id\n"
    "    JOIN sys.objects O ON O.object_id = P.object_id\n"
    "    JOIN sys.dm_exec_sessions ES ON ES.session_id = L.request_session_id\n"
    "    JOIN sys.dm_tran_session_transactions TST ON ES.session_id = TST.session_id\n"
    "    JOIN sys.dm_tran_active_transactions AT ON TST.transaction_id = AT.transaction_id\n"
    "    JOIN sys.dm_exec_connections CN ON CN.session_id = ES.session_id\n"
    "    CROSS APPLY sys.dm_exec_sql_text(CN.most_recent_sql_handle) AS ST\n"
    "WHERE   resource_database_id = db_id()\n"
    "ORDER BY L.request_session_id"
    "');";

const char* GW_SQL_SETTING_INSERT_MSSQL_DEAD_LOCK =
    "insert into tn_gw_stg (stgcd, stgnm, stgtyp, sta, lmt, val)\n"
    "values ('MSSQL.DEAD_LOCK', '死锁查询', 'MSSQL', 'E', CURRENT_TIMESTAMP, '\n"
    "SELECT XEvent.query(''(event/data/value/deadlock)[1]'') AS DeadlockGraph\n"
    "FROM ( \n"
    "  SELECT XEvent.query(''.'') AS XEvent\n"
    "  FROM ( \n"
    "    SELECT CAST(target_data AS XML) AS TargetData\n"
    "    FROM sys.dm_xe_session_targets st\n"
    "    JOIN sys.dm_xe_sessions s\n"
    "    ON s.address = st.event_session_address\n"
    "    WHERE s.name = ''system_health''\n"
    "    AND st.target_name = ''ring_buffer''\n"
    "  ) AS Data\n"
    "  CROSS APPLY \n"
    "  TargetData.nodes (''RingBufferTarget/event[@name=\"xml_deadlock_report\"]'') AS XEventData (XEvent)\n"
    ") AS src"
    "');";

static sqlite3* _gw_sqlite = NULL;

static int
gw_sqlite_callback(void* data, int column_count, char **column_values, char **column_names){
  int i;
  gfc_list_p result = (gfc_list_p) data;
  gfc_map_p row = gfc_map_new();
  for(i = 0; i < column_count; i++)
  {
    if (column_values[i] != NULL)
      gfc_map_put(row, column_names[i], strdup(column_values[i]));
  }
  gfc_list_append(result, row);
  return 0;
}

int
gw_sqlite_open(const char* work_directory)
{
  if (_gw_sqlite != NULL)
  {
    gw_sqlite_close();
    _gw_sqlite = NULL;
  }
  char dbpath[1024] = {'\0'};
  int i;

  strcpy(dbpath, work_directory);
  strcat(dbpath, "/guardwatch.db");
  int rc = sqlite3_open(dbpath, &_gw_sqlite);
  if (rc != SQLITE_OK)
  {
    gfc_log_error("gw_sqlite_open error: %s", sqlite3_errmsg(_gw_sqlite));
    sqlite3_close(_gw_sqlite);
    return rc;
  }

  gw_sqlite_execute(GW_SQL_SETTING_CREATE);
  gw_sqlite_execute(GW_SQL_SETTING_INSERT_ACKNOWLEDGEMENT_ID);
  gw_sqlite_execute(GW_SQL_SETTING_INSERT_SERVER_ADDRESS);
  gw_sqlite_execute(GW_SQL_SETTING_INSERT_SERVER_SNMP_PORT);
  gw_sqlite_execute(GW_SQL_SETTING_INSERT_SERVER_WEB_PORT);
  gw_sqlite_execute(GW_SQL_SETTING_INSERT_AGENT_NAME);
  gw_sqlite_execute(GW_SQL_SETTING_INSERT_AGENT_SNMP_PORT);
  gw_sqlite_execute(GW_SQL_SETTING_INSERT_AGENT_WEB_PORT);
  gw_sqlite_execute(GW_SQL_SETTING_INSERT_AGENT_MSSQL_ADDRESS);
  gw_sqlite_execute(GW_SQL_SETTING_INSERT_AGENT_MSSQL_PORT);
  gw_sqlite_execute(GW_SQL_SETTING_INSERT_AGENT_MSSQL_USERNAME);
  gw_sqlite_execute(GW_SQL_SETTING_INSERT_AGENT_MSSQL_PASSWORD);
  gw_sqlite_execute(GW_SQL_SETTING_INSERT_MSSQL_SLOW_QUERY);
  gw_sqlite_execute(GW_SQL_SETTING_INSERT_MSSQL_LOCK);
  gw_sqlite_execute(GW_SQL_SETTING_INSERT_MSSQL_DEAD_LOCK);

  return 0;
}

int
gw_sqlite_execute(const char* sql)
{
  char* errmsg;
  int rc;
  int i;

  rc = sqlite3_exec(_gw_sqlite, sql, 0, 0, &errmsg);
  if (rc)
  {
    gfc_log_error("couldn't prepare sql statement: %s", errmsg);
    return rc;
  }
  return GW_SQLITE_OK;
}

int
gw_sqlite_query(const char* sql, gfc_list_p* result)
{
  *result = gfc_list_new();

  int rc;
  char* errmsg = NULL;
  int i = 0;

  rc = sqlite3_exec(_gw_sqlite, sql, gw_sqlite_callback, (void*) *result, &errmsg);
  if( rc != SQLITE_OK )
  {
    gfc_log_error("gw_sqlite_query error: %s\n %s", sql, errmsg);
    sqlite3_free(errmsg);
  }

  return GW_SQLITE_OK;
}

void
gw_sqlite_close(void)
{
  sqlite3_close(_gw_sqlite);
  _gw_sqlite = NULL;
}