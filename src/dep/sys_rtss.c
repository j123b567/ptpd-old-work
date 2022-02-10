/*-
* Copyright (c) 2010-2011 Jan Breuer
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
* @file   sys_win32.c
* @date   Tue Jul 6 16:19:46 2011
*
* @brief  Code to call kernel time routines and also display server statistics.
*
*
*/

#include "../ptpd.h"

HANDLE Timer;
HANDLE TimerSelectInterrupt = NULL;


void
getTimeRtss(TimeInternal * time)
{
    struct timespec  RtssTime;
    // lock critical section
    RtGetTime(&RtssTime);
    time->nanoseconds=(Integer32)RtssTime.tv_nsec;
    time->seconds=(Integer32)RtssTime.tv_sec;
}

void
setTimeRtss(TimeInternal * time)
{
    struct timespec  RtssTime;
    RtssTime.tv_nsec=(long)time->nanoseconds;
    RtssTime.tv_sec=(time_t)time->seconds;
    // lock critical section
    RtSetTime(&RtssTime);
}



Boolean
adjFreqRtss(Integer32 adj)
{
    if(adj > ADJ_FREQ_MAX)
        adj = ADJ_FREQ_MAX;
    else if(adj < -ADJ_FREQ_MAX)
        adj = -ADJ_FREQ_MAX;
    // lock critical section
    /* 100 ns units */
    RtAdjFreq(adj);
    return (TRUE);
}


int inet_aton(const char* cp, struct in_addr* inp)
{
    struct in_addr a;
    a.s_addr = inet_addr(cp);

    if(a.s_addr == INADDR_NONE)
        return 0;

    *inp = a;
    return a.s_addr;
}





void RTFCNDCL SetITimerCalback (PVOID context)
{
    void (*callback)(int) = context;
    /* call catch_alarm */
    callback(0);
    /* interrupt netSelect as SIGALRM do */
    if (TimerSelectInterrupt) RtSetEvent(TimerSelectInterrupt);

}


int
setitimerRtss(const TimeInternal * tv, void (*callback)(int))
{
    Boolean Succes;
    LARGE_INTEGER Period;
    Period.QuadPart=(LONGLONG)((tv->nanoseconds+(tv->seconds*PPB_MAX ))/TIMER_UNITS_100ns);

    if (Period.QuadPart == 0) {
        if (TimerSelectInterrupt)
            RtCloseHandle(TimerSelectInterrupt);
        RtDeleteTimer(Timer);
        return 0;
    }

    if (!TimerSelectInterrupt) {
        TimerSelectInterrupt = RtCreateEvent(NULL, TRUE, FALSE, (LPCTSTR)("IntervalTimer"));
    }


    if ((Timer=RtCreateTimer (NULL,0,SetITimerCalback,callback,125,CLOCK_3))!=NULL) {
        if(RtSetTimerRelative(Timer,&Period,&Period))
            Succes=TRUE;
    } else
        Succes=FALSE;


    if(Succes == TRUE) {
        return 0;
    } else {
        errno = EFAULT;
        return -1;
    }
}

int
ether_ntohost(char *hostname, const struct ether_addr *e)
{
    hostname[0] = 0;
    return -1;
}

