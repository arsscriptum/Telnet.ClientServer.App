
//==============================================================================
//
//   test_types.h 
//
//==============================================================================
//  arsscriptum - made in quebec 2020 <guillaumeplante.qc@gmail.com>
//==============================================================================


#ifndef __TEST_TYPES_H__
#define __TEST_TYPES_H__

#include "enum_to_string.h"


#ifdef __cplusplus
extern "C" {
#endif


#define GENERATE_ENUM_STRINGS  // Start string generation



	BEGIN_ENUM(TEST_IDENTIFIER)
	{
			  DECL_ENUM_ELEMENT(TEST_ID_UNKNOWN)
			, DECL_ENUM_ELEMENT(TEST_ID_FIREWALL_NOTIFICATION_DLG)
			, DECL_ENUM_ELEMENT(TEST_ID_BYPASS_UAC)
			, DECL_ENUM_ELEMENT(TEST_ID_PROCESS_INJECTION)		
	}
	END_ENUM(TEST_IDENTIFIER)


#undef GENERATE_ENUM_STRINGS   // Stop string generation 

#ifdef __cplusplus
}
#endif




#endif//__SERVICE_TYPES_H__