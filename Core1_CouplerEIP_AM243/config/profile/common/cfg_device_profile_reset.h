/*!
 *  \file cfg_device_profile_reset.h
 *
 *  \brief
 *  Configuration of factory reset parameters.
 *
 *  \author
 *  Texas Instruments Incorporated
 *
 *  \copyright
 *  Copyright (C) 2025 Texas Instruments Incorporated
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef CFG_DEVICE_PROFILE_RESET_H
#define CFG_DEVICE_PROFILE_RESET_H

/* Factory default non-volatile memory parameters definition */

/* Time Sync object (0x43) - Doc. Vol1_3.38 - Ch. 5B-2 */

/* PTP Enable (Attribute 1) - Ch. 5B-2.4 */
#if defined(EIP_TIME_SYNC) && (EIP_TIME_SYNC == 1)
#define CFG_DEVICE_PROFILE_RESET_FACTORY_DEFAULT_TIMESYNC_PTP_ENABLE                 1 /* PTP enabled (Default value - Ch. 5B-2.4.1 */
#else
#define CFG_DEVICE_PROFILE_RESET_FACTORY_DEFAULT_TIMESYNC_PTP_ENABLE                 0 /* PTP disabled (Default value - Ch. 5B-2.4.1 */
#endif

/* Port Enable (Attribute 13) - Ch. 5B-2.4 */
#define CFG_DEVICE_PROFILE_RESET_FACTORY_DEFAULT_TIMESYNC_PORT_ENABLE                1 /* Port is enabled (Default value - Ch. 5B-2.4.13 */

/* Port Log Announce Interval Configuration (Attribute 14) - Ch. 5B-2.4 */
#define CFG_DEVICE_PROFILE_RESET_FACTORY_DEFAULT_TIMESYNC_LOG_ANNOUNCE_INTERVAL      1 /* Log Announce Interval (Default value - Ch. 5B-2.16.2 */

/* Port Log Sync Interval Configuration (Attribute 15) - Ch. 5B-2.4 */
#define CFG_DEVICE_PROFILE_RESET_FACTORY_DEFAULT_TIMESYNC_LOG_SYNC_INTERVAL          0 /* Log Sync Interval (Default value - Ch. 5B-2.16.2 */

/* Domain Number (Attribute 18) - Ch. 5B-2.4 */
#define CFG_DEVICE_PROFILE_RESET_FACTORY_DEFAULT_TIMESYNC_DOMAIN_NUMBER              0 /* Default value - Ch. 5B-2.16.2 */

/* User Description (Attribute 23) - Ch. 5B-2.4 */
#define CFG_DEVICE_PROFILE_RESET_FACTORY_DEFAULT_TIMESYNC_USER_DESCRIPTION           "Texas Instruments;EthernetIP Adapter;" /* Is volatile, but can be adjust here */

/* Quality of Service object (0x48) - Doc. Vol2_1.34 - Ch. 5-7 */

/* 802.1Q Tag Enable (Attribute 1) - Ch. 5-7.4 */
#define CFG_DEVICE_PROFILE_RESET_FACTORY_DEFAULT_QOS_802_1_TAG_ENABLE                0 /* Default value - Ch. 5-7.4.1 */

/* DSCP PTP Event (Attribute 2) - Ch. 5-7.4 */
#define CFG_DEVICE_PROFILE_RESET_FACTORY_DEFAULT_QOS_DSCP_PTP_EVENT                  59 /* Default value - Ch. 5-7.4.2 */

/* DSCP PTP General (Attribute 3) - Ch. 5-7.4 */
#define CFG_DEVICE_PROFILE_RESET_FACTORY_DEFAULT_QOS_DSCP_PTP_GENERAL                47 /* Default value - Ch. 5-7.4.2 */

/* DSCP Urgent (Attribute 4) - Ch. 5-7.4 */
#define CFG_DEVICE_PROFILE_RESET_FACTORY_DEFAULT_QOS_DSCP_PTP_URGENT                 55 /* Default value - Ch. 5-7.4.2 */

/* DSCP Scheduled (Attribute 5) - Ch. 5-7.4 */
#define CFG_DEVICE_PROFILE_RESET_FACTORY_DEFAULT_QOS_DSCP_PTP_SCHEDULED              47 /* Default value - Ch. 5-7.4.2 */

/* DSCP High (Attribute 6) - Ch. 5-7.4 */
#define CFG_DEVICE_PROFILE_RESET_FACTORY_DEFAULT_QOS_DSCP_HIGH                       43 /* Default value - Ch. 5-7.4.2 */

/* DSCP Low (Attribute 7) - Ch. 5-7.4 */
#define CFG_DEVICE_PROFILE_RESET_FACTORY_DEFAULT_QOS_DSCP_LOW                        31 /* Default value - Ch. 5-7.4.2 */

/* DSCP Explicit (Attribute 8) - Ch. 5-7.4 */
#define CFG_DEVICE_PROFILE_RESET_FACTORY_DEFAULT_QOS_DSCP_EXPLICIT                   27 /* Default value - Ch. 5-7.4.2 */


/* TCP/IP object (0xF5) - Doc. Vol2_1.34 - Ch. 5-4 */

/* Configuration Control (Attribute 3) - Ch. 5-4.3.2 */
#define CFG_DEVICE_PROFILE_RESET_FACTORY_DEFAULT_TCPIP_CFG_CONTROL                   EIP_eCFGMETHOD_STATIC  /* Statically-assigned IP address - Ch. 5-4.3.2.3.1 */

/* Interface Configuration (Attribute 5) - Ch. 5-4.3.2 */
#define CFG_DEVICE_PROFILE_RESET_FACTORY_DEFAULT_TCPIP_IP_ADDRESS                    0xc0a8010a    /* 192.168.1.10 */
#define CFG_DEVICE_PROFILE_RESET_FACTORY_DEFAULT_TCPIP_NW_MASK                       0xffffff00    /* 255.255.255.0 */
#define CFG_DEVICE_PROFILE_RESET_FACTORY_DEFAULT_TCPIP_GW_ADDRESS                    0xc0a80101    /* 192.168.1.1 */
#define CFG_DEVICE_PROFILE_RESET_FACTORY_DEFAULT_TCPIP_SERVER_NAME_1                 0x00000000    /* 0.0.0.0 */
#define CFG_DEVICE_PROFILE_RESET_FACTORY_DEFAULT_TCPIP_SERVER_NAME_2                 0x00000000    /* 0.0.0.0 */
#define CFG_DEVICE_PROFILE_RESET_FACTORY_DEFAULT_TCPIP_DOMAIN_NAME                   ""            /* Empty string */

/* Host Name (Attribute 6) - Ch. 5-4.3.2 */
#define CFG_DEVICE_PROFILE_RESET_FACTORY_DEFAULT_TCPIP_HOST_NAME                     ""            /* Empty string */

/* TTL Value (Attribute 8) - Ch. 5-4.3.2 */
#define CFG_DEVICE_PROFILE_RESET_FACTORY_DEFAULT_TCPIP_TTL_VALUE                     1             /* Default value - Ch. 5-4.3.2.7 */

/* Mcast Config (Attribute 9)- Ch. 5-4.3.2 */
#define CFG_DEVICE_PROFILE_RESET_FACTORY_DEFAULT_TCPIP_MCAST_ALLOC_CONTROL           0             /* Default allocation algorithm is used - Ch. 5-4.3.2.8 */
#define CFG_DEVICE_PROFILE_RESET_FACTORY_DEFAULT_TCPIP_MCAST_RESERVED                0             /* Default value - Ch. 5-4.3.2.8 */
#define CFG_DEVICE_PROFILE_RESET_FACTORY_DEFAULT_TCPIP_MCAST_NUM_CAST                0             /* Default value - Ch. 5-4.3.2.8 */
#define CFG_DEVICE_PROFILE_RESET_FACTORY_DEFAULT_TCPIP_MCAST_START_ADDRESS           0x00000000    /* Default value - Ch. 5-4.3.2.8 */

/* Select ACD (Attribute 10) - Ch. 5-4.3.2 */
#define CFG_DEVICE_PROFILE_RESET_FACTORY_DEFAULT_TCPIP_ACD_ENABLE                    1             /* Default value - Ch. 5-4.3.2.9 */

/* Select ACD (Attribute 11) - Ch. 5-4.3.2 */
#define CFG_DEVICE_PROFILE_RESET_FACTORY_DEFAULT_TCPIP_ACD_STATE                     0             /* Set to "No Conflict Detected" per default - Ch. 5-4.3.2.10 */
#define CFG_DEVICE_PROFILE_RESET_FACTORY_DEFAULT_TCPIP_ACD_REMOTE_MAC                0x00          /* 00:00:00:00:00:00 */
#define CFG_DEVICE_PROFILE_RESET_FACTORY_DEFAULT_TCPIP_ACD_ARP_PDU                   0x00          /* 00 00 00 00 00 00 ... */

/* EtherNet/IP Quick Connect (Attribute 12) - Ch. 5-4.3.2 */
#if defined(EIP_QUICK_CONNECT) && (EIP_QUICK_CONNECT == 1)
#define CFG_DEVICE_PROFILE_RESET_FACTORY_DEFAULT_TCPIP_QUICK_CONNECT_ENABLE          1             /* Set when Quick Connect feature is activated */
#else
#define CFG_DEVICE_PROFILE_RESET_FACTORY_DEFAULT_TCPIP_QUICK_CONNECT_ENABLE          0             /* Default value - Ch. 5-4.3.2.11 */
#endif

/* Encapsulation Inactivity Timeout (Attribute 13) - Ch. 5-4.3.2 */
#define CFG_DEVICE_PROFILE_RESET_FACTORY_DEFAULT_TCPIP_ENCAP_TIMEOUT                 120           /* Default value - Ch. 5-4.3.2 */

/* EtherNet Link object (0xF6) - Doc. Vol2_1.34 - Ch. 5-5 */

/* Interface Control (Attribute 6) - Ch. 5-5.3.2 */
#define CFG_DEVICE_PROFILE_RESET_FACTORY_DEFAULT_ETHLINK_CONTROL_BITS_AUTONEG        1             /* Auto-Negotiate is enabled - Ch. 5-5.3.2.6.1 */
#define CFG_DEVICE_PROFILE_RESET_FACTORY_DEFAULT_ETHLINK_CONTROL_BITS_FORCED_DUPLEX  0             /* Not relevant when Auto-Negotiate is enabled */
#define CFG_DEVICE_PROFILE_RESET_FACTORY_DEFAULT_ETHLINK_FORCED_SPEED                0             /* Not relevant when Auto-Negotiate is enabled */

/* Admin state (Attribute 9) - Ch. 5-5.3.2 */
#define CFG_DEVICE_PROFILE_RESET_FACTORY_DEFAULT_ETHLINK_ADMIN_STATE_ENABLE          1             // Enable the interface - Ch. 5-5.3.2.9


/* LLDP Management Object (0x109) - Doc. Vol2_1.34 - Ch. 5-15 */

/* LLDP Enable Array Length (Attribute 1) - Ch. 5-15.3.2 */
#define CFG_DEVICE_PROFILE_RESET_FACTORY_DEFAULT_LLDP_ENABLE_ARRAY_LENGTH            3             /* Number of bits defined in the LLDP Enable Array (Global + Port 1 + Port 2) */
#define CFG_DEVICE_PROFILE_RESET_FACTORY_DEFAULT_LLDP_ENABLE_ARRAY_BITS              7             /* Enable generation of LLDP Frames both Globally and per Port - Ch. 5-15.3.3.1 */

/* LLDP Message TX Interval (Attribute 2) */
#define CFG_DEVICE_PROFILE_RESET_FACTORY_DEFAULT_LLDP_MSG_TX_INTERVAL                30            /* Recommended value - Ch. 5-15.3.2 */

/* LLDP Message TX Hold (Attribute 3) */
#define CFG_DEVICE_PROFILE_RESET_FACTORY_DEFAULT_LLDP_MSG_TX_HOLD                    4             /* Recommended value - Ch. 5-15.3.2 */

/* Device Level Ring (DLR) Object (0x47) - Doc. Vol2_1.34 - Ch. 5-6 */

#ifdef SDK_VARIANT_PREMIUM
/* DLR Ring Supervisor Config (Attribute 4) - Ch. 5-6.3.6 */
#define CFG_DEVICE_PROFILE_RESET_FACTORY_DEFAULT_DLR_RING_SUPERVISOR_ENABLE          0             /* Default value - Ch. 5-6.3.6.1 */
#define CFG_DEVICE_PROFILE_RESET_FACTORY_DEFAULT_DLR_RING_SUPERVISOR_PRECEDENCE      0             /* Default value - Ch. 5-6.3.6.2 */
#define CFG_DEVICE_PROFILE_RESET_FACTORY_DEFAULT_BEACON_INTERVAL                     400           /* Default value in microseconds - Ch. 5-6.3.6.3 */
#define CFG_DEVICE_PROFILE_RESET_FACTORY_DEFAULT_BEACON_TIMEOUT                      1960          /* Default value in microseconds - Ch. 5-6.3.6.4 */
#define CFG_DEVICE_PROFILE_RESET_FACTORY_DEFAULT_DLR_VLAN_ID                         0             /* Default value - Ch. 5-6.3.6.5 */
#endif


#ifdef __cplusplus
extern "C" {
#endif

#ifdef  __cplusplus
}
#endif

#endif  // CFG_DEVICE_PROFILE_RESET_H
