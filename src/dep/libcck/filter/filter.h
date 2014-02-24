#ifndef _LIBCCK_FILTER_FILTER_H_
#define _LIBCCK_FILTER_FILTER_H_

/**
 * @file    filter.h
 * @authors Jan Breuer
 * @date   Thu Feb 20 10:25:22 CET 2014
 * 
 * filter API
 */

#include "../ccktypes.h"

#define FILTER_EXPONENTIAL_SMOOTH       1
#define FILTER_MOVING_AVERAGE           2

typedef struct _Filter Filter;

struct _Filter {
	BOOL (*feed)(Filter *filter, int32_t * value);
	void (*clear)(Filter *filter);
	void (*destroy)(Filter *filter);
	void (*configure)(Filter *filter, const char * parameter, const char * value);
};


Filter * FilterCreate(int type);

void FilterDestroy(Filter * filter);

void FilterClear(Filter * filter);

BOOL FilterFeed(Filter * filter, int32_t * value);

void FilterConfigure(Filter *filter, const char * parameter, const char * value);

#endif /* _LIBCCK_FILTER_FILTER_H_ */