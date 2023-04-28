
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
    static TestManager& getInstance()
    {
        static TestManager instance; // Guaranteed to be destroyed.
                              // Instantiated on first use.
        return instance;
    }


    void Id(LPCSTR val) { _STRNCPY(_test_id, val, MAX_PATH); }
    LPCSTR  Id() { return _test_id; }

private:
    TestManager();
    // Don't forget to declare these two. You want to make sure they
    // are inaccessible(especially from outside), otherwise, you may accidentally get copies of
    // your singleton appearing.
    TestManager(TestManager const&) = delete;
    void operator=(TestManager const&) = delete;

    TCHAR _test_id[MAX_PATH];
};




#endif//__TEST_MGR_H__