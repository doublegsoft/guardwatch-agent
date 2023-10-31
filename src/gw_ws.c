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

#include <netinet6/in6.h>

#include "gw_ws.h"

struct gw_ws_conn_s
{

  /*!
  ** the client sock file descriptor
  */
  int client_sock;

  /*!
  ** the web socket current state.
  */
  int state;

  /* Timeout thread and locks. */
  pthread_mutex_t mutex_state;
  pthread_cond_t cond_state;
  pthread_t third_timeout;
  int close_third;

  /*!
  ** the mutex for sending
  */
  pthread_mutex_t mutex_send;

  /* IP address. */
  char ip[INET6_ADDRSTRLEN];

  /* Ping/Pong IDs and locks. */
  int32_t last_pong_id;
  int32_t current_ping_id;

  /*!
  ** the mutex for ping/pong.
  */
  pthread_mutex_t mutex_pong;
};

struct gw_ws_frame_s
{
  /*!
  ** @brief Frame read.
  */
  unsigned char frame[GW_WS_MESSAGE_LEN];

  /*!
  ** @brief Processed message at the moment.
  */
  unsigned char *msg;

  /*!
  ** @brief Control frame payload
  */
  unsigned char msg_ctrl[125];

  /*!
  ** @brief Current byte position.
  */
  size_t cur_pos;

  /*!
  ** @brief Amount of read bytes.
  */
  size_t amount_read;

  /*!
  ** @brief Frame type, like text or binary.
  */
  int frame_type;

  /*!
  ** @brief Frame size.
  */
  uint64_t frame_size;

  /*!
  ** @brief Error flag, set when a read was not possible.
  */
  int error;

  /*!
  ** @brief Client connection structure.
  s*/
  gw_ws_conn_t *client;

};

struct gw_ws_events {



};


