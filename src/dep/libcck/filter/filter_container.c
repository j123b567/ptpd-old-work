/**
 * @file    filter_container.c
 * @authors Jan Breuer
 * @date   Mon Feb 24 10:30:00 CET 2014
 * 
 * filter container
 */

#include <stdlib.h>

#include "filter_container.h"
#include "filter.h"

#include "../../iniparser/dictionary.h"
#include "../../iniparser/iniparser.h"

#define CONTAINER_SECTION	"filter"

FilterContainer * filterContainerCreate(void) {
	FilterContainer * fc;
	
	fc = calloc(1, sizeof(FilterContainer));
	
	return fc;
}

static int compare (const void * a, const void * b)
{
	return strcmp((char *)a, (char*)b);
}

void filterContainerLoad(FilterContainer * container, dictionary * dict) {
	char ** seckeys;
	int nkeys, i;
	
	char name[32];
	char * key;
	char * key2;
	
	CCKObject * child;

	nkeys = iniparser_getsecnkeys(dict, CONTAINER_SECTION);
	seckeys = iniparser_getseckeys(dict, CONTAINER_SECTION);

	qsort (seckeys, nkeys, sizeof(char *), compare);
	
	name[0] = '\0';

	/* create objects */
	for(i = 0; i<nkeys; i++) {
		key = sizeof(CONTAINER_SECTION) + seckeys[i];	// offset = section name + ':'
		key2 = strchr (key, ':');
		if (strncmp(name, key, key2-key) != 0) {
			strncpy(name, key, key2-key);
		}
		
		if (strcmp(key2, "type") == 0) {
			/* test if object is not yet created */
			if(cckContainerGet(CCK_CONTAINER(container), name) == NULL) {
				child = CCK_OBJECT(FilterCreate(key2, name));
				if (child != NULL) {
					cckContainerAdd(CCK_CONTAINER(container), child);
				} else {
					/* TODO: exception - unable to create object */
				}
			} else {
				/* TODO: exception - object already exists */
			}
		}
	}

	free(seckeys);
}


