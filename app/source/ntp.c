#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pspkernel.h>
#include <pspnet_inet.h>
#include <pspnet_resolver.h>
#include <stdio.h>
#include <string.h>

#include "ntp.h"
#include "utils.h"

#define NTP_TIMESTAMP_DELTA 2208988800ull
#define MAX_NAME 512

// Function defs
int sceRtcSetTime64_t(pspTime *date, const time_t time);
int pspRtcSetCurrentTick(u64 *tick); // Kernel function

// ntp_packet structure from lettier/ntpclient https://github.com/lettier/ntpclient/blob/master/source/c/main.c#L50
typedef struct {
    u8 li_vn_mode;      // Eight bits. li, vn, and mode.
                        // li.   Two bits.   Leap indicator.
                        // vn.   Three bits. Version number of the protocol.
                        // mode. Three bits. Client will pick mode 3 for client.
    
    u8 stratum;         // Eight bits. Stratum level of the local clock.
    u8 poll;            // Eight bits. Maximum interval between successive messages.
    u8 precision;       // Eight bits. Precision of the local clock.
    
    u32 rootDelay;      // 32 bits. Total round trip delay time.
    u32 rootDispersion; // 32 bits. Max error aloud from primary clock source.
    u32 refId;          // 32 bits. Reference clock identifier.
    
    u32 refTm_s;        // 32 bits. Reference time-stamp seconds.
    u32 refTm_f;        // 32 bits. Reference time-stamp fraction of a second.
    
    u32 origTm_s;       // 32 bits. Originate time-stamp seconds.
    u32 origTm_f;       // 32 bits. Originate time-stamp fraction of a second.
    
    u32 rxTm_s;         // 32 bits. Received time-stamp seconds.
    u32 rxTm_f;         // 32 bits. Received time-stamp fraction of a second.
    
    u32 txTm_s;         // 32 bits and the most important field the client cares about. Transmit time-stamp seconds.
    u32 txTm_f;         // 32 bits. Transmit time-stamp fraction of a second.
} ntp_packet;           // Total: 384 bits or 48 bytes.

static __inline__ unsigned int sceAllegrexWsbw(unsigned int x) {
    return (((x & 0xFF)<<24) | ((x & 0xFF00)<<8) | ((x>>8) & 0xFF00) | ((x>>24) & 0xFF));
}

static __inline__ unsigned int sceAllegrexWsbh(unsigned int x) {
    return (((x<<8) & 0xFF00FF00) | ((x>>8) & 0x00FF00FF));
}

static inline u32 sceNetHtonl(u32 hostlong) {
    return sceAllegrexWsbw(hostlong);
}

static inline u16 sceNetHtons(u16 hostshort) {
    return sceAllegrexWsbh(hostshort);
}

static inline u32 sceNetNtohl(u32 hostlong) {
    return sceAllegrexWsbw(hostlong);
}

// https://github.com/pspdev/pspsdk/blob/a095fcaa1f5ae28ddcae599d50a69c3a15ac1d34/src/libcglue/netdb.c#L75
static struct hostent *sceGetHostByName(const char *name, int *h_errno) {
    static struct hostent ent;
    char buf[1024];
    static char sname[MAX_NAME] = "";
    static struct in_addr saddr = { 0 };
    static char *addrlist[2] = { (char *) &saddr, NULL };
    int rid;
    *h_errno = NETDB_SUCCESS;
    
    if (sceNetInetInetAton(name, &saddr) == 0) {
        int err;
        
        if (sceNetResolverCreate(&rid, buf, sizeof(buf)) < 0) {
            *h_errno = NO_RECOVERY;
            return NULL;
        }
        
        err = sceNetResolverStartNtoA(rid, name, &saddr, 2, 3);
        sceNetResolverDelete(rid);
        
        if (err < 0) {
            *h_errno = HOST_NOT_FOUND;
            return NULL;
        }
    }
    
    snprintf(sname, MAX_NAME, "%s", name);
    ent.h_name = sname;
    ent.h_aliases = 0;
    ent.h_addrtype = AF_INET;
    ent.h_length = sizeof(struct in_addr);
    ent.h_addr_list = addrlist;
    ent.h_addr = (char *) &saddr;
    return &ent;
}

int ntpGetTime(pspTime *psp_time_ntp) {
    int ret = 0;
    const char *server_name = "0.pool.ntp.org";
    const u16 port = 123;

    u64 curr_tick = 0;
    if (R_FAILED(ret = sceRtcGetCurrentTick(&curr_tick))) {
        snprintf(g_err_string, 64, "sceRtcGetCurrentTick() failed: 0x%08x\n", ret);
        return ret;
    }

    pspTime curr_psp_time;
    memset(&curr_psp_time, 0, sizeof(pspTime));
    if (R_FAILED(ret = sceRtcSetTick(&curr_psp_time, &curr_tick))) {
        snprintf(g_err_string, 64, "sceRtcSetTick() failed: 0x%08x\n", ret);
        return ret;
    }
    
    time_t curr_time;
    if (R_FAILED(ret = sceRtcGetTime_t(&curr_psp_time, &curr_time))) {
        snprintf(g_err_string, 64, "sceRtcGetTime_t() failed: 0x%08x\n", ret);
        return ret;
    }

    ntp_packet packet;
    memset(&packet, 0, sizeof(ntp_packet));
    packet.li_vn_mode = (0 << 6) | (4 << 3) | 3;  // LI 0 | Client version 4 | Mode 3
    packet.txTm_s = sceNetHtonl(NTP_TIMESTAMP_DELTA + curr_time);  // Current network time on the console
    
    struct sockaddr_in serv_addr;
    struct hostent *server;

    int sockfd = sceNetInetSocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (R_FAILED(sockfd)) {
        snprintf(g_err_string, 64, "sceNetInetSocket() failed: 0x%08x\n", ret);
        sceNetInetClose(sockfd);
        return sockfd;
    }

    int err = 0;
    if ((server = sceGetHostByName(server_name, &err)) == NULL) {
        snprintf(g_err_string, 64, "sceGetHostByName() failed: 0x%08x\n", err);
        sceNetInetClose(sockfd);
        return err;
    }
    
    memset(&serv_addr, 0, sizeof(struct sockaddr_in));
    serv_addr.sin_family = AF_INET;
    memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr_list[0], 4);
    serv_addr.sin_port = sceNetHtons(port);
    
    if ((ret = sceNetInetConnect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0) {
        snprintf(g_err_string, 64, "sceNetInetConnect() failed: 0x%08x\n", err);
        sceNetInetClose(sockfd);
        return -1;
    }

    if ((ret = sceNetInetSend(sockfd, (char *)&packet, sizeof(ntp_packet), 0)) < 0) {
        snprintf(g_err_string, 64, "sceNetInetSend() failed: 0x%08x\n", err);
        sceNetInetClose(sockfd);
        return -1;
    }

    if ((ret = sceNetInetRecv(sockfd, (char *)&packet, sizeof(ntp_packet), 0)) < (int)sizeof(ntp_packet)) {
        snprintf(g_err_string, 64, "sceNetInetRecv() failed: 0x%08x\n", err);
        sceNetInetClose(sockfd);
        return -1;
    }
    
    packet.txTm_s = sceNetNtohl(packet.txTm_s);
    time_t time_next = (time_t)(packet.txTm_s - NTP_TIMESTAMP_DELTA);

    pspTime psp_time_next;
    memset(&psp_time_next, 0, sizeof(pspTime));
    sceRtcSetTime64_t(&psp_time_next, time_next);

    u64 tick_next = 0, utc_tick = 0;
    if (R_FAILED(ret = sceRtcGetTick(&psp_time_next, &tick_next))) {
        snprintf(g_err_string, 64, "sceRtcGetTick() failed: 0x%08x\n", err);
        return ret;
    }
    
    if (R_FAILED(ret = sceRtcConvertLocalTimeToUTC(&tick_next, &utc_tick))) {
        snprintf(g_err_string, 64, "sceRtcConvertLocalTimeToUTC() failed: 0x%08x\n", err);
        return ret;
    }

    if (R_FAILED(ret = pspRtcSetCurrentTick(&tick_next))) {
        snprintf(g_err_string, 64, "pspRtcSetCurrentTick() failed: 0x%08x\n", err);
        return ret;
    }
    
    memset(&psp_time_next, 0, sizeof(pspTime));
    sceRtcSetTick(&psp_time_next, &tick_next);
    sceNetInetClose(sockfd);
    *psp_time_ntp = psp_time_next;
    return 0;
}
