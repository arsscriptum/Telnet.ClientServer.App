
//==============================================================================
//
//   keycodes_hashtable.h 
//
//==============================================================================
//  arsscriptum - made in quebec 2020 <guillaumeplante.qc@gmail.com>
//==============================================================================



#ifndef __KEYCODES__HASHTABLE_H__
#define __KEYCODES__HASHTABLE_H__

#include <iostream>
#include <unordered_map>
#include "hashtable.h"

class KeyCodesHashTable : public Hashtable {

public:
    void InitializeKeyCodes();
    bool KeyExists(const char* key) {return Find(key);};
    const char* GetCode(const char* key) { return (char*)get(key); };

	void InitializeData();
	void Reset();
	void DumpContentToLogs();
	unsigned int Count();
	bool Find(const char *first);
	bool Test();
};

#endif //__KEYCODES__HASHTABLE_H__

