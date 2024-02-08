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
#include <pwd.h>

#include "gw_app.h"

int _gw_snmp_daemon_running = 1;
int _gw_snmp_trap_running = 1;
int _gw_http_web_running = 1;
int _gw_os_collect_running = 1;

volatile sig_atomic_t _gw_app_stop = 0;

static void
sigint_handler(int signum)
{
  printf("Stopping...");
  _gw_app_stop = 1;
  exit(signum);
}

char g_homedir[4096];

int 
main(int argc, char* argv[])
{
  signal(SIGINT, &sigint_handler);

  char workdir[1024] = {'\0'};
  struct passwd* pw = getpwuid(getuid());
  strcpy(g_homedir, pw->pw_dir);
  
  strcpy(workdir, pw->pw_dir);
  strcat(workdir, "/.guardwatch-agent");
  gfc_fs_mkdirs(workdir);

  gw_app_init(workdir);
  gw_app_start(workdir, workdir);

  while(!_gw_app_stop)
  {
    sleep(1);
  }

  return 0;
}