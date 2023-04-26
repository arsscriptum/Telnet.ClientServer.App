
//==============================================================================
//
//   ITestRunner.h 
//
//==============================================================================
//  arsscriptum - made in quebec 2020 <guillaumeplante.qc@gmail.com>
//==============================================================================

#ifndef __TEST_UACBYPASS_H__
#define __TEST_UACBYPASS_H__

#include <string>
#include <functional>
#include <thread>
#include <memory>

#include "test_types.h"
#include "ITestRunner.h"
#include "uac_bypass.h"

class Test_UACBypass : public ITestRunner {

	protected:
		bool _initialized = false;
	
	public:
		Test_UACBypass();
		virtual ~Test_UACBypass();

		virtual void Initialize() override;
		virtual void Destroy() override;
		virtual void Execute() override;
		virtual bool IsInitialized() const { return _initialized; };
		
	};



#endif //__TEST_UACBYPASS_H__
