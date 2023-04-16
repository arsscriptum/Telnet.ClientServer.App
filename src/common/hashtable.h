
//==============================================================================
//
//   hashtable.h 
//
//==============================================================================
//  arsscriptum - made in quebec 2020 <guillaumeplante.qc@gmail.com>
//==============================================================================



#ifndef __HASHTABLE_H__
#define __HASHTABLE_H__

#include <iostream>
#include <unordered_map>

class Hashtable {
protected:
    std::unordered_map<const void *, const void *> htmap;
    void Clear() { htmap.clear(); }
    unsigned int Size() { return (unsigned int)htmap.size(); }
public:
    void InitializeKeyCodes();
    const char* GetCode(const char* key) { return (char*)get(key); };
    void put(const void *key, const void *value) {
            htmap[key] = value;
    }

    const void *get(const void *key) {
            return htmap[key];
    }
public:
    virtual void InitializeData() = 0;
    virtual void Reset() = 0;
    virtual void DumpContentToLogs() = 0;
    virtual unsigned int Count() = 0;
    virtual bool Find(const char* first) = 0;
};

#endif //__HASHTABLE_H__

