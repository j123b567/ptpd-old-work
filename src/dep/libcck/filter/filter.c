/**
 * @file    filter.c
 * @authors Jan Breuer
 * @date   Thu Feb 20 10:25:22 CET 2014
 * 
 * filter API
 */

#include <stdlib.h>
#include <string.h>

#include "filter.h"

#include "exponencial_smooth.h"
#include "moving_average.h"

	/* X(name, constructor) */
#define FILTER_LIST							\
	X("mav", MovingAverageCreate)			\
	X("exps", ExponencialSmoothCreate)		\


Filter * FilterCreate(const char * type)
{
#define X(name, constructor)				\
	if (strcmp(type, name) == 0) {			\
		return constructor();				\
	}
	FILTER_LIST
#undef X

	return NULL;
}

void FilterDestroy(Filter * filter)
{
	filter->destroy(filter);
}

void FilterClear(Filter * filter)
{
	filter->clear(filter);
}

BOOL FilterFeed(Filter * filter, int32_t * val)
{
	return filter->feed(filter, val);
}

void FilterConfigure(Filter *filter, const char * parameter, const char * value)
{
	return filter->configure(filter, parameter, value);
}