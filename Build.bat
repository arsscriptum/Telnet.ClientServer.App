@echo off
setlocal EnableDelayedExpansion

:: ==============================================================================
:: 
::      Build.bat
::
::      Build different configuration of the app
::
:: ==============================================================================
::   arsccriptum - made in quebec 2020 <guillaumeplante.qc@gmail.com>
:: ==============================================================================

goto :init

:init
    set "__scripts_root=%AutomationScriptsRoot%"
    call :read_script_root development\build-automation  BuildAutomation
    set "__script_file=%~0"
    set "__target=%~1"
    set "__script_path=%~dp0"
    set "__makefile=%__scripts_root%\make\make.bat"
    set "__lib_date=%__scripts_root%\batlibs\date.bat"
    set "__lib_out=%__scripts_root%\batlibs\out.bat"
    ::*** This is the important line ***
    set "__build_cfg=%__script_path%buildcfg.ini"
    set "__build_cfg_agent=%__script_path%agent.ini"
    set "__build_cfg_ctrl=%__script_path%ctrl.ini"
    set "__build_cfg_lnch=%__script_path%launcher.ini"
    set "__bin_path=%__script_path%bin"
    set "__tmp_path=%__script_path%vs\___temp_compilation_files"
    set "__build_cancelled=0"
    goto :validate


:header
    echo. %__script_name% v%__script_version%
    echo.    This script is part of arsccriptum build wrappers.
    echo.
    goto :eof

:header_err
    echo.**************************************************
    echo.This script is part of arsccriptum build wrappers.
    echo.**************************************************
    echo.
    echo. YOU NEED TO HAVE THE BuildAutomation Scripts setup on you system...
    echo. https://github.com/arsccriptum/BuildAutomation
    goto :eof


:read_script_root
    set regpath=%OrganizationHKCU::=%
    for /f "tokens=2,*" %%A in ('REG.exe query %regpath%\%1 /v %2') do (
            set "__scripts_root=%%B"
        )
    goto :eof

:validate
    if not defined __scripts_root          call :header_err && call :error_missing_path __scripts_root & goto :eof
    if not exist %__makefile%  call :error_missing_script "%__makefile%" & goto :eof
    if not exist %__lib_date%  call :error_missing_script "%__lib_date%" & goto :eof
    if not exist %__lib_out%  call :error_missing_script "%__lib_out%" & goto :eof
    if not exist %__build_cfg%  call :error_missing_script "%__build_cfg%" & goto :eof

    goto :prebuild_header




:prebuild_header
    call %__lib_date% :getbuilddate
    call %__lib_out% :__out_d_red " ======================================================================="
    call %__lib_out% :__out_l_red " Compilation started for %cd%  %__target%"  
    call %__lib_out% :__out_d_red " ======================================================================="
    call :build
    goto :eof


:prebuild_clean
    call %__lib_out% :__out_d_yel " ======================================================================="
    call %__lib_out% :__out_l_yel "                              CLEANING                                  "  
    call %__lib_out% :__out_d_yel " ======================================================================="
    call %__lib_out% :__out_n_d_cya "DELETE "  
    call %__lib_out% :__out_d_grn "%__tmp_path%"  
    rmdir /Q /S %__tmp_path% 2> NUL
    call %__lib_out% :__out_n_d_cya "DELETE "  
    call %__lib_out% :__out_d_grn "%__bin_path%"  
    rmdir /Q /S %__bin_path% 2> NUL

    goto :eof

:: ==============================================================================
::   call make
:: ==============================================================================
:call_make_build_agent
    set config=%1
    set platform=%2
    call %__makefile% /v /i %__build_cfg_agent% /t Build /c %config% /p %platform%
    goto :finished

:: ==============================================================================
::   call make
:: ==============================================================================
:call_make_build_ctrl
    set config=%1
    set platform=%2
    call %__makefile% /v /i %__build_cfg_ctrl% /t Build /c %config% /p %platform%
    goto :finished

:: ==============================================================================
::   call make
:: ==============================================================================
:call_make_build_lnch
    set config=%1
    set platform=%2
    call %__makefile% /v /i %__build_cfg_lnch% /t Build /c %config% /p %platform%
    goto :finished

:: ==============================================================================
::   call make
:: ==============================================================================
:call_make_build
    set config=%1
    set platform=%2
    call %__makefile% /v /i %__build_cfg% /t Build /c %config% /p %platform%
    goto :finished

:call_make_build_export
    set config=%1
    set platform=%2
    set export_path=%3
    call %__makefile% /v /i %__build_cfg% /t Build /c %config% /p %platform% /x %export_path%
    goto :finished


:: =============================================================================
::   Build Win32
:: ==============================================================================
:build_Win32
    set config=%1
    call :call_make_build %config% Win32
    goto :eof


:: ==============================================================================
::   Build x64
:: ==============================================================================
:build_x64
    set config=%1
    call :call_make_build %config% x64
    goto :eof


:: ==============================================================================
::   clean all
:: ==============================================================================
:clean
    call %__makefile% /v /i %__build_cfg% /t Clean /c Debug /p x86
    call %__makefile% /v /i %__build_cfg% /t Clean /c Release /p x86
    call %__makefile% /v /i %__build_cfg% /t Clean /c Debug /p x64
    call %__makefile% /v /i %__build_cfg% /t Clean /c Release /p x64
    goto :eof

:ask 
    set /p QConfirm= "Continue? [y/N] "
    if %QConfirm%==y (goto :continue_build) ELSE (goto :eof)

:: ==============================================================================
::   Build
:: ==============================================================================
:build
    call :build_x64 Debug
    goto :eof

:error_missing_path
    echo.
    echo   Error
    echo    Missing path: %~1
    echo.
    goto :eof



:error_missing_script
    echo.
    echo    Error
    echo    Missing bat script: %~1
    echo.
    goto :eof



:finished
    call %__lib_out% :__out_d_grn "Build complete"
