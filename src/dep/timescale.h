/**
 * Implementace èasové stupnice pomocí funkcí QPF a QPC s možností regulace rychlosti chodu
 * 
 * Urèeno pro Win32api a RTX
 *
 * @author      Jan Rùžièka, Jan Breuer
 * @version     1.1
 * @since       2013-03-01
 * @file		timescale.h
 */

#ifndef _TIMESCALE_H_
#define _TIMESCALE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define NS_IN_SEC	1000000000L

	//-----------------------------------------------------------------------------
	// Global structures
	//-----------------------------------------------------------------------------

	struct timespec {
		time_t   tv_sec;        /* seconds */
		long    tv_nsec;       /* nanoseconds */
	};

	//-----------------------------------------------------------------------------
	// Function prototypes
	//-----------------------------------------------------------------------------
	void RtInitTimescale(void);
	void RtAdjFreq      (int32_t);
	void RtAdjTime      (const struct timespec *);
	void RtSetTime      (const struct timespec *);
	void RtGetTime      (struct timespec *);


#ifdef __cplusplus
}
#endif
#endif // _TIMESCALE_H_