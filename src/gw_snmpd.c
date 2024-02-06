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

/*!
**
**
** @author <a href="mailto:guo.guo.gan@gmail.com">Christian Gann</a>
**
** @since 0.1
**
** @version
** 0.1 - added agent heartbeat notification
*/
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/library/large_fd_set.h>

#include <time.h>
#include <unistd.h>
#ifdef __linux__
#include <signal.h>
#endif

#include <gfc.h>

/*
 * contains prototypes
 */
#include "gw_app.h"

#define NETSNMP_LARGE_FD_ZERO(fdset)          do { (fdset)->lfs_setptr->fd_count = 0; } while(0)
#define HEARTBEAT_RATE                        60

extern gw_app_settings_t _gw_app_settings;

extern int _gw_snmp_daemon_running;
extern int _gw_snmp_trap_running;

static const char* app_name = "guardwatch-agent";

// standard trap oid
oid oid_snmp_trap[]           = { 1, 3, 6, 1, 6, 3, 1, 1, 4, 1};
oid oid_sys_uptime[]          = {1, 3, 6, 1, 2, 1, 1, 3, 0};

oid oid_gw[]                  = {1, 3, 6, 1, 4, 1, 80717, 1};

// heartbeat trap oid
oid oid_gw_heartbeat[]        = {1, 3, 6, 1, 4, 1, 80717, 1, 3, 1};

//
oid oid_gw_trap_address[]     = {1, 3, 6, 1, 4, 1, 80717, 1, 1, 1, 0};
oid oid_gw_agent_name[]       = {1, 3, 6, 1, 4, 1, 80717, 1, 1, 2, 0};
oid oid_gw_agent_location[]   = {1, 3, 6, 1, 4, 1, 80717, 1, 1, 3, 0};

u_char*
gw_snmpd_access(struct variable *vp,
               oid* name,
               size_t* length,
               int exact,
               size_t* var_len,
               WriteMethod** write_method)
{
  gfc_log_info("%s", "执行到这里，证明就成功了");
  return 0;
}

struct variable7 var_gw[] = {
    {1, ASN_OBJECT_ID, NETSNMP_OLDAPI_RWRITE, gw_snmpd_access, 1, {1}},
    {11, ASN_OCTET_STR, NETSNMP_OLDAPI_RWRITE, gw_snmpd_access, 2, {1, 1}},
    {12, ASN_OCTET_STR, NETSNMP_OLDAPI_RWRITE, gw_snmpd_access, 2, {1, 2}},
    {13, ASN_OCTET_STR, NETSNMP_OLDAPI_RWRITE, gw_snmpd_access, 2, {1, 3}},
    {2, ASN_OBJECT_ID, NETSNMP_OLDAPI_RONLY, gw_snmpd_access, 1, {2}},
    {3, ASN_OBJECT_ID, NETSNMP_OLDAPI_RONLY, gw_snmpd_access, 1, {3}},
    {31, ASN_OBJECT_ID, NETSNMP_OLDAPI_RONLY, gw_snmpd_access, 2, {3, 1}}
};

static void
gw_snmpd_error(netsnmp_session* session)
{
  int cliberr = 0;
  int snmperr = 0;
  char* errmsg;
  snmp_error(session, &cliberr, &snmperr, &errmsg);
  gfc_log_error("SNMP Error %d: %s", snmperr, errmsg);
}

/*!
**
** @param reg
** @param arg
*/
static void
gw_snmpd_heartbeat(unsigned int reg, void* arg)
{
  netsnmp_session*  ss;
  netsnmp_session   session;
  netsnmp_pdu    *pdu, *response;
  char *trap = NULL;
  char comm[] = "public";
  char peername[2048];

  strcpy(peername, _gw_app_settings.server_address);
  strcat(peername, ":");
  strcat(peername, _gw_app_settings.server_snmp_port);
  snmp_sess_init(&session);
  session.version = SNMP_VERSION_2c;
  session.community = (u_char*) comm;
  session.community_len = strlen(comm);

  session.peername = peername;

  SOCK_STARTUP;

  ss = snmp_add(&session,
                netsnmp_transport_open_client(app_name, session.peername),
                NULL, NULL);
  if (ss == NULL) {
    int cliberr = 0;
    int snmperr = 0;
    char* errmsg;
    snmp_error(&session, &cliberr, &snmperr, &errmsg);
    gfc_log_error("无法连接到%s，错误码 %d:%s", session.peername, snmperr, errmsg);
    SOCK_CLEANUP;
    return;
  }

  pdu = snmp_pdu_create(SNMP_MSG_TRAP2);
  pdu->community = (u_char*) comm;
  pdu->community_len = strlen(comm);
  pdu->trap_type = SNMP_TRAP_ENTERPRISESPECIFIC;

  long sysuptime;
  char csysuptime [20];
  sysuptime = get_uptime ();
  sprintf (csysuptime, "%ld", sysuptime);
  trap = csysuptime;

  snmp_add_var(pdu, oid_sys_uptime, OID_LENGTH(oid_sys_uptime), 't', trap);
  snmp_add_var(pdu, oid_gw_heartbeat, OID_LENGTH(oid_gw_heartbeat), ASN_OBJECT_ID, (const char*) oid_gw_heartbeat);
//  snmp_add_var(pdu, oid_gw_agent_name, OID_LENGTH(oid_gw_agent_name), ASN_OCTET_STR, "this is the name");
//  snmp_add_var(pdu, oid_gw_agent_location, OID_LENGTH(oid_gw_agent_location), ASN_OCTET_STR, "this is then location");

  pdu->variables->next_variable = NULL;   /* No more variables after this one */
  pdu->variables->name = oid_gw_agent_name;
  pdu->variables->name_length = OID_LENGTH(oid_gw_agent_name);
  pdu->variables->type = ASN_OCTET_STR;

  char sending_message[2048] = {'\0'};
  strcpy(sending_message, _gw_app_settings.acknowledgement_id);
  strcat(sending_message, "/");
  strcat(sending_message, _gw_app_settings.agent_web_port);
  pdu->variables->val.string = (unsigned char *) sending_message;
  pdu->variables->val_len = strlen(sending_message);

  send_trap_to_sess(ss, pdu);
  snmp_close(ss);

  SOCK_CLEANUP;
}

/*!
**
*/
static int
gw_snmpd_set(int action,
            u_char* var_val,
            u_char var_val_type,
            size_t var_val_len,
            u_char* stat,
            oid* name,
            size_t name_len)
{
  switch(action)
  {
  case ACTION:
    break;
  }
  return SNMP_ERR_NOERROR;
}

int
gw_snmpd_request(netsnmp_mib_handler*           handler,
                netsnmp_handler_registration*  reginfo,
                netsnmp_agent_request_info*    reqinfo,
                netsnmp_request_info*          requests)
{

  gfc_log_info("Got request, mode = %d, data = %s", reqinfo->mode, requests->requestvb_start->val.string);
  switch (reqinfo->mode) {
  case MODE_SET_ACTION:
    break;
  case MODE_GET:
    break;
  }

  return SNMP_ERR_NOERROR;
}

/*!
**
** @param num
*/
static void
gw_snmpd_shutdown(int num)
{

}

static int
gw_snmpd_oid_callback(int majorID, int minorID,
                     void* serverarg, void* clientarg)
{
  struct view_parameters* view_params = (struct view_parameters*) serverarg;
  gfc_log_info("进入了回调函数：oid = %s", (char*) view_params->pdu->enterprise);

  view_params->errorcode = VACM_SUCCESS;
  return VACM_SUCCESS;
}

void*
gw_snmpd_trap(void* data)
{
//  init_snmp(app_name);
  _gw_snmp_trap_running = 1;
  gfc_log_info("%s", "注册心跳TRAP！");
  while(_gw_snmp_trap_running)
  {
    sleep(HEARTBEAT_RATE);
    gw_snmpd_heartbeat(0, NULL);
  }
}

void*
gw_snmpd_serve(void* data)
{
  const char* work_directory = (const char*) data;
  int             prepared_sockets = 0;
  int             rc = SNMP_ERR_NOERROR;

  if (_gw_snmp_daemon_running == 2)
    goto RUNNING;

  signal(SIGTERM, gw_snmpd_shutdown);
  signal(SIGINT, gw_snmpd_shutdown);

  // without root privilege
  netsnmp_ds_set_boolean(NETSNMP_DS_APPLICATION_ID,
                         NETSNMP_DS_AGENT_NO_ROOT_ACCESS, TRUE);

  netsnmp_ds_set_boolean(NETSNMP_DS_APPLICATION_ID,
                         NETSNMP_DS_AGENT_AGENTX_MASTER, TRUE);

  netsnmp_ds_set_boolean(NETSNMP_DS_LIBRARY_ID,
                         NETSNMP_DS_LIB_DONT_LOAD_HOST_FILES, TRUE);

  netsnmp_ds_set_int(NETSNMP_DS_APPLICATION_ID,
                     NETSNMP_DS_AGENT_AGENTX_TIMEOUT, -1);
  netsnmp_ds_set_int(NETSNMP_DS_APPLICATION_ID,
                     NETSNMP_DS_AGENT_AGENTX_RETRIES, -1);
  netsnmp_ds_set_int(NETSNMP_DS_APPLICATION_ID,
                     NETSNMP_DS_AGENT_CACHE_TIMEOUT, 5);

  netsnmp_ds_set_string(NETSNMP_DS_APPLICATION_ID,
                        NETSNMP_DS_AGENT_PORTS, "udp:0.0.0.0:6161");

  netsnmp_ds_set_string(NETSNMP_DS_LIBRARY_ID,
                        NETSNMP_DS_LIB_APPTYPE, app_name);

  init_snmp_logging();
  snmp_set_do_debugging(TRUE);

  char logpath[2048];
  strcpy(logpath, work_directory);
  strcat(logpath, "/log/snmp.log");
  snmp_enable_filelog(logpath, 2);

  SOCK_STARTUP;

  /*!
  ** initialize agent
  */
  if (init_agent(app_name) != SNMP_ERR_NOERROR)
  {
    gfc_log_info("%s", "初始化代理程序失败！");
    return NULL;
  }

  // 注册Trap的监听地址，格式为IP:Port。

  /*!
  ** 以下代码是正确的，只要PDU完全通过了安全认证就可以正常回掉了。
  */
  netsnmp_handler_registration* reg_gw_trap_address;
  reg_gw_trap_address = netsnmp_create_handler_registration(app_name,
                                                            gw_snmpd_request,
                                                            oid_gw_trap_address,
                                                            OID_LENGTH(oid_gw_trap_address),
                                                            HANDLER_CAN_RWRITE);
  netsnmp_register_instance(reg_gw_trap_address);

  /*!
  ** References the code in snmp_agent.c
  ** ```
  ** case SNMP_MSG_GET:
  **   view = in_a_view(varbind_ptr->name, &varbind_ptr->name_length,
  **                    asp->pdu, varbind_ptr->type);
  **   if (view != VACM_SUCCESS)
  **     snmp_set_var_typed_value(varbind_ptr, SNMP_NOSUCHOBJECT, NULL, 0);
  **   break;
  ** ```
  **
  ** And the code in agent_registry.c
  ** ```
  ** NETSNMP_RUNTIME_PROTOCOL_CHECK(pdu->version,unsupported_version);
  ** snmp_call_callbacks(SNMP_CALLBACK_APPLICATION,
  **                     SNMPD_CALLBACK_ACM_CHECK, &view_parms);
  ** return view_parms.errorcode;
  ** ```
  */
  snmp_register_callback(1, 0, gw_snmpd_oid_callback, NULL);

  /*!
  ** start snmp library
  */
  init_snmp(app_name);

  netsnmp_transport *transport;

  if ((rc = init_master_agent()) != SNMP_ERR_NOERROR)
  {
    gfc_log_info("错误代码(%d)：%s", rc, "初始化主代理程序失败！");
    return NULL;
  }

//  rc = netsnmp_daemonize(0, NULL);
//  if(rc != SNMP_ERR_NOERROR) {
//    gfc_log_info("错误代码(%d)：%s", rc, "启动主程序Daemon失败！");
//    return NULL;
//  }
//
//  send_easy_trap(0, 0);

  int                         numfds;
  netsnmp_large_fd_set        readfds, writefds, exceptfds;
  struct timeval              timeout, *tvp = &timeout;
  int                         count, block, i;

  netsnmp_large_fd_set_init(&readfds, FD_SETSIZE);
  netsnmp_large_fd_set_init(&writefds, FD_SETSIZE);
  netsnmp_large_fd_set_init(&exceptfds, FD_SETSIZE);

RUNNING:
  while (_gw_snmp_daemon_running)
  {
    agent_check_and_process(1);
  }

  netsnmp_large_fd_set_cleanup(&readfds);
  netsnmp_large_fd_set_cleanup(&writefds);
  netsnmp_large_fd_set_cleanup(&exceptfds);

  SOCK_CLEANUP;

  gfc_log_info("%s", "代理监听程序退出！");
}

void
gw_snmpd_register()
{
  int rc = register_mib(app_name, (struct variable*) var_gw, sizeof(struct variable7), 7, oid_gw, OID_LENGTH(oid_gw));
  if (MIB_REGISTERED_OK != rc)
    gfc_log_error("%s(%d)", "MIB注册失败！", rc);
  else
    gfc_log_info("%s(%d)", "MIB注册成功！", rc);
}