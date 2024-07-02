
//==============================================================================
//
//   custom_assert.h 
//
//==============================================================================
//  arsscriptum - made in quebec 2020 <guillaumeplante.qc@gmail.com>
//==============================================================================


#ifndef __CUSTOM_ASSERT_H__
#define __CUSTOM_ASSERT_H__


#include <cstdlib>
#include <cstdio>


#define STR(x) #x

#ifdef _DEBUG_DISABLED
#define MY_ASSERT(x) if (!(x)) { ASSERT_TRACE_ERROR("My custom assertion failed: (%s), file %s, line %d.", STR(x), __FILE__, __LINE__); abort(); }
#else
#define MY_ASSERT(x)
#endif

#endif //__CUSTOM_ASSERT_H__

