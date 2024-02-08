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
#include <rrd.h>

#include "gw_app.h"

int _gw_snmp_daemon_running = 1;
int _gw_snmp_trap_running = 1;
int _gw_http_web_running = 1;
int _gw_os_collect_running = 1;

static void
sigterm_handler(int signum)
{
  exit(signum);
}

char g_homedir[4096];

int 
main(int argc, char* argv[])
{
  signal(SIGTERM, sigterm_handler);

  char workdir[1024] = {'\0'};
  char logdir[1024] = {'\0'};
  char rrddir[1024] = {'\0'};
  char rrdfile[1024] = {'\0'};

  struct passwd* pw = getpwuid(getuid());
  strcpy(g_homedir, pw->pw_dir);
  
  strcpy(workdir, pw->pw_dir);
  strcat(workdir, "/.guardwatch-agent");
  gfc_fs_mkdirs(workdir);

  strcpy(logdir, workdir);
  strcat(logdir, "/log");
  
  strcpy(rrddir, workdir);
  strcat(rrddir, "/rrd");
  gfc_fs_mkdirs(rrddir);

  strcpy(rrdfile, rrddir);
  strcat(rrdfile, "/guardwatch.rrd");
  printf("%s\n", rrdfile);

  const char* rrd_argv[] = {
    "create", rrdfile, 
    "-s", "300",
    "DS:cpu:GAUGE:600:U:U", 
    "RRA:AVERAGE:0.5:1:600", NULL
  };
  int rc = rrd_create(6, (char**)rrd_argv);
  if (rc) {
    printf("error: %s\n", rrd_get_error());
  }

  const char* rrdupt_argv[] = {
    "update", rrdfile,
    "N:42",
    NULL
  };
  rrd_update(3, rrdupt_argv);

  gfc_gc_init();
  gfc_log_init(logdir, "guardwatch-agent");
  gw_app_start(workdir, workdir);

  // while(1)
  // {
  //   sleep(1);
  // }

  return 0;
}