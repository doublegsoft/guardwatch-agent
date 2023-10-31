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
#include <windows.h>

#include <stdio.h>

#include <gfc/gfc.h>

#include "gw_app.h"
#include "gw_win.h"
#include "gw_mssql.h"
#include "../res/resource.h"

int _gw_snmp_daemon_running = 1;
int _gw_snmp_trap_running = 1;
int _gw_http_web_running = 1;
int _gw_os_collect_running = 1;

#ifdef WIN32
/*!
** It is the main entry for application for windows system.
**
** @return application result
*/
int WINAPI
WinMain(HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPSTR     lpCmdLine,
        int       nCmdShow)
{
  LPWSTR*     argv;
  int         argc;

  argv = CommandLineToArgvW(GetCommandLine(), &argc);
  if (strstr(argv[0], "--version") != NULL)
  {
    fprintf(stdout, "%s\n", GW_VERSION);
    LocalFree(argv);
    return 0;
  }
  LocalFree(argv);

  // logging
  gfc_log_init(GW_INSTALL_PATH "/log", "guardwatch");

  // registry
  HKEY hKey;
  const char* szValue = "c:\\doublegsoft\\guardwatch-agent\\guardwatch-agent.exe";
  LONG lRes = RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                            "Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                            0,
                            KEY_READ,
                            &hKey);
  if (lRes == ERROR_SUCCESS)
  {
    char szBuffer[2048] = {'\0'};
    DWORD dwBufferSize = sizeof(szBuffer);
    lRes = RegQueryValueExW(hKey, "GUARDWATCH代理程序", 0, NULL, (LPBYTE)szBuffer, &dwBufferSize);
    if (ERROR_SUCCESS != lRes)
    {
      lRes = RegSetValueEx(hKey,
                           "GUARDWATCH代理程序",
                           0,
                           REG_EXPAND_SZ,
                           (LPBYTE)szValue,
                           (DWORD)strlen(szValue) + 1);
    }
  }
  RegCloseKey(hKey);

  // make it single instance
  HANDLE mutex;
  mutex = CreateMutex(NULL, TRUE, "guardwatch");
  if ( GetLastError() == ERROR_ALREADY_EXISTS )
  {
    MessageBox(NULL, "程序已经在运行中...", "提示", MB_OK);
    return -1;
  }

  gfc_log_info("%s", "开始启动应用程序...");

//  int rc = gw_mssql_connect("120.79.137.143", 1433, "THIS4", "sa", "Winning123");
//  if (rc != 0)
//    gfc_log_info("%s", "数据库连接不成功！");

  // gfc_log_info("select 1 = %d", gw_mssql_query("select 1"));

  // 启动WIN32程序，包括系统托盘，各种服务线程
  gw_win_start(hInstance);

  gfc_log_info("%s", "应用程序服务启动完成！");

  CloseHandle(mutex);
  return 0;
}
#else
int main(int argc, char* argv[])
{
  return 0;
}
#endif