//==============================================================================
//
//   ServerService.cpp
//
//==============================================================================
//  Copyright (C) Guilaume Plante 2020 <guillaumeplante.qc@gmail.com>
//==============================================================================


#include "stdafx.h"
// link with Kernel32.lib and Advapi32.lib.

#include <windows.h>

#include <strsafe.h>

#include <aclapi.h>

#include <stdio.h>

 

//#pragma comment(lib, advapi32.lib)

 

WCHAR szCommand[10];

WCHAR szSvcName[80];

 

SC_HANDLE schSCManager;

SC_HANDLE schService;

 

VOID __stdcall DoStartSvc(void);

VOID __stdcall DoUpdateSvcDacl(void);

VOID __stdcall DoStopSvc(void);

BOOL __stdcall StopDependentServices(void);

 

// Purpose: Entry point function. Executes specified command from user.

// Parameters: Command-line syntax is: svccontrol [command] [service_name]

// Return value: None

 

// Purpose: Starts the service if possible.

// Parameters: None

// Return value: None

VOID __stdcall DoStartSvc()

{

    SERVICE_STATUS_PROCESS ssStatus;

    DWORD dwOldCheckPoint;

    DWORD dwStartTickCount;

    DWORD dwWaitTime;

    DWORD dwBytesNeeded;

 

    // Get a handle to the SCM database

    schSCManager = OpenSCManager(

        NULL,                    // local computer

        NULL,                    // servicesActive database

        SC_MANAGER_ALL_ACCESS);  // full access rights

 

    if (NULL == schSCManager)

    {

        wprintf(LOpenSCManager() failed, error %d\n, GetLastError());

        return;

    }

 

    // Get a handle to the service

    schService = OpenService(

        schSCManager,         // SCM database

        szSvcName,            // name of service

        SERVICE_ALL_ACCESS);  // full access

 

    if (schService == NULL)

    {

        wprintf(LOpenService() failed, error %d\n, GetLastError());

        CloseServiceHandle(schSCManager);

        return;

    }   

 

    // Check the status in case the service is not stopped.

    if (!QueryServiceStatusEx(

            schService,                     // handle to service

            SC_STATUS_PROCESS_INFO,         // information level

            (LPBYTE) &ssStatus,             // address of structure

            sizeof(SERVICE_STATUS_PROCESS), // size of structure

            &dwBytesNeeded ) )              // size needed if buffer is too small

    {

        wprintf(LQueryServiceStatusEx() failed, error %d\n, GetLastError());

        CloseServiceHandle(schService);

        CloseServiceHandle(schSCManager);

        return;

    }

 

    // Check if the service is already running. It would be possible

    // to stop the service here, but for simplicity this example just returns.

    if(ssStatus.dwCurrentState != SERVICE_STOPPED && ssStatus.dwCurrentState != SERVICE_STOP_PENDING)

    {

        wprintf(LCannot start the %s service because it is already running\n, szSvcName);

        CloseServiceHandle(schService);

        CloseServiceHandle(schSCManager);

        return;

    }

 

    // Save the tick count and initial checkpoint

    dwStartTickCount = GetTickCount();

    dwOldCheckPoint = ssStatus.dwCheckPoint;

 

    // Wait for the service to stop before attempting to start it

    while (ssStatus.dwCurrentState == SERVICE_STOP_PENDING)

    {

        // Do not wait longer than the wait hint. A good interval is

        // one-tenth of the wait hint but not less than 1 second 

        // and not more than 10 seconds

        dwWaitTime = ssStatus.dwWaitHint / 10;

 

        if( dwWaitTime < 1000 )

            dwWaitTime = 1000;

        else if ( dwWaitTime > 10000 )

            dwWaitTime = 10000;

 

        Sleep( dwWaitTime );

 

        // Check the status until the service is no longer stop pending

        if (!QueryServiceStatusEx(

                schService,                     // handle to service

                SC_STATUS_PROCESS_INFO,         // information level

                (LPBYTE) &ssStatus,             // address of structure

                sizeof(SERVICE_STATUS_PROCESS), // size of structure

                &dwBytesNeeded ) )              // size needed if buffer is too small

        {

            wprintf(LQueryServiceStatusEx failed (%d)\n, GetLastError());

            CloseServiceHandle(schService);

            CloseServiceHandle(schSCManager);

            return;

        }

 

        if ( ssStatus.dwCheckPoint > dwOldCheckPoint )

        {

            // Continue to wait and check

            dwStartTickCount = GetTickCount();

            dwOldCheckPoint = ssStatus.dwCheckPoint;

        }

        else

        {

            if(GetTickCount()-dwStartTickCount > ssStatus.dwWaitHint)

            {

                wprintf(LTimeout waiting for %s service to stop\n, szSvcName);

                CloseServiceHandle(schService);

                CloseServiceHandle(schSCManager);

                return;

            }

        }

    }

 

    // Attempt to start the service.

    if (!StartService(

            schService,  // handle to service

            0,           // number of arguments

            NULL) )      // no arguments

    {

        wprintf(LStartService() failed, error %d\n, GetLastError());

        CloseServiceHandle(schService);

        CloseServiceHandle(schSCManager);

        return;

    }

    else

            wprintf(LService %s start pending...\n, szSvcName);

 

    // Check the status until the service is no longer start pending.

    if (!QueryServiceStatusEx(

            schService,                     // handle to service

            SC_STATUS_PROCESS_INFO,         // info level

            (LPBYTE) &ssStatus,             // address of structure

            sizeof(SERVICE_STATUS_PROCESS), // size of structure

            &dwBytesNeeded ) )              // if buffer too small

    {

        wprintf(LQueryServiceStatusEx failed, error %d\n, GetLastError());

        CloseServiceHandle(schService);

        CloseServiceHandle(schSCManager);

        return;

    }

 

    // Save the tick count and initial checkpoint

    dwStartTickCount = GetTickCount();

    dwOldCheckPoint = ssStatus.dwCheckPoint;

 

    while (ssStatus.dwCurrentState == SERVICE_START_PENDING)

    {

        // Do not wait longer than the wait hint. A good interval is

        // one-tenth the wait hint, but no less than 1 second and no more than 10 seconds

        dwWaitTime = ssStatus.dwWaitHint / 10;

 

        if( dwWaitTime < 1000 )

            dwWaitTime = 1000;

        else if ( dwWaitTime > 10000 )

            dwWaitTime = 10000;

 

        Sleep( dwWaitTime );

 

        // Check the status again

        if (!QueryServiceStatusEx(

            schService,             // handle to service

            SC_STATUS_PROCESS_INFO, // info level

            (LPBYTE) &ssStatus,             // address of structure

            sizeof(SERVICE_STATUS_PROCESS), // size of structure

            &dwBytesNeeded ) )              // if buffer too small

        {

            wprintf(LQueryServiceStatusEx failed, error %d\n, GetLastError());

            break;

        }

 

        if ( ssStatus.dwCheckPoint > dwOldCheckPoint )

        {

            // Continue to wait and check

            dwStartTickCount = GetTickCount();

            dwOldCheckPoint = ssStatus.dwCheckPoint;

        }

        else

        {

            if(GetTickCount()-dwStartTickCount > ssStatus.dwWaitHint)

            {

                // No progress made within the wait hint.

                break;

            }

        }

    }

 

    // Determine whether the service is running

    if (ssStatus.dwCurrentState == SERVICE_RUNNING)

    {

        wprintf(LService %s started successfully.\n, szSvcName);

    }

    else

    {

        wprintf(LService %s not started. \n, szSvcName);

        wprintf(L  Current State: %d\n, ssStatus.dwCurrentState);

        wprintf(L  Exit Code: %d\n, ssStatus.dwWin32ExitCode);

        wprintf(L  Check Point: %d\n, ssStatus.dwCheckPoint);

        wprintf(L  Wait Hint: %d\n, ssStatus.dwWaitHint);

    }

 

    CloseServiceHandle(schService);

    CloseServiceHandle(schSCManager);

}

 

// Purpose:

//   Updates the service DACL to grant start, stop, delete, and read

//   control access to the Guest account.//

// Parameters: None

// Return value: None

VOID __stdcall DoUpdateSvcDacl()

{

    EXPLICIT_ACCESS      ea;

    SECURITY_DESCRIPTOR  sd;

    PSECURITY_DESCRIPTOR psd            = NULL;

    PACL                 pacl           = NULL;

    PACL                 pNewAcl        = NULL;

    BOOL                 bDaclPresent   = FALSE;

    BOOL                 bDaclDefaulted = FALSE;

    DWORD                dwError        = 0;

    DWORD                dwSize         = 0;

    DWORD                dwBytesNeeded  = 0;

 

    // Get a handle to the SCM database.

    schSCManager = OpenSCManager(

        NULL,                    // local computer

        NULL,                    // ServicesActive database

        SC_MANAGER_ALL_ACCESS);  // full access rights

 

    if (NULL == schSCManager)

    {

        wprintf(LOpenSCManager() failed (%d)\n, GetLastError());

        return;

    }

 

    // Get a handle to the service

    schService = OpenService(

        schSCManager,              // SCManager database

        szSvcName,                 // name of service

        READ_CONTROL | WRITE_DAC); // access

 

    if (schService == NULL)

    {

        wprintf(LOpenService() failed, error %d\n, GetLastError());

        CloseServiceHandle(schSCManager);

        return;

    }   

 

    // Get the current security descriptor

    if (!QueryServiceObjectSecurity(schService,

        DACL_SECURITY_INFORMATION,

        &psd,           // using NULL does not work on all versions

        0,

        &dwBytesNeeded))

    {

        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)

        {

            dwSize = dwBytesNeeded;

            psd = (PSECURITY_DESCRIPTOR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSize);

            if (psd == NULL)

            {

                // Note: HeapAlloc does not support GetLastError.

                wprintf(LHeapAlloc failed!\n);

                goto dacl_cleanup;

            }

 

            if (!QueryServiceObjectSecurity(schService, DACL_SECURITY_INFORMATION, psd, dwSize, &dwBytesNeeded))

            {

                wprintf(LQueryServiceObjectSecurity() failed, error %d\n, GetLastError());

                goto dacl_cleanup;

            }

        }

        else

        {

            wprintf(LQueryServiceObjectSecurity() failed %d\n, GetLastError());

            goto dacl_cleanup;

        }

    }

 

 

    // Get the DACL

    if (!GetSecurityDescriptorDacl(psd, &bDaclPresent, &pacl, &bDaclDefaulted))

    {

        wprintf(LGetSecurityDescriptorDacl() failed, error %d\n, GetLastError());

        goto dacl_cleanup;

    }

 

    // Build the ACE

    BuildExplicitAccessWithName(&ea, LGUEST, SERVICE_START | SERVICE_STOP | READ_CONTROL | DELETE,

        SET_ACCESS, NO_INHERITANCE);

 

    dwError = SetEntriesInAcl(1, &ea, pacl, &pNewAcl);

    if (dwError != ERROR_SUCCESS)

    {

        wprintf(LSetEntriesInAcl failed, error %d\n, dwError);

        goto dacl_cleanup;

    }

 

    // Initialize a new security descriptor

    if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION))

    {

        wprintf(LInitializeSecurityDescriptor failed, error %d\n, GetLastError());

        goto dacl_cleanup;

    }

 

    // Set the new DACL in the security descriptor

    if (!SetSecurityDescriptorDacl(&sd, TRUE, pNewAcl, FALSE))

    {

        wprintf(LSetSecurityDescriptorDacl failed, error %d\n, GetLastError());

        goto dacl_cleanup;

    }

 

    // Set the new DACL for the service object

    if (!SetServiceObjectSecurity(schService, DACL_SECURITY_INFORMATION, &sd))

    {

        wprintf(LSetServiceObjectSecurity failed, error %d\n, GetLastError());

        goto dacl_cleanup;

    }

    else

            wprintf(LService DACL updated successfully\n);

 

dacl_cleanup:

    CloseServiceHandle(schSCManager);

    CloseServiceHandle(schService);

 

    if(NULL != pNewAcl)

        LocalFree((HLOCAL)pNewAcl);

    if(NULL != psd)

        HeapFree(GetProcessHeap(), 0, (LPVOID)psd);

}

 

// Purpose: Stops the service.

// Parameters: None

// Return value: None

VOID __stdcall DoStopSvc()

{

    SERVICE_STATUS_PROCESS ssp;

    DWORD dwStartTime = GetTickCount();

    DWORD dwBytesNeeded;

    DWORD dwTimeout = 30000; // 30-second time-out

    DWORD dwWaitTime;

 

    // Get a handle to the SCM database

    schSCManager = OpenSCManager(

        NULL,                    // local computer

        NULL,                    // ServicesActive database

        SC_MANAGER_ALL_ACCESS);  // full access rights

 

    if (NULL == schSCManager)

    {

        wprintf(LOpenSCManager failed, error %d\n, GetLastError());

        return;

    }

 

    // Get a handle to the service

    schService = OpenService(

        schSCManager,         // SCM database

        szSvcName,            // name of service

        SERVICE_STOP |

        SERVICE_QUERY_STATUS |

        SERVICE_ENUMERATE_DEPENDENTS); 

 

    if (schService == NULL)

    {

        wprintf(LOpenService failed, error %d\n, GetLastError());

        CloseServiceHandle(schSCManager);

        return;

    }   

 

    // Make sure the service is not already stopped

    if ( !QueryServiceStatusEx(

            schService,

            SC_STATUS_PROCESS_INFO,

            (LPBYTE)&ssp,

            sizeof(SERVICE_STATUS_PROCESS),

            &dwBytesNeeded ) )

    {

        wprintf(LQueryServiceStatusEx failed, error %d\n, GetLastError());

        goto stop_cleanup;

    }

 

    if ( ssp.dwCurrentState == SERVICE_STOPPED )

    {

        wprintf(L%s service is already stopped.\n, szSvcName);

        goto stop_cleanup;

    }

 

    // If a stop is pending, wait for it

    while ( ssp.dwCurrentState == SERVICE_STOP_PENDING )

    {

        wprintf(L%s service stop pending...\n, szSvcName);

 

        // Do not wait longer than the wait hint. A good interval is

        // one-tenth of the wait hint but not less than 1 second 

        // and not more than 10 seconds

        dwWaitTime = ssp.dwWaitHint / 10;

 

        if( dwWaitTime < 1000 )

            dwWaitTime = 1000;

        else if ( dwWaitTime > 10000 )

            dwWaitTime = 10000;

 

        Sleep( dwWaitTime );

 

        if ( !QueryServiceStatusEx(

                 schService,

                 SC_STATUS_PROCESS_INFO,

                 (LPBYTE)&ssp,

                 sizeof(SERVICE_STATUS_PROCESS),

                 &dwBytesNeeded ) )

        {

            wprintf(LQueryServiceStatusEx failed, error %d\n, GetLastError());

            goto stop_cleanup;

        }

 

        if ( ssp.dwCurrentState == SERVICE_STOPPED )

        {

            wprintf(L%s service stopped successfully.\n, szSvcName);

            goto stop_cleanup;

        }

 

        if ( GetTickCount() - dwStartTime > dwTimeout )

        {

            wprintf(LService stop timed out.\n);

            goto stop_cleanup;

        }

    }

 

    // If the service is running, dependencies must be stopped first

    StopDependentServices();

 

    // Send a stop code to the service

    if ( !ControlService(schService, SERVICE_CONTROL_STOP, (LPSERVICE_STATUS) &ssp ) )

    {

        wprintf(LControlService failed, error %d\n, GetLastError() );

        goto stop_cleanup;

    }

 

    // Wait for the service to stop

    while ( ssp.dwCurrentState != SERVICE_STOPPED )

    {

        Sleep( ssp.dwWaitHint );

        if ( !QueryServiceStatusEx(

                schService,

                SC_STATUS_PROCESS_INFO,

                (LPBYTE)&ssp,

                sizeof(SERVICE_STATUS_PROCESS),

                &dwBytesNeeded ) )

        {

            wprintf(LQueryServiceStatusEx() failed, error %d\n, GetLastError() );

            goto stop_cleanup;

        }

 

        if ( ssp.dwCurrentState == SERVICE_STOPPED )

            break;

 

        if ( GetTickCount() - dwStartTime > dwTimeout )

        {

            wprintf(LWait timed out...\n );

            goto stop_cleanup;

        }

    }

    wprintf(LService stopped successfully\n);

 

stop_cleanup:

    CloseServiceHandle(schService);

    CloseServiceHandle(schSCManager);

}

 

BOOL __stdcall StopDependentServices()

{

    DWORD i;

    DWORD dwBytesNeeded;

    DWORD dwCount;

 

    LPENUM_SERVICE_STATUS   lpDependencies = NULL;

    ENUM_SERVICE_STATUS     ess;

    SC_HANDLE               hDepService;

    SERVICE_STATUS_PROCESS  ssp;

 

    DWORD dwStartTime = GetTickCount();

    DWORD dwTimeout = 30000; // 30-second time-out

 

    // Pass a zero-length buffer to get the required buffer size.

    if ( EnumDependentServices( schService, SERVICE_ACTIVE, lpDependencies, 0, &dwBytesNeeded, &dwCount ) )

    {

         // If the Enum call succeeds, then there are no dependent

         // services, so do nothing.

         return TRUE;

    }

    else

    {

        if ( GetLastError() != ERROR_MORE_DATA )

            return FALSE; // Unexpected error

 

        // Allocate a buffer for the dependencies.

        lpDependencies = (LPENUM_SERVICE_STATUS) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwBytesNeeded );

 

        if ( !lpDependencies )

            return FALSE;

 

        __try {

            // Enumerate the dependencies.

            if ( !EnumDependentServices( schService, SERVICE_ACTIVE, lpDependencies, dwBytesNeeded, &dwBytesNeeded, &dwCount ) )

            return FALSE;

 

            for ( i = 0; i < dwCount; i++ )

            {

                ess = *(lpDependencies + i);

                // Open the service.

                hDepService = OpenService( schSCManager, ess.lpServiceName, SERVICE_STOP | SERVICE_QUERY_STATUS );

 

                if ( !hDepService )

                   return FALSE;

 

                __try {

                    // Send a stop code.

                    if ( !ControlService( hDepService, SERVICE_CONTROL_STOP,  (LPSERVICE_STATUS) &ssp ) )

                    return FALSE;

 

                    // Wait for the service to stop.

                    while ( ssp.dwCurrentState != SERVICE_STOPPED )

                    {

                        Sleep( ssp.dwWaitHint );

                        if ( !QueryServiceStatusEx(

                                hDepService,

                                SC_STATUS_PROCESS_INFO,

                                (LPBYTE)&ssp,

                                sizeof(SERVICE_STATUS_PROCESS),

                                &dwBytesNeeded ) )

                        return FALSE;

 

                        if ( ssp.dwCurrentState == SERVICE_STOPPED )

                            break;

 

                        if ( GetTickCount() - dwStartTime > dwTimeout )

                            return FALSE;

                    }

                }

                __finally

                {

                    // Always release the service handle.

                    CloseServiceHandle( hDepService );

                }

            }

        }

        __finally

        {

            // Always free the enumeration buffer.

            HeapFree( GetProcessHeap(), 0, lpDependencies );

        }

    }

    return TRUE;

}

 