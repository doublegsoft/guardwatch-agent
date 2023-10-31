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
#include <winternl.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <psapi.h>
#include <json-c/json.h>

#include <gtdb.h>

#include "gw_os.h"

#define SystemProcessorPerformanceInformation 0x8
#define SystemBasicInformation    0x0

extern int _gw_os_collect_running;

static const __int64 MEGABYTES = 1024 * 1024;

void gw_os_error(const char* fun, int err)
{
  WCHAR   wszMsgBuff[512];
  DWORD   dwChars;

  // Try to get the message from the system errors.
  dwChars = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
                          FORMAT_MESSAGE_IGNORE_INSERTS,
                          NULL,
                          err,
                          0,
                          wszMsgBuff,
                          512,
                          NULL );

  if (0 == dwChars)
  {
    HINSTANCE hInst;

    // Load the library.
    hInst = LoadLibrary(L"ntdsbmsg.dll");
    if ( NULL == hInst )
    {
      gfc_log_error("gw_os_error: could not load ntdsbmsg.dll");
      return;
    }

    // Try getting message text from ntdsbmsg.
    dwChars = FormatMessage(FORMAT_MESSAGE_FROM_HMODULE |
                            FORMAT_MESSAGE_IGNORE_INSERTS,
                            hInst,
                            err,
                            0,
                            wszMsgBuff,
                            512,
                            NULL);
    FreeLibrary(hInst);
  }
  gfc_log_error("%s: error code %d, %s", fun, err, wszMsgBuff);
}

int
gw_os_cpu(char** result)
{
  /*!
  **
  */
  typedef DWORD(WINAPI * PNTQUERYSYSYTEMINFORMATION)(DWORD info_class, void *out, DWORD size, DWORD *out_size);
  PNTQUERYSYSYTEMINFORMATION pNtQuerySystemInformation = NULL;
  pNtQuerySystemInformation = (PNTQUERYSYSYTEMINFORMATION)GetProcAddress(GetModuleHandle("NTDLL.DLL"), "NtQuerySystemInformation");

  SYSTEM_BASIC_INFORMATION sbi;
  SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION* spi;

  DWORD return_length;
  DWORD status = pNtQuerySystemInformation(SystemBasicInformation, &sbi,
                                           sizeof(SYSTEM_BASIC_INFORMATION),
                                           &return_length);

  spi = malloc(sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION) * sbi.NumberOfProcessors);

  memset(spi, 0, sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION)*sbi.NumberOfProcessors);

  status = pNtQuerySystemInformation(SystemProcessorPerformanceInformation,
                                     spi,
                                     sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION) * sbi.NumberOfProcessors,
                                     &return_length);
  int number_of_cores = return_length / sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION);

  static ULARGE_INTEGER  ul_sys_idle_old[32];
  static ULARGE_INTEGER  ul_sys_kernel_old[32];
  static ULARGE_INTEGER  ul_sys_user_old[32];

  float          usage = 0;
  float          usageAccum = 0;

  status = pNtQuerySystemInformation(SystemProcessorPerformanceInformation, spi,
                                     sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION) * number_of_cores,
                                     &pNtQuerySystemInformation);
  usageAccum = 0;

  struct json_object* jobj = json_object_new_object();
  struct json_object* jarr = json_object_new_array();

  json_object_object_add(jobj, "data", jarr);

  for (int ii = 0; ii < number_of_cores; ii++)
  {
    struct json_object* jobj_row = json_object_new_object();

    ULARGE_INTEGER        ul_sys_idle;
    ULARGE_INTEGER        ul_sys_kernel;
    ULARGE_INTEGER        ul_sys_user;

    ul_sys_idle.QuadPart = spi[ii].IdleTime.QuadPart;
    ul_sys_kernel.QuadPart = spi[ii].KernelTime.QuadPart;
    ul_sys_user.QuadPart = spi[ii].UserTime.QuadPart;

    ULONGLONG kernelTime = (ul_sys_kernel.QuadPart - ul_sys_kernel_old[ii].QuadPart);
    ULONGLONG usertime = (ul_sys_user.QuadPart - ul_sys_user_old[ii].QuadPart);
    ULONGLONG idletime = (ul_sys_idle.QuadPart - ul_sys_idle_old[ii].QuadPart);

    ul_sys_kernel_old[ii].QuadPart = ul_sys_kernel.QuadPart;
    ul_sys_user_old[ii].QuadPart = ul_sys_user.QuadPart;
    ul_sys_idle_old[ii].QuadPart = ul_sys_idle.QuadPart;

    ULONGLONG proctime = kernelTime + usertime - idletime;

    ULONGLONG totaltime = kernelTime + usertime;

    json_object_object_add(jobj_row, "processed", json_object_new_double(proctime));
    json_object_object_add(jobj_row, "total", json_object_new_double(totaltime));
    json_object_array_add(jarr, json_object_get(jobj_row));

    usage = (float) (proctime * 100) / totaltime;
    usageAccum += usage;
  }
  struct json_object* jobj_accum = json_object_new_object();
  usageAccum /= number_of_cores;
  json_object_object_add(jobj, "accum", json_object_new_double(usageAccum));


  const char* json = json_object_to_json_string(jobj);
  *result = malloc(sizeof(char) * (strlen(json) + 1));
  strcpy(*result, json);

  json_object_put(jobj);

  free(spi);
  return 0;
}

int
gw_os_memory(char** result)
{
  MEMORYSTATUSEX status;
  status.dwLength = sizeof (status);
  GlobalMemoryStatusEx (&status);

  struct json_object* jobj = json_object_new_object();
  struct json_object* jobj_data = json_object_new_object();

  json_object_object_add(jobj, "data", jobj_data);

  json_object_object_add(jobj_data, "free", json_object_new_double(status.ullAvailPhys / MEGABYTES));
  json_object_object_add(jobj_data, "total", json_object_new_double(status.ullTotalPhys / MEGABYTES));

  const char* json = json_object_to_json_string(jobj);
  *result = malloc(sizeof(char) * (strlen(json) + 1));
  strcpy(*result, json);

  json_object_put(jobj);

  return GW_OS_OK;
}

int
gw_os_volumes(char** result)
{
  char        volume[MAX_PATH];
  char        name[MAX_PATH];
  int         count = MAX_PATH;
  BOOL        successful = FALSE;
  HANDLE      hFound = INVALID_HANDLE_VALUE;

  __int64        free_bytes;
  __int64        total_bytes;
  __int64        total_free_bytes;

  hFound = FindFirstVolume(volume, MAX_PATH);

  if (hFound == INVALID_HANDLE_VALUE)
    return GW_OS_ERROR;

  struct json_object* jobj = json_object_new_object();
  struct json_object* jarr = json_object_new_array();

  json_object_object_add(jobj, "data", jarr);
  for (;;)
  {
    for (;;)
    {
      successful = GetVolumePathNamesForVolumeName(volume, name, count, &count);
      if (successful) break;
      if (GetLastError() != ERROR_MORE_DATA) break;
    }
    if (successful && strlen(name) > 0)
    {
      int rc = GetDiskFreeSpaceEx(name, &free_bytes, &total_bytes, &total_free_bytes);

      if (rc != 0)
      {
        struct json_object* jobj_row = json_object_new_object();
        json_object_object_add(jobj_row, "volume", json_object_new_string(name));
        json_object_object_add(jobj_row, "free", json_object_new_double(free_bytes / MEGABYTES));
        json_object_object_add(jobj_row, "total", json_object_new_double(total_bytes / MEGABYTES));
        json_object_array_add(jarr, json_object_get(jobj_row));
      }
    }
    successful = FindNextVolume(hFound, volume, MAX_PATH);
    if (!successful)
      break;
  }

  const char* json = json_object_to_json_string(jobj);
  *result = malloc(sizeof(char) * (strlen(json) + 1));
  strcpy(*result, json);

  FindVolumeClose(hFound);
  json_object_put(jobj);

  return GW_OS_OK;
}

int
gw_os_processes(char** result)
{
  DWORD aProcesses[1024],
        cbNeeded,
        countProcess;

  PMIB_TCPTABLE2 pTcpTable;
  ULONG ulSize = 0;
  DWORD dwRetVal = 0;

  char szLocalAddr[128];
  char szRemoteAddr[128];

  struct in_addr IpAddr;

  unsigned int i, j;

  if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
  {
    gw_os_error("gw_os_processes", GetLastError());
    return GW_OS_ERROR;
  }

  struct json_object* jobj = json_object_new_object();
  struct json_object* jarr = json_object_new_array();
  json_object_object_add(jobj, "data", jarr);

  countProcess = cbNeeded / sizeof(DWORD);
  for ( i = 0; i < countProcess; i++ )
  {
    if( aProcesses[i] == 0 )
      continue;
    char szProcessName[MAX_PATH] = {'\0'};
    char szFilePath[2048] = {'\0'};

    /*!
    ** Get a handle to the process.
    */
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                                  FALSE,
                                  aProcesses[i]);
    if (NULL != hProcess)
    {
      HMODULE hMod;
      DWORD cbNeeded;
      IO_COUNTERS ioCounters;

      GetProcessImageFileNameA(hProcess, szFilePath, 2048);
      if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded))
      {
        GetModuleBaseNameA(hProcess,
                           hMod,
                           szProcessName,
                           sizeof(szProcessName) / sizeof(TCHAR));

        struct json_object* jobj_row = json_object_new_object();
        json_object_object_add(jobj_row, "id", json_object_new_int(aProcesses[i]));
        json_object_object_add(jobj_row, "name", json_object_new_string(szProcessName));
        json_object_array_add(jarr, json_object_get(jobj_row));
      }

      // process IO statistics
      GetProcessIoCounters(hProcess, &ioCounters);

      if (szProcessName[0] == 0)
        continue;
    }

    CloseHandle(hProcess);
  } // for

  pTcpTable = (MIB_TCPTABLE2*) HeapAlloc(GetProcessHeap(), 0, sizeof(MIB_TCPTABLE2));
  if (pTcpTable == NULL) return GW_OS_ERROR;

  ulSize = sizeof (MIB_TCPTABLE);

  /*!
  ** Make an initial call to GetTcpTable2 to get the necessary size into the ulSize variable
  */
  if ((dwRetVal = GetTcpTable2(pTcpTable, &ulSize, TRUE)) == ERROR_INSUFFICIENT_BUFFER)
  {
    HeapFree(GetProcessHeap(), 0, pTcpTable);
    pTcpTable = HeapAlloc(GetProcessHeap(), 0, ulSize);
    if (pTcpTable == NULL)
    {
      gw_os_error("gw_os_processes", GetLastError());
      return GW_OS_ERROR_INSUFFICIENT_BUFFER;
    }
  }

  /*!
  ** Make a second call to GetTcpTable2 to get the actual data we require
  */
  if ((dwRetVal = GetTcpTable2(pTcpTable, &ulSize, TRUE)) == NO_ERROR)
  {
    struct in_addr IpAddr;
    char szLocalAddr[128];
    for (i = 0; i < (int) pTcpTable->dwNumEntries; i++) {
      if (pTcpTable->table[i].dwState == MIB_TCP_STATE_LISTEN)
      {
        IpAddr.S_un.S_addr = (u_long) pTcpTable->table[i].dwLocalAddr;
        strcpy_s(szLocalAddr, sizeof (szLocalAddr), inet_ntoa(IpAddr));
        for (j = 0; j < json_object_array_length(jarr); j++)
        {
          struct json_object* jobj_row = json_object_array_get_idx(jarr, j);
          struct json_object* jobj_row_id;
          json_object_object_get_ex(jobj_row, "id", &jobj_row_id);
          if (json_object_get_int(jobj_row_id) == pTcpTable->table[i].dwOwningPid
              && strcmp(szLocalAddr, "127.0.0.1") != 0)
          {
            json_object_object_add(jobj_row, "port",
                                   json_object_new_int(ntohs((u_short) pTcpTable->table[i].dwLocalPort)));
            break;
          }
        }
      }
    }
  }
  else
  {
    HeapFree(GetProcessHeap(), 0, pTcpTable);
  }

  if (pTcpTable != NULL)
  {
    HeapFree(GetProcessHeap(), 0, pTcpTable);
    pTcpTable = NULL;
  }

  const char* json = json_object_to_json_string(jobj);
  *result = malloc(sizeof(char) * (strlen(json) + 1));
  strcpy(*result, json);

  json_object_put(jobj);

  return GW_OS_OK;
}

/*!
** Collects os performance metrics.
**
** @return always {@code NULL}
*/
void*
gw_os_collect(void* data)
{
  time_t raw;
  char* os_result;
  while(1)
  {
    if (_gw_os_collect_running != 1) break;

    time (&raw);

    /*!
    ** CPU
    */
    gw_os_cpu(&os_result);
    gtdb_insert("cpu", raw, os_result);
    free(os_result);

    /*!
    ** MEMORY
    */
    gw_os_memory(&os_result);
    gtdb_insert("memory", raw, os_result);
    free(os_result);

    sleep(1);
  }
}