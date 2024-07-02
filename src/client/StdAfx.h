
//==============================================================================
//
//   stdafx.h 
//
//==============================================================================
//  arsscriptum - made in quebec 2020 <guillaumeplante.qc@gmail.com>
//==============================================================================



#if !defined(AFX_STDAFX_H__FE95BC89_9860_11D3_8CD5_00C0F0405B24__INCLUDED_)
#define AFX_STDAFX_H__FE95BC89_9860_11D3_8CD5_00C0F0405B24__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x601
#endif

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#define CSBLOCK

#pragma  warning( disable: 4996 )

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC OLE automation classes
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include "log.h"
#include "custom_assert.h"

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__FE95BC89_9860_11D3_8CD5_00C0F0405B24__INCLUDED_)
