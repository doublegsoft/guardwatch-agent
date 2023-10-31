#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include <time.h>
#include <unistd.h>

#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

u_char*
gw_snmp_access(struct variable *vp,
               oid * name,
               size_t * length,
               int exact, size_t *
               var_len,
               WriteMethod ** write_method);

static const char* app_name = "guardwatch";

// standard trap oid
oid oid_snmp_trap[]           = { 1, 3, 6, 1, 6, 3, 1, 1, 4, 1, 0};
oid oid_sys_uptime[]          = {1, 3, 6, 1, 2, 1, 1, 3, 0};

oid oid_gw[]                  = {1, 3, 6, 1, 4, 1, 80717, 1};

// heartbeat trap oid
oid oid_gw_heartbeat[]        = {1, 3, 6, 1, 4, 1, 80717, 1, 3, 1};

//
oid oid_gw_trap_address[]     = {1, 3, 6, 1, 4, 1, 80717, 1, 1, 1, 0};
oid oid_gw_agent_name[]       = {1, 3, 6, 1, 4, 1, 80717, 1, 1, 2, 0};
oid oid_gw_agent_location[]   = {1, 3, 6, 1, 4, 1, 80717, 1, 1, 3, 0};

struct variable7 var_gw[] = {
  {1, ASN_OBJECT_ID, NETSNMP_OLDAPI_RWRITE, gw_snmp_access, 1, {1}},
  {11, ASN_OCTET_STR, NETSNMP_OLDAPI_RWRITE, gw_snmp_access, 2, {1, 1}},
  {12, ASN_OCTET_STR, NETSNMP_OLDAPI_RWRITE, gw_snmp_access, 2, {1, 2}},
  {13, ASN_OCTET_STR, NETSNMP_OLDAPI_RWRITE, gw_snmp_access, 2, {1, 3}},
  {2, ASN_OBJECT_ID, NETSNMP_OLDAPI_RONLY, gw_snmp_access, 1, {2}},
  {3, ASN_OBJECT_ID, NETSNMP_OLDAPI_RONLY, gw_snmp_access, 1, {3}},
  {31, ASN_OBJECT_ID, NETSNMP_OLDAPI_RONLY, gw_snmp_access, 2, {3, 1}}
};

u_char*
gw_snmp_access(struct variable *vp,
               oid * name,
               size_t * length,
               int exact, size_t *
               var_len,
               WriteMethod ** write_method)
{
  return "";
}

int main()
{
  // the ip address
//  char hostbuffer[256];
//  struct hostent *host_entry;
//  int hostname;
//
//  hostname = gethostname(hostbuffer, sizeof(hostbuffer));
//  host_entry = gethostbyname(hostbuffer);
//
//  char* sys_name;
//  sys_name = inet_ntoa(*((struct in_addr*)host_entry->h_addr_list[0]));

  const char* sys_location = "二楼";
  const char* sys_descr = "自助机";

  char sys_uptime[24] = {'\0'};

  time_t rawtime;
  struct tm* tm;

  time (&rawtime);
  tm = localtime(&rawtime);
  sprintf(sys_uptime, "%d-%02d-%02d %02d:%02d:%02d",
          tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
          tm->tm_hour, tm->tm_min, tm->tm_sec);

  register_mib(app_name, var_gw, sizeof(struct variable7), 7, oid_gw, OID_LENGTH(oid_gw));

  printf("REGISTER_MIB\n");

  netsnmp_session*  ss;
  netsnmp_session   session;
  netsnmp_pdu    *pdu, *response;
  char *trap = NULL;

  char comm[] = "public";
  snmp_sess_init(&session);
  session.version = SNMP_VERSION_2c;
  session.community = comm;
  session.community_len = strlen(comm);
  session.peername = "10.211.55.2:162";

  SOCK_STARTUP;

  ss = snmp_add(&session,
                netsnmp_transport_open_client(app_name, session.peername),
                NULL, NULL);
  if (ss == NULL) {
    printf("error to connect to %s\n", session.peername);
    return -1;
  }
  pdu = snmp_pdu_create(SNMP_MSG_TRAP2);
  pdu->community = comm;
  pdu->community_len = strlen(comm);
  pdu->trap_type = SNMP_TRAP_ENTERPRISESPECIFIC;

  long sysuptime;
  char csysuptime [20];
  sysuptime = get_uptime ();
  sprintf (csysuptime, "%ld", sysuptime);
  trap = csysuptime;

  snmp_add_var(pdu, oid_sys_uptime, OID_LENGTH(oid_sys_uptime), 't', trap);
  snmp_add_var(pdu, oid_gw_heartbeat, OID_LENGTH(oid_gw_heartbeat), ASN_OBJECT_ID, (u_char*)oid_gw_heartbeat);
  snmp_add_var(pdu, oid_gw_agent_name, OID_LENGTH(oid_gw_agent_name), ASN_OCTET_STR, "this is the name");
  snmp_add_var(pdu, oid_gw_agent_location, OID_LENGTH(oid_gw_agent_location), ASN_OCTET_STR, "this is then location");

  send_trap_to_sess(ss, pdu);
  snmp_close(ss);

  SOCK_CLEANUP;
  return 0;
}