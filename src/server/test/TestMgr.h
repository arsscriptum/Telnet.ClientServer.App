
//==============================================================================
//
//   TestMgr.h 
//
//==============================================================================
//  arsscriptum - made in quebec 2020 <guillaumeplante.qc@gmail.com>
//==============================================================================


#ifndef __TEST_MGR_H__
#define __TEST_MGR_H__


#include <iostream>

using namespace std;

class TestManager {
public:
    // This is how clients can access the single instance
    static TestManager* getInstance();

    void Id(LPCSTR val) { _STRNCPY(_test_id, val, MAX_PATH); }
    LPCSTR  Id() { return _test_id; }

protected:
    TCHAR _test_id[MAX_PATH];

private:
    static TestManager* inst_;   // The one, single instance
    TestManager(); // private constructor
    TestManager(const TestManager&);
    TestManager& operator=(const TestManager&);
};

// Define the static Singleton pointer
TestManager* TestManager::inst_ = NULL;

TestManager* TestManager::getInstance() {
    if (inst_ == NULL) {
        inst_ = new TestManager();
    }
    return(inst_);
}



#endif//__TEST_MGR_H__