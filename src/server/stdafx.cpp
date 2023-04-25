
//==============================================================================
//
//   stdafx.cpp
//
//==============================================================================
//  arsscriptum - made in quebec 2020 <guillaumeplante.qc@gmail.com>
//==============================================================================


#include "stdafx.h"


#ifndef __DEJA_INSIGHT_BUILD_INFO
#define __DEJA_INSIGHT_BUILD_INFO 0x01
#ifdef DEJA_DISABLED
#  pragma message("DEJA INSIGHT IS DISABLED")
#else
#  pragma message("DEJA INSIGHT IS ENABLED")
#endif 
#ifdef _SERVICE_EXECUTABLE
#  pragma message("COMPILING STANDALONE SERVICE EXECUTABLE")
#elif _SERVICE_DLL
#  pragma message("COMPILING SERVICE DLL")
#else
#  pragma message("SERVICE TYPE UNDEFINED")
#endif // _SERVICE_DLL _SERVICE_EXECUTABLE

#ifdef _MFC_LIBS_STATIC
#  pragma message("USING MFC STATIC LIBS")
#elif _MFC_LIBS_DYNAMIC
#  pragma message("USING MFC DLLs DYNAMIC LIBS")
#else
#  pragma message("MFC LINKAGE UNDEFINED")
#endif // _SERVICE_DLL _SERVICE_EXECUTABLE


#endif



// TODO: reference any additional headers you need in STDAFX.H
// and not in this file
std::string base_name(std::string const& path)
{
	return path.substr(path.find_last_of("/\\") + 1);
}
inline std::string string_base_name(std::string const& path)
{
	return base_name(path);
}

void ErrorMessage(char* str)  //display detailed error info
{
	LPVOID msg;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR)&msg,
		0,
		NULL
	);
	LOG_ERROR("svcmain::%s", "%s", (char*)str, (char*)msg);
	//printf("%s: %s\n",(char*)str,(char*)msg);
	LocalFree(msg);
}


void decomposePath(const TCHAR* filePath, TCHAR* fileDir, TCHAR* fileName, TCHAR* fileExt)
{
	const TCHAR* lastSeparator = _STRRCHR(filePath, __PATH_SEPARATOR);
	const TCHAR* lastDot = _STRRCHR(filePath, _T('.'));
	const TCHAR* endOfPath = filePath + _STRLEN(filePath);
	const TCHAR* startOfName = lastSeparator ? lastSeparator + 1 : filePath;
	const TCHAR* startOfExt = lastDot > startOfName ? lastDot : endOfPath;
	if (fileDir)
		__SNPRINTF(fileDir, MAX_PATH, _T("%.*s"), int(startOfName - filePath), filePath);

	if (fileName)
		__SNPRINTF(fileName, MAX_PATH, _T("%.*s"), int(startOfExt - startOfName), startOfName);

	if (fileExt)
		__SNPRINTF(fileExt, MAX_PATH, _T("%s"), startOfExt);

}

String GetErrorMessage(DWORD dwErrorCode)
{
	LPTSTR psz{ nullptr };
	const DWORD cchMsg = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM
		| FORMAT_MESSAGE_IGNORE_INSERTS
		| FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL, // (not used with FORMAT_MESSAGE_FROM_SYSTEM)
		dwErrorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		reinterpret_cast<LPTSTR>(&psz),
		0,
		NULL);
	if (cchMsg > 0)
	{
		// Assign buffer to smart pointer with custom deleter so that memory gets released
		// in case String's c'tor throws an exception.
		auto deleter = [](void* p) { ::LocalFree(p); };
		std::unique_ptr<TCHAR, decltype(deleter)> ptrBuffer(psz, deleter);
		return String(ptrBuffer.get(), cchMsg);
	}
	else
	{
		auto error_code{ ::GetLastError() };
		throw std::system_error(error_code, std::system_category(),
			"Failed to retrieve error message string.");
	}
}

