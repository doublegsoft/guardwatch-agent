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
#include <gfc.h>
#include <signal.h>
#include <unistd.h>

#include "gw_app.h"

int _gw_snmp_daemon_running = 1;
int _gw_snmp_trap_running = 1;
int _gw_http_web_running = 1;
int _gw_os_collect_running = 1;

static void
sigterm_handler(int signum)
{
//  printf("Handler thread ID: %d\n", syscall(SYS_gettid));
  exit(signum);
}

int main(int argc, char* argv[])
{
  char workdir[1024] = {'\0'};
  char logdir[1024] = {'\0'};

  signal(SIGTERM, sigterm_handler);

  strcpy(workdir, "/root/.guardwatch-agent");
  gfc_fs_mkdirs(workdir);

  strcpy(logdir, workdir);
  strcat(logdir, "/log");

  gfc_gc_init();
  gfc_log_init(logdir, "guardwatch-agent");
  gw_app_start(workdir, workdir);

  while(1)
    sleep(1);

  return 0;
}