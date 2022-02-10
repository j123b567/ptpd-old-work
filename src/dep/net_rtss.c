/*-
* Copyright (c) 2010-2013 Jan Breuer,Jan Ruzicka
* Copyright (c) 2009-2011 George V. Neville-Neil, Steven Kreuzer,
*                         Martin Burnicki, Gael Mace, Alexandre Van Kempen
* Copyright (c) 2005-2008 Kendall Correll, Aidan Williams
*
* All Rights Reserved
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are
* met:
* 1. Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
* BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
* IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/**
* @file   net_dep.c
* @date   Tue Jun 6 16:17:49 2011
*
* @brief  Functions to interact with the network sockets and NIC driver.
*
*
*/

#include "../ptpd.h"

/**
* Shutdown platform dependent network stuff
*
* @param netPath
*
* @return TRUE if successful
*/

Boolean
netShutdownWSA2Rtss(NetPath * netPath)
{
    /* Close sockets */
	if (netPath->eventSock != INVALID_SOCKET)
        closesocket(netPath->eventSock);
    netPath->eventSock = INVALID_SOCKET;

    if (netPath->generalSock != INVALID_SOCKET)
        closesocket(netPath->generalSock);
    netPath->generalSock = INVALID_SOCKET;


    /* Close WinSock */
    WSACleanup();

    return TRUE;
}

/* Find the local network interface */
Boolean
netFindInterfaceRtss(NetPath * netPath, Octet * ifaceName, UInteger8 * communicationTechnology,
                     Octet * uuid, struct in_addr * ifaceAddr)
{
    struct RTXiface {
        Octet RTXifaceName [IFACE_NAME_LENGTH];
        Octet  RTXuuid [PTP_UUID_LENGTH];
        SOCKADDR_IN  RTXaddr;  //unsigned long
    }   RTX_NIC;


    Octet szBuffer[SZBUFFER_SIZE];
    Octet ch;
    BYTE by;
    Boolean result = FALSE;
    Boolean fMACAddressPresent = FALSE;
    int i,k;
    int number=0;
    int counter=0;
    size_t sizeRtnd=0;
    Octet * pszSectionNameRtnd="rtnd";
    Octet * copyString;
    const Octet * pszSectionNameTCP="TCP/IP";
    const WCHAR * StackIniFile=L"C:\\Program Files\\IntervalZero\\RTX\\bin\\RtxTcpIp.ini";


    //RTX

    //not a default option I have to set value at 1 in case of that option is not present
    if ((counter =
             RtnGetPrivateProfileString(StackIniFile, pszSectionNameTCP, "NumOfInterfaces", "1",	szBuffer, SZBUFFER_SIZE))<0)
        ERROR("error reading from INI file.\n");

    sizeRtnd=strlen(szBuffer);

    if ((counter==1) && (strncmp(szBuffer,"0",1)==0))
        ERROR("Nenasel jsem nastaveni pro NIC kartu nebo neni pritomna zadna karta\n ");
    else {
        number=atoi(szBuffer);
        //maximum number of NIC cards is 255
        if (number>9)
            ERROR("Chybne nastaveny INI file,prilis mnoho NIC karet,maximum je 9 karet ");
        sizeRtnd = strlen(pszSectionNameRtnd);
        copyString = (char*)malloc(sizeRtnd + 1 + 1); /* one for extra char, one for trailing zero */
        for (k = 0; (k < number); k++) {
            if  (number<=9) {
                ch='0' + k;//compatible conversion integer to ASCII number

                strcpy(copyString, pszSectionNameRtnd);
                copyString[(sizeRtnd + 1 + 1) - 2] = ch;
                copyString[(sizeRtnd + 1 + 1) - 1] = '\0';

                if ((counter = RtnGetPrivateProfileString(StackIniFile, copyString, "IPAddr", "",	szBuffer, SZBUFFER_SIZE))<0)
                    ERROR("error reading from INI file.\n");

                RTX_NIC.RTXaddr.sin_addr.s_addr = inet_addr(szBuffer);

                if ((counter = RtnGetPrivateProfileString(StackIniFile, copyString, "EA", "",	szBuffer, SZBUFFER_SIZE))<0)
                    ERROR("error reading from INI file.\n");

                if (counter = 12) {
                    for (i = 0; (i < 6); i++) {
                        ch = szBuffer[(i * 2)];
                        if ((ch >= '0') && (ch <= '9')) {
                            by = (ch - '0');
                        } else if ((ch >= 'A') && (ch <= 'F')) {
                            by = ((ch - 'A') + 10);
                        } else if ((ch >= 'a') && (ch <= 'f')) {
                            by = ((ch - 'a') + 10);
                        } else {
                            fMACAddressPresent = FALSE;
                            break;
                        }
                        RTX_NIC.RTXuuid[i] = (by << 4);
                        ch = szBuffer[((i * 2) + 1)];
                        if ((ch >= '0') && (ch <= '9')) {
                            by = (ch - '0');
                        } else if ((ch >= 'A') && (ch <= 'F')) {
                            by = ((ch - 'A') + 10);
                        } else if ((ch >= 'a') && (ch <= 'f')) {
                            by = ((ch - 'a') + 10);
                        } else {
                            fMACAddressPresent = FALSE;
                            break;
                        }

                        RTX_NIC.RTXuuid[i] |= by;
                        if (RTX_NIC.RTXuuid[i] != 0x00) {
                            fMACAddressPresent = TRUE;
                        }

                    }
                }
                strcpy(RTX_NIC.RTXifaceName, copyString);

                /* there is no other way to get interface names
                * we must show available names to user */
                INFO("found interface: %s\n",RTX_NIC.RTXifaceName);

                if (((ifaceName[0] == 0) && (result == 0))
                    || (0==strncmp(ifaceName, RTX_NIC.RTXifaceName, strlen(RTX_NIC.RTXifaceName)))) {
                    DBG("Interface %s selected\n", RTX_NIC.RTXifaceName);
                    *communicationTechnology = PTP_ETHER;
                    strcpy(ifaceName, RTX_NIC.RTXifaceName);


                    if (fMACAddressPresent==TRUE) {
                        memcpy(uuid,RTX_NIC.RTXuuid,min(sizeof(RTX_NIC.RTXuuid), PTP_UUID_LENGTH));
                    } else
                        ERROR("error translating MAC adress from INI file for NIC adapter %s .\n",RTX_NIC.RTXifaceName);

                    *ifaceAddr = RTX_NIC.RTXaddr.sin_addr;
                    result = TRUE;
                }

            }

        }


        free(copyString);
    }

    if (ifaceName[0] == '\0') {
        ERROR("failed to find a usable interface\n");
        result = FALSE;
    }

    return result;
}

/**
* start platform dependent network initialization
*
* @param netPath
*
* @return TRUE if successful
*/
Boolean
netInitWSA2Rtss(NetPath * netPath)
{
    WSADATA data;
    WORD version;
    int ret = 0;


    // Winsock 2 required
    version = (MAKEWORD(2, 2));
    ret = WSAStartup(version, &data);

    if(ret != 0) {
        PERROR("netInitWSA2");
        return FALSE;
    }


    return TRUE;
}


/**
* Check if data have been received
*
* @param timeout
* @param netPath
*
* @return result > 0: some message has been recieved,
* result = 0: timeout or external interrupt (SIG_ALRM)
* result < 0: error occured
*/
int
netSelectWSA2Rtss(TimeInternal * timeout, NetPath * netPath)
{

    int ret, nfds;
    fd_set readfds;
    struct timeval tv, *tv_ptr;
#if defined(TIMESTAMPING_GET_TIME)
    TimeInternal timestamp;
#endif

    if (timeout < 0)
        return FALSE;

    FD_ZERO(&readfds);
	if (netPath->eventSock != INVALID_SOCKET)
		FD_SET(netPath->eventSock, &readfds);
	if (netPath->generalSock != INVALID_SOCKET)
	    FD_SET(netPath->generalSock, &readfds);

    if (timeout) {
        tv.tv_sec = timeout->seconds;
        tv.tv_usec = timeout->nanoseconds / 1000;
        tv_ptr = &tv;
    } else
        tv_ptr = 0;

    if (netPath->eventSock > netPath->generalSock)
        nfds = netPath->eventSock;
    else
        nfds = netPath->generalSock;

    ret = select(nfds + 1, &readfds, 0, 0, tv_ptr) > 0;

    if (netPath->eventSock != INVALID_SOCKET && FD_ISSET(netPath->eventSock, &readfds)) {
#if defined(TIMESTAMPING_GET_TIME)
        /* we dont have better oportunity to recieve timestamp */
        getTime(&timestamp);
        netPath->lastReceivedSeconds = timestamp.seconds;
        netPath->lastReceivedNanoseconds = timestamp.nanoseconds;
#endif
    }

    if (netPath->generalSock != INVALID_SOCKET && FD_ISSET(netPath->generalSock, &readfds)) {
        //add action
    }

    if (ret < 0) {
        if (errno == EAGAIN || errno == EINTR)
            return 0;
    }

    return ret;

    /*int ret = 0;
    int n = 0;

    WSAEVENT pEvents[4];

    // initialize events
    if(!netPath->eventHEvent) {

    netPath->eventHEvent = RtCreateEvent( NULL, FALSE, TRUE, "TestEvent1" );//WSACreateEvent();
    if ((WSAEventSelect(netPath->eventSock, netPath->eventHEvent, FD_READ))==SOCKET_ERROR)
    PERROR("netSelectWSA2");


    netPath->generalHEvent =  RtCreateEvent( NULL, FALSE, TRUE, "TestEvent2" );//WSACreateEvent();
    if ((WSAEventSelect(netPath->generalSock, netPath->generalHEvent, FD_READ))==SOCKET_ERROR)
    PERROR("netSelectWSA2");
    }


    pEvents[n++] = netPath->eventHEvent;

    pEvents[n++] = netPath->generalHEvent;

    if(NULL != TimerSelectInterrupt) {
    pEvents[n++] = TimerSelectInterrupt;
    }

    #if defined(TIMESTAMPING_PCAP)
    pEvents[n++] = pcap_getevent(netPath->eventPcap);
    #endif

    ret = RtWaitForMultipleObjects(n, pEvents, FALSE, INFINITE);

    if (ret >= WSA_WAIT_EVENT_0) {
    ret -= WSA_WAIT_EVENT_0;
    ret += 1;
    /* cleare appropriate event*/
    /*RtResetEvent(pEvents[ret-1]);

    /* if the event is from timer, return value is 0 */
    /*if(pEvents[ret-1] == TimerSelectInterrupt) {
    ret = 0;
    }
    } else {
    ret = 0;
    }
    RtPrintf("hodnota ret %d\n",ret);
    return ret;
    */
}


ssize_t
netRecvEventWSA2Rtss(Octet *buf, TimeInternal *time, NetPath * netPath)
{
    ssize_t ret;
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);

#if defined(TIMESTAMPING_GET_TIME)
    if(NULL != time) {
        /* we dont have better oportunity to recieve timestamp  */
        time->seconds = netPath->lastReceivedSeconds;
        time->nanoseconds = netPath->lastReceivedNanoseconds;
    }
#endif

    ret = recvfrom(netPath->eventSock, buf, PACKET_SIZE, MSG_PEEK, (struct sockaddr *)&addr, &addr_len);
    if(ret > 0) {
        ret = recvfrom(netPath->eventSock, buf, PACKET_SIZE, 0, (struct sockaddr *)&addr, &addr_len);
    } else {
        errno = EAGAIN;
        ret = 0;
    }
    if(ret <= 0) {
        if(errno == EAGAIN || errno == EINTR)
            return 0;
        return ret;
    }

    return ret;
}

/**
*
* store received data from network to "buf", timestamping
* of general messages is not necessery
*  * @param buf
* @param time
* @param netPath
*
* @return size of recieved packet
*/

ssize_t
netRecvGeneralWSA2Rtss(Octet * buf, NetPath * netPath)
{
    ssize_t ret;
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);

    /* on windows platform is not MSG_DONTWAIT, using MSG_PEEK insted */
    ret = recvfrom(netPath->generalSock, buf, PACKET_SIZE, MSG_PEEK, (struct sockaddr *)&addr, &addr_len);
    if(ret > 0) {
        ret = recvfrom(netPath->generalSock, buf, PACKET_SIZE, 0, (struct sockaddr *)&addr, &addr_len);
    } else {
        errno = EAGAIN;
        ret = 0;
    }

    if (ret <= 0) {
        if (errno == EAGAIN || errno == EINTR)
            return 0;

        return ret;
    }
    return ret;
}




