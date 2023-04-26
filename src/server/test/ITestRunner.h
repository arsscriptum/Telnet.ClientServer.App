
//==============================================================================
//
//   ITestRunner.h 
//
//==============================================================================
//  arsscriptum - made in quebec 2020 <guillaumeplante.qc@gmail.com>
//==============================================================================

#ifndef __ITEST_RUNNER_H__
#define __ITEST_RUNNER_H__


#include <string>
#include <functional>
#include <thread>
#include <memory>

#include "test_types.h"

class ITestRunner{

	protected:
		bool _initialized = false;
	
	public:
		ITestRunner();
		virtual ~ITestRunner();

		virtual void Initialize() = 0;
		virtual void Destroy() = 0;
		virtual void Execute() = 0;
		virtual bool IsInitialized() const { return _initialized; };
		
	};



#endif //__ITEST_RUNNER_H__
