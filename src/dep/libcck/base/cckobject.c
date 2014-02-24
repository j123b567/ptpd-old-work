/**
 * @file    cckobject.c
 * @authors Jan Breuer
 * @date   Mon Feb 24 09:20:21 CET 2014
 * 
 * libcck base object
 */

#include <string.h>
#include "cckobject.h"

void cckObjectNameSet(CCKObject * obj, const char * name) {
	if (name != NULL) {
		strncpy(obj->name, name, sizeof(obj->name));
	} else {
		obj->name[0] = '\0';
	}
}

const char * cckObjectNameGet(CCKObject * obj) {
	return obj->name;
}

void cckObjectInit(CCKObject * obj, const char * name) {
	cckObjectNameSet(obj, name);
	obj->parent = NULL;
	obj->next = NULL;
}