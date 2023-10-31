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
#include<json.h>
#include<gfc.h>

#include "mongoose.h"

#include "gw_app.h"
#include "gw_sqlite.h"
#include "gw_httpd.h"
#ifdef WIN32
#include "gw_os.h"
#endif


/*!
** global application settings.
*/
extern gw_app_settings_t  _gw_app_settings;

/*!
** http web server running flag, defined in gw_main.c.
*/
extern int                _gw_http_web_running;

struct mg_serve_http_opts _http_server_opts;

const struct mg_str GW_HTTP_METHOD_OPTIONS            = MG_MK_STR("OPTIONS");

const struct mg_str GW_HTTP_METHOD_POST               = MG_MK_STR("POST");

const struct mg_str GW_HTTP_URI_OS_CPU                = MG_MK_STR("/api/agent/os/cpu");

const struct mg_str GW_HTTP_URI_OS_MEMORY             = MG_MK_STR("/api/agent/os/memory");

const struct mg_str GW_HTTP_URI_OS_PROCESS            = MG_MK_STR("/api/agent/os/process");

const struct mg_str GW_HTTP_URI_OS_VOLUME             = MG_MK_STR("/api/agent/os/volume");

const struct mg_str GW_HTTP_URI_ACKNOWLEDGE           = MG_MK_STR("/api/agent/acknowledge");

/*!
**
** @param param
**
** @return
*/
static void*
gw_httpd_thread(void* param)
{
  gw_request_t* data = (gw_request_t*) param;

  free(data->body);
  free(data);
  return NULL;
}

static int
gw_httpd_prefix_check(const struct mg_str *uri, const struct mg_str *prefix)
{
  return uri->len >= prefix->len && memcmp(uri->p, prefix->p, prefix->len) == 0;
}

/*!
** Sends ok status and result to http client.
**
** @param conn
**        the mongoose connection
**
** @param json
**        the json response
*/
static void
gw_httpd_ok(struct mg_connection* conn, const char* json_response)
{
  mg_printf(conn,
            "HTTP/1.1 200 OK\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Access-Control-Allow-Headers: *\r\n"
            "Access-Control-Allow-Methods: *\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: %d\r\n\r\n%s",
            (int) strlen(json_response), json_response);
}

static void
gw_httpd_202(struct mg_connection* conn)
{
  mg_printf(conn,
            "HTTP/1.1 204 No Content\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Access-Control-Allow-Headers: *\r\n"
            "Access-Control-Allow-Methods: *\r\n\r\n");
}

/*!
**
**
** @param conn
**
** @param ev
**
** @param ev_data
**
*/
static void
gw_httpd_event(struct mg_connection* conn,
              int ev,
              void* ev_data)
{
  struct http_message *hm = (struct http_message *) ev_data;
  struct mg_str key;
  int i = 0;

  switch (ev) {
  case MG_EV_HTTP_REQUEST:
    if (gw_httpd_prefix_check(&hm->method, &GW_HTTP_METHOD_OPTIONS))
    {
      gw_httpd_202(conn);
      return;
    }
    /*!
    ** just allow post method.
    */
    if (!gw_httpd_prefix_check(&hm->method, &GW_HTTP_METHOD_POST))
    {
      mg_serve_http(conn, hm, _http_server_opts);
      return;
    }
    char* result = NULL;
    /*!
    ** acknowledge the agent and retrieve id from server.
    */
    if (gw_httpd_prefix_check(&hm->uri, &GW_HTTP_URI_ACKNOWLEDGE))
    {
      if (hm->body.len > 0)
      {
        struct json_object* json_obj = json_tokener_parse(hm->body.p);
        struct json_object* json_obj_ackid;

        json_object_object_get_ex(json_obj, "ackid", &json_obj_ackid);
        char* ackid = json_object_get_string(json_obj_ackid);
        if (ackid == NULL)
        {
          gw_httpd_ok(conn, "{\"error\":{\"code\":-1,\"message\":\"参数中没有识别标识！\"}");
          return;
        }

        /*!
        ** update acknowledgement id in cache
        */
        strcpy(_gw_app_settings.acknowledgement_id, ackid);

        json_object_put(json_obj);

        /*!
        ** write acknowledgement id back to database
        */
        char sql[2048] = {'\0'};
        sprintf(sql, "update tn_gw_stg set val = '%s' where stgcd = '" KEY_AGENT_ACKNOWLEDGEMENT_ID "'", _gw_app_settings.acknowledgement_id);
        gw_sqlite_execute(sql);
        
        char          agent_name[1024] = {'\0'};
        gfc_list_p    result = NULL;
        char*         key = NULL;
        char*         val = NULL;
        gw_sqlite_query("select * from tn_gw_stg where stgcd = '" KEY_AGENT_NAME "'", &result);

        for (i = 0; result != NULL && i < gfc_list_size(result); i++)
        {
          gfc_map_p row = gfc_list_get(result, i);
          gfc_map_get(row, "stgcd", &key);

          if (strcmp(key, KEY_AGENT_NAME) == 0)
          {
            gfc_map_get(row, "val", &val);
            strcpy(agent_name, val);
          }

          gfc_map_free(row);
        }
        if (result != NULL)
          gfc_list_free(result);

        char utf8_agent_name[1024] = {'\0'};
        char resp[2048] = {'\0'};
        gfc_string_utf8(agent_name, utf8_agent_name, 1024);
        sprintf(resp, "{\"data\": {\"agentName\":\"%s\"}}", utf8_agent_name);
        gw_httpd_ok(conn, resp);
        return;
      }
    }
#ifdef WIN32
    else if (gw_httpd_prefix_check(&hm->uri, &GW_HTTP_URI_OS_CPU))
    {
      gw_os_cpu(&result);
    }
    else if (gw_httpd_prefix_check(&hm->uri, &GW_HTTP_URI_OS_MEMORY))
    {
      gw_os_memory(&result);
    }
    else if (gw_httpd_prefix_check(&hm->uri, &GW_HTTP_URI_OS_PROCESS))
    {
      gw_os_processes(&result);
    }
    else if (gw_httpd_prefix_check(&hm->uri, &GW_HTTP_URI_OS_VOLUME))
    {
      gw_os_volumes(&result);
    }
#endif
    if (result != NULL)
    {
      gw_httpd_ok(conn, result);
      free(result);
      return;
    }
    mg_serve_http(conn, hm, _http_server_opts);
    break;
  default:
    break;
  }
}

void*
gw_httpd_serve(void* data)
{
  struct mg_mgr             mgr;
  struct mg_connection*     conn;

  _gw_http_web_running = 1;

  mg_mgr_init(&mgr, NULL);
  conn = mg_bind(&mgr, _gw_app_settings.agent_web_port, gw_httpd_event);

  mg_set_protocol_http_websocket(conn);
  _http_server_opts.document_root = _gw_app_settings.server_web_root;
  _http_server_opts.enable_directory_listing = "false";
  _http_server_opts.extra_headers = "Access-Control-Allow-Origin: *";

  gfc_log_info("web server listening on %s", _gw_app_settings.agent_web_port);

  while (1)
  {
    if (_gw_http_web_running != 1) break;
    mg_mgr_poll(&mgr, 100);
  }

  mg_mgr_free(&mgr);

  return NULL;
}