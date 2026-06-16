/*!
 *  \file web_server.c
 *
 *  \brief
 *  Application Web Server task
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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "kernel/dpl/TaskP.h"
#include "kernel/dpl/SystemP.h"
#include "osal.h"
#include "osal_error.h"
#include "web_server.h"
#include "gspws_html.h"
#include "gspws_js.h"
#include "lwip/sockets.h"
#include "lwip/errno.h"

#include "cmn_cpu_api.h"
#include "FWVER.h"
#include "ipc_shareMem.h"

/*!
 *  \brief Application Web server task's stack size.
 */
#define WEB_SERVER_TASK_STACK_SIZE     3072U

#define MAX_HTML_SIZE 8192
/* Extern */
extern volatile ipc_data_t gSharedMem;
extern const unsigned char gsp_favicon_ico[];
extern const unsigned int gsp_favicon_ico_len;
extern int generate_io_table(char *html_out, int max_size, IOCoupler_Device *dev);
extern int generate_io_json(char *json_out, int max_size, IOCoupler_Device *dev);

/*!
 *  \brief Application Web server task's stack.
 */
static uint8_t
WEB_SERVER_taskStack_g[WEB_SERVER_TASK_STACK_SIZE] __attribute__((aligned(32), section(".threadstack"))) = {0};

static char jsonBuf[MAX_HTML_SIZE];
static char header[256];

/*!
 *  \brief Application Web server task's object.
 */
static TaskP_Object WEB_SERVER_taskObj_g = { 0 };

static int send_SVG(int fd, const char *svg_data, size_t svg_len)
{
    memset(header, 0, sizeof(header));
    
    int hlen = snprintf(header,
                        sizeof(header),
                        "HTTP/1.1 200 OK\r\n"
                        "Content-Type: image/svg+xml\r\n"
                        "Content-Length: %u\r\n"
                        "Connection: close\r\n"
                        "\r\n",
                        (unsigned int)svg_len);

    int ret = send(fd, header, hlen, 0);

    if(ret < 0)
    {
        DebugP_log("send_SVG Header send failed %d\r\n", ret);
        return -1;
    }

    size_t offset = 0;

    while (offset < svg_len)
    {
        int sent = send(fd,
                        (const char *)svg_data + offset,
                        svg_len - offset > 1024 ? 1024 : (svg_len - offset),
                        0);
        // DebugP_log("Sent SVG len=%u\r\n", (unsigned)sent);
        if (sent < 0){
            DebugP_log("SVG send failed %d\r\n", ret);
            return -1;
        }

        if (sent == 0)
            continue;

        offset += sent;
    }

    return (int)offset;
}

/*!
*  Function: WEB_SERVER_strError
*
*  \brief
*  Helper function to map errno to a string.
*
*  \details
*  Helper function to map errno to a string. Only the relevant
*  errno for sockets are mapped.
*
*  \param[inout]     err_p         errno set by the lwip stack.
*
*  \return           result of the operation as character string.
*
*/
static char *WEB_SERVER_strError(int err_p)
{
    switch (err_p)
    {
    case 0:  return ("No error");
    case EACCES:  return ("Permission denied");
    case EPERM:  return ("Permission denied");
    case EADDRINUSE:  return ("Local Address already in use");
    case EADDRNOTAVAIL:  return ("Address not available");
    case EAFNOSUPPORT:  return ("Incorrect address family address");
    case EAGAIN:  return ("Operation would block, Try Again");
    case EALREADY:  return ("Previous operation still in progress");
    case EBADF:  return ("Not a valid file descriptor");
    case ECONNREFUSED:  return ("Connection refused");
    case EFAULT:  return ("Socket structure address is not valid");
    case EINPROGRESS:  return ("Previous operation is now in progress");
    case EINTR:  return ("Operation interrupted by signal");
    case EISCONN:  return ("Socket already in use");
    case ENETUNREACH:  return ("Network is unreachable");
    case ENOTSOCK:  return ("Given file descriptor is not a socket");
    case EPROTOTYPE:  return ("Socket does not support the comms protocol");
    case ETIMEDOUT:  return ("Timeout while attempting connection");
    case ELOOP:  return ("Too many symbolic links were encountered");
    case ENAMETOOLONG:  return ("File name too long");
    case ENOENT:  return ("Socket pathname does not exist");
    case ENOMEM:  return ("Insufficient kernel memory");
    case ENOTDIR:  return ("Path prefix is not a directory");
    case EROFS:  return ("Socket inode in a read-only filesystem");
    case EOPNOTSUPP:  return ("Operation not supported");
    case EPROTO:  return ("Socket does not support the comms protocol");
    case ECONNABORTED:  return ("Connection aborted");
    case ECONNRESET:  return ("Connection reset by peer");
    case ENOTCONN:  return ("Transport endpoint not connected");
    }

    return ("Unknown error");
}


/*!
*  Function: WEB_SERVER_isFatalErr
*
*  \brief
*  Helper function to check if errno is fatal.
*
*  \details
*  Helper function to check if errno is fatal.
*  Non-Fatal are :
*  EAGAIN
*  EWOULDBLOCK
*  ECONNABORTED
*  EINTR
*  ECONNRESET
*
*  \param[inout]     errNo_p    errno set by the lwip stack.
*
*  \return           result of the operation as bool
*  \retval           true          Error is fatal
*  \retval           false         Error is not fatal
*
*/
static bool WEB_SERVER_isFatalErr(int errNo_p)
{
    bool isFatal = true;
    if ((errno == EAGAIN) ||
        (errno == EWOULDBLOCK) ||
        (errno == ECONNABORTED) ||
        (errno == EINTR) ||
        (errno == ECONNRESET))
    {
        isFatal = false;
    }
    return isFatal;
}

/*!
*  Function: WEB_SERVER_init
*
*  \brief
*  Helper function to initialize the http socket.
*
*  \details
*  Helper function to initialize the http socket.
*  If an operation fails the reason for the failure is read through errno.
*
*
*  \param[inout]     pSockFd_p   Pointer to location where the socket file descriptor will be stored.
*
*  \return           result of the operation as int
*  \retval            0          Operation succeeded
*  \retval           -1          Operation failed
*
*/
static int WEB_SERVER_init(int *pSockFd_p)
{
    int sock = -1;
    int ret = -1;
    struct sockaddr_in addr = { 0 };
    addr.sin_family = AF_INET;
    addr.sin_port = PP_HTONS(80);
    addr.sin_addr.s_addr = PP_HTONL(INADDR_ANY);

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0)
    {
        DebugP_log("Failed to open socket : %d : %s \r\n", -errno,
                   WEB_SERVER_strError(errno));
    }
    else
    {
        ret = bind(sock, (struct sockaddr*)&addr, sizeof(addr));

        if (ret == 0)
        {
            // ret = listen(sock, 3);

            // Enable immediate address reuse to clean up dead bindings quickly
            int reuse = 1;
            setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
            ret = listen(sock, 8); // Increased queue
            if (ret == 0)
            {
                *pSockFd_p = sock;
            }
        }

        if (ret < 0)
        {
            DebugP_log("Failed to bind / listen to socket : %d : %s \r\n",
                       -errno, WEB_SERVER_strError(errno));
        }
    }

    return ret;
}

/*!
*  Function: WEB_SERVER_processGetAndRespond
*
*  \brief
*  Function to process the received http GET request and respond to it.
*
*  \details
*  Function to process the received http GET request and respond to it.
*  At the moment it only responds to the following get requests :
*  "/"
*  "/main.html"
*  "/main.js"
*  "/favicon.ico
*  "/cpuLoad"
*  A 404 error will be returned for any other request.
*
*  \param[in]        clientFd_p   Client file descriptor for sending response.
*  \param[in]        pBuf_p       Pointer to received data (with "GET " request removed).
*
*  \return           result of the operation as int
*  \retval            > 0          Operation succeeded
*  \retval            < 0          Operation failed
*
*/
static int WEB_SERVER_processGetAndRespond(int clientFd_p, const char *const pBuf_p)
{
    int ret = -1;

    CMN_CPU_API_SData_t* data = CMN_CPU_API_getData();

    if ((strncmp(&pBuf_p[0], "/ HTTP/1.1\r\n", 12) == 0) ||
        (strncmp(&pBuf_p[0], "/main.html HTTP/1.1\r\n", 21) == 0))
    {
        ret = send(clientFd_p, response_200_content_html, strlen(response_200_content_html), 0);
        
        ret |= send(clientFd_p, html_top, strlen(html_top), 0);
        
        memset(jsonBuf, 0, sizeof(jsonBuf));
        sprintf(jsonBuf, html_top_body, "getCpuLoad()", "active", "", "");
        ret |= send(clientFd_p, jsonBuf, strlen(jsonBuf), 0);
        
        ret |= send(clientFd_p, page_main, strlen(page_main), 0);
        ret |= send(clientFd_p, html_bottom, strlen(html_bottom), 0);
    }
    else if ((strncmp(&pBuf_p[0], "/IO_Mapping.html HTTP/1.1\r\n", 27) == 0))
    {
        ret = send(clientFd_p, response_200_content_html, strlen(response_200_content_html), 0);
        
        ret |= send(clientFd_p, html_top_iopage, strlen(html_top_iopage), 0);

        memset(jsonBuf, 0, sizeof(jsonBuf));
        sprintf(jsonBuf, html_top_body, "", "", "active", "");
        ret |= send(clientFd_p, jsonBuf, strlen(jsonBuf), 0);

        memset(jsonBuf, 0, sizeof(jsonBuf));
        sprintf(jsonBuf, page_iomap, INDS_COMM_TYPE, INDS_COMM_FW_VERSION);
        ret |= send(clientFd_p, jsonBuf, strlen(jsonBuf), 0);
        
        ret |= send(clientFd_p, html_bottom, strlen(html_bottom), 0);
    }
    else if ((strncmp(&pBuf_p[0], "/Network.html HTTP/1.1\r\n", 21) == 0))
    {
        ret = send(clientFd_p, response_200_content_html, strlen(response_200_content_html), 0);
        
        ret |= send(clientFd_p, html_top, strlen(html_top), 0);

        memset(jsonBuf, 0, sizeof(jsonBuf));
        sprintf(jsonBuf, html_top_body, "", "", "", "active");
        ret |= send(clientFd_p, jsonBuf, strlen(jsonBuf), 0);
        
        ret |= send(clientFd_p, page_network, strlen(page_network), 0);
        ret |= send(clientFd_p, html_bottom, strlen(html_bottom), 0);
    }
    else if (strncmp(pBuf_p, "/style.css HTTP/1.1\r\n", 21) == 0)
    {
        ret = send(clientFd_p, response_200_content_css, strlen(response_200_content_css), 0);
        ret |= send(clientFd_p, style_css, strlen(style_css), 0);
    }
    else if (strncmp(pBuf_p, "/iomap.css HTTP/1.1\r\n", 21) == 0)
    {
        ret = send(clientFd_p, response_200_content_css, strlen(response_200_content_css), 0);
        ret |= send(clientFd_p, style_iomap_css, strlen(style_iomap_css), 0);
    }
    else if ((strncmp(&pBuf_p[0], "/main.js HTTP/1.1\r\n", 19) == 0))
    {
        // get request for main.js
        ret = send(clientFd_p, response_200_content_js, strlen(response_200_content_js), 0);

        if (ret > 0)
        {
            ret = send(clientFd_p, main_js, strlen(main_js), 0);
        }
    }
    else if ((strncmp(&pBuf_p[0], "/iomap.js HTTP/1.1\r\n", 20) == 0))
    {
        ret = send(clientFd_p, response_200_content_js, strlen(response_200_content_js), 0);

        if (ret > 0)
        {
            ret = send(clientFd_p, iomap_js, strlen(iomap_js), 0);
        }
    }
    else if ((strncmp(&pBuf_p[0], "/network.js HTTP/1.1\r\n", 19) == 0))
    {
        ret = send(clientFd_p, response_200_content_js, strlen(response_200_content_js), 0);

        if (ret > 0)
        {
            ret = send(clientFd_p, main_js, strlen(main_js), 0);
        }
    }
    else if ((strncmp(&pBuf_p[0], "/favicon.ico HTTP/1.1\r\n", 23) == 0))
    {
        // get request for favicon.ico
        ret = send(clientFd_p, response_200_content_image, strlen(response_200_content_image), 0);

        if (ret > 0)
        {
            ret = send(clientFd_p, gsp_favicon_ico, gsp_favicon_ico_len, 0);
        }
    }
    else if (strncmp(pBuf_p, "/iomap.svg HTTP/1.1", strlen("/iomap.svg HTTP/1.1")) == 0)
    {
        int svg_len = strlen(svg_io_template);
        ret = send_SVG(clientFd_p, svg_io_template, svg_len);
    }
    else if (strncmp(pBuf_p, "/plcmap.svg HTTP/1.1", strlen("/plcmap.svg HTTP/1.1")) == 0)
    {
        int svg_len = strlen(svg_plc_template);
        ret = send_SVG(clientFd_p, svg_plc_template, svg_len);
    }
    else if ((strncmp(&pBuf_p[0], "/logo.png HTTP/1.1\r\n", 16) == 0))
    {
        ret = 1;
        // char header[128];

        // sprintf(header,
        //     "HTTP/1.1 200 OK\r\n"
        //     "Content-Type: image/png\r\n"
        //     "Content-Length: %u\r\n"
        //     "Connection: close\r\n"
        //     "\r\n",
        //     (unsigned int)GespantLogo_len);
        
        // ret = send(clientFd_p, header, strlen(header), 0);

        // if (ret > 0){
        //     int totalSent = 0;
        //     while (totalSent < GespantLogo_len)
        //     {
        //         int sent = send(clientFd_p, GespantLogo + totalSent,
        //                         GespantLogo_len - totalSent, 0);

        //         if (sent <= 0)
        //             break;

        //         totalSent += sent;
        //     }
        // }
        // if (ret > 0)
        // {
        //     ret = send(clientFd_p, GespantLogo, GespantLogo_len, 0);
        // }
    }
    else if (strncmp(&pBuf_p[0], "/cpuLoad", 8) == 0)
    {
        memset(jsonBuf, 0, sizeof(jsonBuf));
        int textSize = 0;
        int remaining = (int)sizeof(jsonBuf);
        int n;

    #define JAPPEND(...) \
        do { \
            n = snprintf(&jsonBuf[textSize], (size_t)remaining, __VA_ARGS__); \
            if (n > 0 && n < remaining) { textSize += n; remaining -= n; } \
        } while(0)

        /* ── Move IDLE task to index 0 ───────────────────────────── */
        int idIdle = -1;
        for (int i = 0; i < data->tasksNum; i++)
        {
            if (strncmp(data->tasks[i].name, "IDLE", 32) == 0)
            {
                idIdle = i;
                break;
            }
        }

        if (idIdle > 0)
        {
            CMN_CPU_API_SLoad_t tmp = data->tasks[0];
            data->tasks[0]          = data->tasks[idIdle];
            data->tasks[idIdle]     = tmp;
        }

        /* ── CPU: aggregate + per-task rows ──────────────────────── */
        JAPPEND("%s,%2d.%02d %%\n",
                data->cpu.name,
                data->cpu.cpuLoad / 100,
                data->cpu.cpuLoad % 100);

        for (int i = 0; i < data->tasksNum; i++)
        {
            JAPPEND("%s,%2d.%02d %%\n",
                    data->tasks[i].name,
                    data->tasks[i].cpuLoad / 100,
                    data->tasks[i].cpuLoad % 100);
        }

        /* ── MCAN stats separator + error counters ───────────────── */
        app_ipc_sharemem_lock();
        const core_mcan_stats_t *mcan = &gSharedMem.ipc_sys.core0_mcan;
        app_ipc_sharemem_unlock();

        JAPPEND("---MCAN---\n");

        /* Error counter block */
        JAPPEND("REC,%lu\n",  (unsigned long)mcan->recErrCnt);
        JAPPEND("TEC,%lu\n",  (unsigned long)mcan->transErrLogCnt);
        JAPPEND("CEL,%lu\n",  (unsigned long)mcan->canErrLogCnt);
        JAPPEND("RP,%lu\n",   (unsigned long)mcan->rpStatus);

        /* Protocol status block */
        JAPPEND("CPU,%lu\n",  (unsigned long)mcan->cpu_load);
        JAPPEND("HB,%lu\n",  (unsigned long)mcan->heartbeat);
        JAPPEND("LEC,%lu\n",  (unsigned long)mcan->ProtocolStatus_lastErrCode);
        JAPPEND("ACT,%lu\n",  (unsigned long)mcan->ProtocolStatus_act);
        JAPPEND("DLEC,%lu\n", (unsigned long)mcan->ProtocolStatus_dlec);
        JAPPEND("TDCV,%lu\n", (unsigned long)mcan->ProtocolStatus_tdcv);

        /* Bus state flags */
        JAPPEND("EP,%lu\n",   (unsigned long)mcan->ProtocolStatus_errPassive);
        JAPPEND("WARN,%lu\n", (unsigned long)mcan->ProtocolStatus_warningStatus);
        JAPPEND("BOFF,%lu\n", (unsigned long)mcan->ProtocolStatus_busOffStatus);
        JAPPEND("RESI,%lu\n", (unsigned long)mcan->ProtocolStatus_resi);
        JAPPEND("RBRS,%lu\n", (unsigned long)mcan->ProtocolStatus_rbrs);
        JAPPEND("RFDF,%lu\n", (unsigned long)mcan->ProtocolStatus_rfdf);
        JAPPEND("PXE,%lu\n",  (unsigned long)mcan->ProtocolStatus_pxe);

    #undef JAPPEND

        if (textSize > 0)
        {
            ret = send(clientFd_p, jsonBuf, (size_t)textSize, 0);
        }
    }
    else if ((strncmp(&pBuf_p[0], "/io-data", 8) == 0))
    {
        memset(jsonBuf, 0, sizeof(jsonBuf));
        int len = generate_io_table(jsonBuf, MAX_HTML_SIZE, (IOCoupler_Device *)&gSharedMem.IOCoupler_Devices);

        if (len > 0)
        {
            ret = send(clientFd_p, jsonBuf, len, 0);
        }
    }
    else if ((strncmp(&pBuf_p[0], "/io-json", 8) == 0))
    {
        memset(header, 0, sizeof(header));
        // DebugP_log("Handle /io-data-json\r\n");
        memset(jsonBuf, 0, sizeof(jsonBuf));
        int len = generate_io_json(jsonBuf, MAX_HTML_SIZE, (IOCoupler_Device *)&gSharedMem.IOCoupler_Devices);
        // DebugP_log("len %d \r\n", len);
        if (len > 0)
        {
            int hlen = snprintf(
                header,
                sizeof(header),
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: application/json\r\n"
                "Content-Length: %d\r\n"
                "Connection: close\r\n"
                "\r\n",
                len);

            ret = send(clientFd_p, header, hlen, 0);
            if (ret > 0) 
            {
                ret = send(clientFd_p, jsonBuf, len, 0);
            }
        }
    }
    else if ((strncmp(&pBuf_p[0], "/api/scan/status", 16) == 0))
    {
        char body[128];
        int bodyLen = 0;
        int hLen    = 0;

        app_ipc_sharemem_lock();
        WSScanStatus_t scanStatus = gSharedMem.ipc_sys.ws_scan_status;
        app_ipc_sharemem_unlock();

        switch (scanStatus)
        {
            case SCAN_STATUS_RUNNING:  /* BUSY / IN PROGRESS */
                bodyLen = snprintf(body, sizeof(body), "{\"status\":\"scanning\"}");
                break;
            case SCAN_STATUS_DONE:  /* DONE */
                bodyLen = snprintf(body, sizeof(body), "{\"status\":\"done\"}");
                break;
            case SCAN_STATUS_ERROR:  /* ERROR */
                bodyLen = snprintf(body, sizeof(body), "{\"status\":\"error\",\"message\":\"Scan failed\"}");
                break;
            case SCAN_STATUS_IDLE:  /* IDLE */
            default:
                bodyLen = snprintf(body, sizeof(body), "{\"status\":\"idle\"}");
                break;
        }

        memset(header, 0, sizeof(header));
        hLen = snprintf(
            header,
            sizeof(header),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: %d\r\n"
            "Connection: close\r\n"
            "\r\n",
            bodyLen);

        ret = send(clientFd_p, header, hLen, 0);
        if (ret > 0)
        {
            ret = send(clientFd_p, body, bodyLen, 0);
        }
    }
    else
    {
        // Unknown get request
        ret = send(clientFd_p, response_404, strlen(response_404), 0);
    }
    return ret;
}

/**
 * @brief Handles incoming HTTP POST requests and guarantees a clean response layout
 */
static int WEB_SERVER_processPostAndRespond(int clientFd_p, const char *const pBuf_p)
{
    int ret = -1;
    
    // Ensure the request target starts with your API path
    if ((strncmp(&pBuf_p[0], "/api/scan", 9) == 0))
    {   
        char resp[128]; // Increased size slightly to safely prevent any buffer clipping
        int len;

        DebugP_log("Get cmd to re-scan IO !\r\n");
        
        app_ipc_sharemem_lock();
        
        if (gSharedMem.ipc_sys.ws_scan_status == SCAN_STATUS_RUNNING)
        {
            app_ipc_sharemem_unlock();
            
            DebugP_log("Scan ignored: Hardware scan already in progress.\r\n");
            snprintf(resp, sizeof(resp), "{\"status\":\"busy\",\"message\":\"Scan already in progress\"}");
        }
        else 
        {
            gSharedMem.ipc_sys.ws_scan_status = SCAN_STATUS_IDLE;
            gSharedMem.ipc_sys.ws_cmd = WS_SCAN_IO;
            app_ipc_sharemem_unlock();

            DebugP_log("Scan initiated successfully in background.\r\n");
            snprintf(resp, sizeof(resp), "{\"status\":\"initiated\",\"message\":\"Scan task started background\"}");
        }

        len = strlen(resp);

        memset(header, 0, sizeof(header));
        int hlen = snprintf(
            header,
            sizeof(header),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: %d\r\n"
            "Connection: close\r\n"
            "\r\n",
            len);

        ret = send(clientFd_p, header, hlen, 0);
        if (ret > 0) 
        {
            ret = send(clientFd_p, resp, len, 0);
        }
        
        DebugP_log("POST /api/scan response dispatched instantly!\r\n");
    }
    else
    {
        // Default Fallback: Route Not Found (404)
        const char* http_404_err = 
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Length: 0\r\n"
            "Connection: close\r\n"
            "\r\n";
            
        ret = send(clientFd_p, http_404_err, strlen(http_404_err), 0);
    }

    return ret;
}

/*!
*  Function: WEB_SERVER_task
*
*  \brief
*  Web server task function.
*
*  \details
*  Webserver task function. It waits for bytes to arrive in a blocking manner.
*  If an operation fails the reason for the failure is read through errno.
*  If the error is other than : EAGAIN, EWOULDBLOCK, ECONNABORTED, EINTR,
*  ECONNRESET and ENOTCONN (only for shutdown), then the task is stopped.
*
*  \param[in]        pArgs_p   Not used.
*
*  \return           None
*
*/
static void WEB_SERVER_task(void* pArgs_p)
{
    int socketFd = -1;
    int ret = WEB_SERVER_init(&socketFd);

    if ((ret == 0) && (socketFd >= 0))
    {
        while (1)
        {
            int clientFd = accept(socketFd, NULL, NULL);
            static char aBuf[1500] = { 0 };
            memset(aBuf, 0, sizeof(aBuf));
            if (clientFd < 0)
            {
                if (WEB_SERVER_isFatalErr(errno))
                {
                    // If error is fatal log it and stop webserver task.
                    DebugP_log("Failed to accept connection : %d : %s \r\n", -errno,
                               WEB_SERVER_strError(errno));
                    break;
                }
                continue;
            }

            ret = recv(clientFd, aBuf, sizeof(aBuf), 0);
            if (ret < 0)
            {
                if (WEB_SERVER_isFatalErr(errno))
                {
                    // If error is fatal log it and stop webserver task.
                    DebugP_log("Failed to receive bytes on connection : %d : %s \r\n", -errno,
                               WEB_SERVER_strError(errno));
                    break;
                }
            }
            else if (ret == 0)
            {
                DebugP_log("Connection closed ! \r\n");
            }
            else
            {
                if (strncmp(&aBuf[0], "GET ", 4) == 0)
                {
                    // We received some bytes and it is a get request,
                    // call processing function.
                    // DebugP_log("aBuf = %s\r\n", aBuf);
                    ret = WEB_SERVER_processGetAndRespond(clientFd, &aBuf[4]);
                    if (ret <= 0)
                    {
                        if (WEB_SERVER_isFatalErr(errno))
                        {
                            DebugP_log("Failed to process request sent : %d : %s \r\n", -errno,
                                       WEB_SERVER_strError(errno));
                            break;
                        }
                    }
                }
                else if (strncmp(&aBuf[0], "POST ", 5) == 0)
                {
                    // Pass the buffer starting immediately after "POST " to catch the route string
                    ret = WEB_SERVER_processPostAndRespond(clientFd, &aBuf[5]);
                    if (ret <= 0)
                    {
                        if (WEB_SERVER_isFatalErr(errno))
                        {
                            DebugP_log("Failed to process POST request : %d : %s \r\n", -errno,
                                       WEB_SERVER_strError(errno));
                            break;
                        }
                    }
                }
            }

            char drainBuf[64];
            while (recv(clientFd, drainBuf, sizeof(drainBuf), MSG_DONTWAIT) > 0) {
                // Spooling out unread browser data bytes to guarantee a clean TCP FIN handshake
            }
            ret = shutdown(clientFd, SHUT_RDWR);
            if (ret != 0)
            {
                if ((WEB_SERVER_isFatalErr(errno)) &&
                    (errno != ENOTCONN))
                {
                    DebugP_log("Failed to shutdown connection : %d : %s \r\n", -errno,
                               WEB_SERVER_strError(errno));
                    break;
                }
            }

            ret = close(clientFd);
            if (ret != 0)
            {
                if (WEB_SERVER_isFatalErr(errno))
                {
                    DebugP_log("Failed to close connection : %d : %s \r\n", -errno,
                               WEB_SERVER_strError(errno));
                    break;
                }
            }

            OSAL_SCHED_sleep(WEB_SERVER_TASK_TICK_MS);
        }

        ret = close(socketFd);
        if (ret != 0)
        {
            DebugP_log("Failed to close socket : %d : %s \r\n", -errno,
                       WEB_SERVER_strError(errno));
        }
    }

    TaskP_exit();
}

/*!
 *  Function: WEB_SERVER_startTask
 *
 *  \brief
 *  Function to start the Web server task.
 *
 *  \details
 *  Function to start the Web server task.
 *
 *  \param[in]     None
 *
 *  \return        Result of the operation as boolean.
 *  \retval        true        Operation completed successfully.
 *  \retval        false       Operation could not be completed successfully.
 */
bool WEB_SERVER_startTask(WEB_SERVER_SParams_t* pParams_p)
{
    bool result = true;
    int32_t taskErr = SystemP_SUCCESS;
    TaskP_Params webserverTaskParam = { 0 };

    TaskP_Params_init (&webserverTaskParam);

    webserverTaskParam.name        = "webserver_task";
    webserverTaskParam.stackSize   = sizeof(WEB_SERVER_taskStack_g);
    webserverTaskParam.stack       = (uint8_t*)WEB_SERVER_taskStack_g;
    webserverTaskParam.priority    = pParams_p->taskPrio;
    webserverTaskParam.taskMain    = (TaskP_FxnMain)WEB_SERVER_task;
    webserverTaskParam.args        = pParams_p;

    taskErr = TaskP_construct(&WEB_SERVER_taskObj_g, &webserverTaskParam);
    if (taskErr != SystemP_SUCCESS)
    {
        OSAL_printf("[APP] ERROR: Failed to create task %s (%ld)\r\n",
                    webserverTaskParam.name, taskErr);
        TaskP_destruct(&WEB_SERVER_taskObj_g);
        result = false;
    }
    return result;
}
