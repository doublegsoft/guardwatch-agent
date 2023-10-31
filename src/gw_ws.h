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

#ifndef __GW_WS_H__
#define __GW_WS_H__

#ifdef __cplusplus
extern "C" {
#endif

/*!
** the max client count when connecting once.
*/
#define GW_WS_CLIENT_MAX                    8
#define GW_WS_MESSAGE_LEN                   2048

#define GW_WS_HANDSHAKE_ACCEPT              "HTTP/1.1 101 Switching Protocols\r\n" \
                                            "Upgrade: websocket\r\n"               \
                                            "Connection: Upgrade\r\n"              \
                                            "Sec-WebSocket-Accept: "

typedef struct gw_ws_conn_s                 gw_ws_conn_t;
typedef struct gw_ws_frame_s                gw_ws_frame_t;

gw_ws_conn_t* gw_ws_client_open();

void gw_ws_client_close(gw_ws_conn_t* client);

#ifdef __cplusplus
}
#endif

#endif /* __GW_WS_H__ */
