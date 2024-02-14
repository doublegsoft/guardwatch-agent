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

#ifndef __GW_HTTPD_H__
#define __GW_HTTPD_H__

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct gw_request_s
{
  struct mg_connection*     connection;

  char*                     body;
}
gw_request_t;

typedef struct gw_httpd_s
{
  char work_directory[2048];
}
gw_httpd_t;

/*!
**
** @param response_body
**
** @return
*/
void*
gw_httpd_serve(void*);

/*!
** Replies response to http client.
*/
int
gw_httpd_respond(char* response_body);

/*!
** Processes http post methods.
*/
int
gw_httpd_post(const char* request_body);

#ifdef __cplusplus
}
#endif

#endif // __GW_HTTPD_H__