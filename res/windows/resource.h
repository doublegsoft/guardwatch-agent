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

#define GW_VERSION        "0.1.0"
#define GW_INSTALL_PATH   "c:\\doublegsoft\\guardwatch-agent"

#define WM_TRAY_CALLBACK_MESSAGE          (WM_USER + 1)
#define WM_TIMER_SECOND                   (WM_USER + 2)

#define WC_TRAY_CLASS_NAME                "TRAY"

#define IDC_APP_ICON                      2
#define IDC_STATIC                        -1

#define IDI_TRAY_ICON                     101
#define IDI_TRAY_ICON_REVERSED            102

#define IDB_APP_START                     111
#define IDB_APP_STOP                      112
#define IDB_APP_SETTINGS                  113
#define IDB_APP_ABOUT                     114
#define IDB_APP_EXIT                      115

#define IDD_ABOUT                         201
#define IDD_SETTINGS                      202
#define IDC_SETTINGS_SERVER_ADDRESS       20201
#define IDC_SETTINGS_SERVER_SNMP_PORT     20202
#define IDC_SETTINGS_SERVER_WEB_PORT      20203
#define IDC_SETTINGS_AGENT_SNMP_PORT      20204
#define IDC_SETTINGS_AGENT_WEB_PORT       20205
#define IDC_SETTINGS_AGENT_NAME           20206
#define IDC_SETTINGS_SAVE                 20291

#define IDM_APP_START                     32778
#define IDM_APP_STOP                      32778 + 1
#define IDM_APP_SETTINGS                  32778 + 2
#define IDM_APP_ABOUT                     32778 + 3
#define IDM_APP_EXIT                      32778 + 4
#define IDM_APP_SEPARATOR                 32778 + 5

#define APPLICATION_NAME                  "GuardWatch Agent - WINDOWS系统终端监控代理程序"