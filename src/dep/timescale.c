#include "../ptpd.h"

struct timescale {
	int64_t counterTime;
	struct timespec referenceTime;
	int64_t  counterFrequency;
	int64_t  correctedFrequency;
};

static struct timescale rtTimescale;

/**
 * @brief Initial calibration of QueryPerformanceCounter
 * Query performance frequency is not precise and it is not possible to use it
 * @return Measured frequency of QueryPerformanceCounter
 */
static int64_t RtQueryPerformanceFrequency(void)
{
#if defined(WIN32) && !defined(UNDER_RTSS)
    /* Windows TODO*/
#else
    int64_t nsecElapsed, ticksElapsed;
    
	LARGE_INTEGER beginRt, endRt;
    LARGE_INTEGER beginQpc,endQpc;
    
	/* resolution CLOCK_2 is 1 micro second, but it depends on HAL timer settings */
    RtGetClockTime(CLOCK_2, &beginRt);
    QueryPerformanceCounter(&beginQpc);
    
	/* Some unspecified sleep measured by RtGetClockTime and QueryPerformanceCounter */
	Sleep(1000);

    RtGetClockTime(CLOCK_2, &endRt);
    QueryPerformanceCounter(&endQpc);
    
	nsecElapsed = (endRt.QuadPart - beginRt.QuadPart) * 100;
    ticksElapsed = endQpc.QuadPart - beginQpc.QuadPart;
    
	return ticksElapsed * NS_IN_SEC / nsecElapsed;
#endif
}

/**
 * @brief Normalize input time value
 * @param value: time to normalize (nanoseconds part will be in range -999999999 to 999999999
 */
static void RtNormalizeTime(struct timespec * value)
{
    value->tv_sec += value->tv_nsec / NS_IN_SEC;
    value->tv_nsec -= value->tv_nsec / NS_IN_SEC * NS_IN_SEC;

    if (value->tv_sec > 0 && value->tv_nsec < 0) {
        value->tv_sec -= 1;
        value->tv_nsec += NS_IN_SEC;
    } else if (value->tv_sec < 0 && value->tv_nsec > 0) {
        value->tv_sec += 1;
        value->tv_nsec -= NS_IN_SEC;
    }
}

/**
 * @brief Add times
 * @param value
 * @param value1
 *
 * value = value + value1
 */
static void RtAddTime(struct timespec * value, const struct timespec * value1)
{
    value->tv_sec  +=  value1->tv_sec;
    value->tv_nsec +=  value1->tv_nsec;
    RtNormalizeTime(value);
}

/**
 * @brief Substract times
 * @param value
 * @param value1
 *
 * value = value - value1
 */
static void RtAubTime(struct timespec * value, const struct timespec * value1)
{
    value->tv_sec  -=  value1->tv_sec;
    value->tv_nsec -=  value1->tv_nsec;
    RtNormalizeTime(value);
}

/**
 * @brief Adjust timescale frequency
 * @param adj Adjustment in PPB (parts per bilion = 1/1e9)
 */
void RtAdjFreq(int32_t adj)
{
    struct timespec helptime;
    int64_t newCorrFreq;

    newCorrFreq = rtTimescale.counterFrequency * adj / NS_IN_SEC;
    newCorrFreq = rtTimescale.counterFrequency - newCorrFreq;

    if (rtTimescale.correctedFrequency != newCorrFreq) {
        /* reset timescale */
		RtGetTime(&helptime);
        RtSetTime(&helptime);
		/* set new correction frequency */
		rtTimescale.correctedFrequency = newCorrFreq;
    }
}


/**
 * @brief Adjust timescale offset
 * @param offset
 * new time = old time + offset
 */
void RtAdjTime(const struct timespec * offset)
{
    RtAddTime(&rtTimescale.referenceTime, offset);
}

/**
 * @brief Initialize timescale
 * Initialize timescale with time 0 and frequency from RtQueryPerformanceFrequency
 */
void RtInitTimescale (void)
{
    struct timespec helptime;
	LARGE_INTEGER freq;

	QueryPerformanceFrequency(&freq);

	rtTimescale.counterFrequency = RtQueryPerformanceFrequency();
    rtTimescale.correctedFrequency = rtTimescale.counterFrequency;
    
	helptime.tv_sec = 0;
    helptime.tv_nsec = 0;
    
	RtSetTime(&helptime);

	RtPrintf("InitTimescale: Orig=%lld Hz\tMeasured=%lld Hz\r\n", freq.QuadPart, rtTimescale.counterFrequency);
}

/**
 * @brief Set timescale time
 * @param value: time to set
 */
void RtSetTime (const struct timespec * value)
{
    LARGE_INTEGER tick;

    QueryPerformanceCounter(&tick);   
	rtTimescale.counterTime = tick.QuadPart;
    
	rtTimescale.referenceTime.tv_sec = value->tv_sec;
    rtTimescale.referenceTime.tv_nsec = value->tv_nsec;
}

/**
 * @brief Get current time
 * @param value: returned current time
 */
void RtGetTime (struct timespec * value)
{
    LARGE_INTEGER tick;

	/* get current timestamp */
    QueryPerformanceCounter(&tick);

	/* */
	tick.QuadPart -= rtTimescale.counterTime;
    value->tv_sec = (time_t)( tick.QuadPart / rtTimescale.correctedFrequency );
	value->tv_nsec = (long)(( tick.QuadPart % rtTimescale.correctedFrequency ) * NS_IN_SEC / rtTimescale.correctedFrequency);

	RtAddTime(value, &rtTimescale.referenceTime);
}