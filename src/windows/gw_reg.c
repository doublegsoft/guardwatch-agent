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

#include "../res/registry.h"

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
  // registry
  HKEY hKey;
  const char* szValue = "c:\\doublegsoft\\guardwatch-agent\\guardwatch-agent.exe";
  LONG lRes = RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                            "Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                            0,
                            KEY_ALL_ACCESS,
                            &hKey);
  if (lRes == ERROR_SUCCESS)
  {
    char szBuffer[2048] = {'\0'};
    DWORD dwBufferSize = sizeof(szBuffer);
    lRes = RegQueryValueExA(hKey, "GUARDWATCH代理程序", 0, NULL, (LPBYTE)szBuffer, &dwBufferSize);
    if (ERROR_SUCCESS != lRes)
    {
      lRes = RegSetValueEx(hKey,
                           "GUARDWATCH代理程序",
                           0,
                           REG_EXPAND_SZ,
                           (LPBYTE)szValue,
                           (DWORD)strlen(szValue) + 1);
      if (lRes == ERROR_SUCCESS)
        MessageBox(NULL,
                   "成功注册GUARDWATCH代理程序！",
                   "注册成功",
                   MB_OK);
    }
    else
    {
      MessageBox(NULL,
                 "GUARDWATCH注册信息已经存在！",
                 "提示",
                 MB_OK);
    }
  }
  else
  {
    MessageBox(NULL,
               "请用管理员权限运行注册程序！",
               "错误",
               MB_OK);
  }
  RegCloseKey(hKey);

  return 0;
}