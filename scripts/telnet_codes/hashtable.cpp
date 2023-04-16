\
//==============================================================================
//
//   keycodes_hashtable.cpp 
//
//==============================================================================
//  arsscriptum - made in quebec 2020 <guillaumeplante.qc@gmail.com>
//==============================================================================




#include "stdafx.h"
#include "hashtable.h"

void Hashtable::put(const void *key, const void *value) {
	htmap[key] = value;
}

const void *Hashtable::get(const void *key) {
	return htmap[key];
}