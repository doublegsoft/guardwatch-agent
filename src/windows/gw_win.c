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
#include <shellapi.h>
#include <string.h>

#include <gfc/gfc.h>

#include "gw_app.h"

#include "../res/resource.h"

extern gw_app_settings_t    _gw_app_settings;

extern const char* GW_SQL_SETTING_FIND;

static HINSTANCE  _hInstance;
static HWND       _hWndTray;
static BOOL       _bStarting = TRUE;

static void
gw_win_trayicon(UINT uIcon)
{
  static int _icon = 0;
  NOTIFYICONDATA  nid;

  memset(&nid, 0, sizeof(nid));

  nid.cbSize  = sizeof(nid);
  nid.hWnd    = _hWndTray;
  nid.uID     = 100;

  nid.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(uIcon));

  nid.uFlags |= NIF_ICON;
  nid.uFlags |= NIF_TIP;

  Shell_NotifyIcon(NIM_MODIFY, &nid);
}

static void
gw_win_on_start(HWND hWnd)
{
  /*!
  ** start the application.
  */
  gw_app_start(GW_INSTALL_PATH);
  gw_win_trayicon(IDI_TRAY_ICON_REVERSED);
  _bStarting = TRUE;
}

static void
gw_win_on_stop(HWND hWnd)
{
  gw_app_stop();
  gw_win_trayicon(IDI_TRAY_ICON);
  _bStarting = FALSE;
}

static void
gw_win_on_exit(HWND hWnd)
{
  gw_app_stop();
  exit(0);
}

static BOOL CALLBACK
gw_win_on_about(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
  switch(uMessage)
  {
  case WM_INITDIALOG:
    return TRUE;
  case WM_COMMAND:
    switch(LOWORD(wParam))
    {
    case IDOK:
    case IDCANCEL:
      EndDialog(hWnd, 0);
      break;
    }
    break;
  default:
    return FALSE;
  }
  return TRUE;
}

static BOOL CALLBACK
gw_win_on_settings(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
  gfc_list_p    result;
  char*         key = NULL;
  char*         val = NULL;
  char*         sql = "select * from tn_gw_stg";

  char          str_val[1024] = {'\0'};
  char          sql_update[2048] = {'\0'};
  int i;

  switch(uMessage)
  {
  case WM_INITDIALOG:
    /*!
    ** NOT WORKING ON WIN7
    */
    // gfc_sql_get("GW_SQL_SETTING_FIND", "{}", &sql);
    gw_sqlite_query(sql, &result);
    for (i = 0; i < gfc_list_size(result); i++)
    {
      gfc_map_p row = gfc_list_get(result, i);
      gfc_map_get(row, "stgcd", &key);

      if (strcmp(key, KEY_SERVER_ADDRESS) == 0)
      {
        gfc_map_get(row, "val", &val);
        strcpy(_gw_app_settings.server_address, val);
        SetWindowText(GetDlgItem(hWnd, IDC_SETTINGS_SERVER_ADDRESS), val);
      }

      if (strcmp(key, KEY_SERVER_SNMP_PORT) == 0)
      {
        gfc_map_get(row, "val", &val);
        strcpy(_gw_app_settings.server_snmp_port, val);
        SetWindowText(GetDlgItem(hWnd, IDC_SETTINGS_SERVER_SNMP_PORT), val);
      }

      if (strcmp(key, KEY_SERVER_WEB_PORT) == 0)
      {
        gfc_map_get(row, "val", &val);
        strcpy(_gw_app_settings.server_web_port, val);
        SetWindowText(GetDlgItem(hWnd, IDC_SETTINGS_SERVER_WEB_PORT), val);
      }

      if (strcmp(key, KEY_AGENT_SNMP_PORT) == 0)
      {
        gfc_map_get(row, "val", &val);
        strcpy(_gw_app_settings.agent_snmp_port, val);
        SetWindowText(GetDlgItem(hWnd, IDC_SETTINGS_AGENT_SNMP_PORT), val);
      }

      if (strcmp(key, KEY_AGENT_WEB_PORT) == 0)
      {
        gfc_map_get(row, "val", &val);
        strcpy(_gw_app_settings.agent_web_port, val);
        SetWindowText(GetDlgItem(hWnd, IDC_SETTINGS_AGENT_WEB_PORT), val);
      }

      if (strcmp(key, KEY_AGENT_NAME) == 0)
      {
        gfc_map_get(row, "val", &val);
        strcpy(_gw_app_settings.agent_name, val);
        SetWindowText(GetDlgItem(hWnd, IDC_SETTINGS_AGENT_NAME), val);
      }
      gfc_map_free(row);
    }
    // free(sql);
    gfc_list_free(result);
    return TRUE;
  case WM_COMMAND:
    switch(LOWORD(wParam))
    {
    case IDOK:

      /*!
      ** Saves the settings to local database.
      */
      // AGENT NAME
      GetWindowText(GetDlgItem(hWnd, IDC_SETTINGS_AGENT_NAME), str_val, 1024);
      sprintf(sql_update,
              "update tn_gw_stg set val = '%s' where stgcd = '" KEY_AGENT_NAME "'",
              str_val);
      gw_sqlite_execute(sql_update);

      // AGENT SNMP PORT
      GetWindowText(GetDlgItem(hWnd, IDC_SETTINGS_AGENT_SNMP_PORT), str_val, 1024);
      sprintf(sql_update,
      "update tn_gw_stg set val = '%s' where stgcd = '" KEY_AGENT_SNMP_PORT "'",
      str_val);
      gw_sqlite_execute(sql_update);

      // AGENT WEB PORT
      GetWindowText(GetDlgItem(hWnd, IDC_SETTINGS_AGENT_WEB_PORT), str_val, 1024);
      sprintf(sql_update,
      "update tn_gw_stg set val = '%s' where stgcd = '" KEY_AGENT_WEB_PORT "'",
      str_val);
      gw_sqlite_execute(sql_update);

      // SERVER ADDRESS
      GetWindowText(GetDlgItem(hWnd, IDC_SETTINGS_SERVER_ADDRESS), str_val, 1024);
      sprintf(sql_update,
      "update tn_gw_stg set val = '%s' where stgcd = '" KEY_SERVER_ADDRESS "'",
      str_val);
      gw_sqlite_execute(sql_update);

      // SERVER SNMP PORT
      GetWindowText(GetDlgItem(hWnd, IDC_SETTINGS_SERVER_SNMP_PORT), str_val, 1024);
      sprintf(sql_update,
      "update tn_gw_stg set val = '%s' where stgcd = '" KEY_SERVER_SNMP_PORT "'",
      str_val);
      gw_sqlite_execute(sql_update);

      // SERVER WEB PORT
      GetWindowText(GetDlgItem(hWnd, IDC_SETTINGS_SERVER_WEB_PORT), str_val, 1024);
      sprintf(sql_update,
      "update tn_gw_stg set val = '%s' where stgcd = '" KEY_SERVER_WEB_PORT "'",
      str_val);
      gw_sqlite_execute(sql_update);
    case IDCANCEL:
      EndDialog(hWnd, 0);
      break;
    }
    break;
  default:
    return FALSE;
  }
  return TRUE;
}

static void
gw_win_on_context_menu(HWND hWnd)
{
  HMENU hPopMenu;
  POINT lpClickPoint;
  UINT uFlags = MF_BYPOSITION | MF_STRING;
  GetCursorPos(&lpClickPoint);
  hPopMenu = CreatePopupMenu();

  HBITMAP bitmaps[4];

  bitmaps[0] = (HBITMAP)LoadImage((HMODULE)_hInstance, MAKEINTRESOURCE(IDB_APP_STOP), IMAGE_BITMAP, 16, 16, 0);
  bitmaps[1] = (HBITMAP)LoadImage((HMODULE)_hInstance, MAKEINTRESOURCE(IDB_APP_SETTINGS), IMAGE_BITMAP, 16, 16, 0);
  bitmaps[2] = (HBITMAP)LoadImage((HMODULE)_hInstance, MAKEINTRESOURCE(IDB_APP_ABOUT), IMAGE_BITMAP, 16, 16, 0);
  bitmaps[3] = (HBITMAP)LoadImage((HMODULE)_hInstance, MAKEINTRESOURCE(IDB_APP_EXIT), IMAGE_BITMAP, 16, 16, 0);

  if ( _bStarting == TRUE )
  {
    InsertMenu(hPopMenu, 0xFFFFFFFF, uFlags, IDM_APP_STOP, "停止");
  }
  else
  {
    InsertMenu(hPopMenu, 0xFFFFFFFF, uFlags, IDM_APP_START, "启动");
  }

  // https://docs.microsoft.com/en-us/windows/win32/menurc/using-menus#example-of-menu-item-bitmaps
  InsertMenu(hPopMenu, 0xFFFFFFFF, uFlags, IDM_APP_SETTINGS, "设置...");
  InsertMenu(hPopMenu, 0xFFFFFFFF, MF_SEPARATOR, IDM_APP_SEPARATOR, "");

  InsertMenu(hPopMenu, 0xFFFFFFFF, uFlags, IDM_APP_ABOUT, "关于...");
  InsertMenu(hPopMenu, 0xFFFFFFFF, MF_SEPARATOR, IDM_APP_SEPARATOR, "");

  InsertMenu(hPopMenu, 0xFFFFFFFF, uFlags, IDM_APP_EXIT, "退出");

  SetMenuItemBitmaps(hPopMenu, 0, MF_BITMAP, bitmaps[0], bitmaps[0]);
  SetMenuItemBitmaps(hPopMenu, 1, MF_BITMAP, bitmaps[1], bitmaps[1]);
  SetMenuItemBitmaps(hPopMenu, 2, MF_BITMAP, bitmaps[2], bitmaps[2]);
  SetMenuItemBitmaps(hPopMenu, 3, MF_BITMAP, bitmaps[3], bitmaps[3]);

  SetForegroundWindow(hWnd);
  TrackPopupMenu(hPopMenu,
                 TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_BOTTOMALIGN,
                 lpClickPoint.x,
                 lpClickPoint.y,
                 0,
                 hWnd,
                 NULL);

}

static LRESULT CALLBACK
gw_win_trayproc(HWND hWnd,
                UINT msg,
                WPARAM wParam,
                LPARAM lParam)
{
  int wlEvent;
  switch(msg)
  {
  case WM_CREATE:
    // SetTimer(hWnd, WM_TIMER_SECOND, 1000, NULL);
    break;
  case WM_CLOSE:
    DestroyWindow(hWnd);
    return 0;
  case WM_TRAY_CALLBACK_MESSAGE:
    switch(lParam)
    {
    case WM_LBUTTONDBLCLK:
      // MessageBox(NULL, "激活了一个系统铜盘双击事件", "提示", MB_OK);
      break;
    case WM_RBUTTONDOWN:
    case WM_CONTEXTMENU:
      gw_win_on_context_menu(hWnd);
      break;
    }
    break;
  case WM_TIMER:
    switch(wParam)
    {
    case WM_TIMER_SECOND:
      // gw_win_trayicon(0);
      break;
    }
    break;
  case WM_COMMAND:
    wlEvent = LOWORD(wParam);
    if (wlEvent == IDM_APP_ABOUT)
      DialogBox(_hInstance, MAKEINTRESOURCE(IDD_ABOUT), hWnd, gw_win_on_about);
    else if (wlEvent == IDM_APP_SETTINGS)
    {
      DialogBox(_hInstance, MAKEINTRESOURCE(IDD_SETTINGS), hWnd, gw_win_on_settings);
    }
    else if (wlEvent == IDM_APP_START)
      gw_win_on_start(hWnd);
    else if (wlEvent == IDM_APP_STOP)
      gw_win_on_stop(hWnd);
    else if (wlEvent == IDM_APP_EXIT)
      gw_win_on_exit(hWnd);
    break;
  }
  return DefWindowProc(hWnd, msg, wParam, lParam);
}

void
gw_win_start(HINSTANCE hInstance)
{
  // create a window to hide
  WNDCLASSEX wc;
  LPCTSTR MainWndClass = TEXT(APPLICATION_NAME);
  HWND hWnd;
  MSG msg;

  _hInstance = hInstance;

  // Class for our main window.
  wc.cbSize        = sizeof(wc);
  wc.style         = SW_HIDE;
  wc.cbClsExtra    = 0;
  wc.cbWndExtra    = 0;
  wc.hInstance     = hInstance;
  wc.hIcon         = (HICON) LoadImage(hInstance,
                                       MAKEINTRESOURCE(IDI_TRAY_ICON),
                                       IMAGE_ICON,
                                       0,
                                       0,
                                       LR_DEFAULTSIZE | LR_DEFAULTCOLOR | LR_SHARED);
  wc.hCursor       = (HCURSOR) LoadImage(NULL, IDC_ARROW, IMAGE_CURSOR, 0, 0, LR_SHARED);
  wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
  wc.lpszClassName = MainWndClass;

  RegisterClassEx(&wc);
  hWnd = CreateWindowEx(0,
                        MainWndClass,
                        MainWndClass,
                        WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        320, 200, NULL, NULL, hInstance, NULL);

  SetWindowText(hWnd, APPLICATION_NAME);
  ShowWindow(hWnd, SW_HIDE);
  UpdateWindow(hWnd);

  // make notification icon
  WNDCLASSEX wcTray;
  memset(&wcTray, 0, sizeof(wcTray));
  wcTray.lpfnWndProc = gw_win_trayproc;
  wcTray.cbSize = sizeof(WNDCLASSEX);
  wcTray.hInstance = GetModuleHandle(NULL);
  wcTray.lpszClassName = WC_TRAY_CLASS_NAME;
  RegisterClassEx(&wcTray);

  _hWndTray = CreateWindowEx(0, WC_TRAY_CLASS_NAME, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  UpdateWindow(_hWndTray);

  NOTIFYICONDATA nid;
  memset(&nid, 0, sizeof(nid));
  nid.cbSize = sizeof(nid);
  nid.hWnd = _hWndTray;
  nid.uID = 100;
  nid.uVersion = NOTIFYICON_VERSION;
  strcpy(nid.szTip, APPLICATION_NAME);
  nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
  nid.hIcon = (HICON) LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_TRAY_ICON));
  nid.uCallbackMessage = WM_TRAY_CALLBACK_MESSAGE;
  Shell_NotifyIcon(NIM_ADD, &nid);

  // 启动各种服务线程
  gw_win_on_start(_hWndTray);

  // Main message loop.
  while(GetMessage(&msg, NULL, 0, 0) > 0)
  {
    if (!TranslateAccelerator(msg.hwnd, NULL, &msg))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  gw_sqlite_close();
}