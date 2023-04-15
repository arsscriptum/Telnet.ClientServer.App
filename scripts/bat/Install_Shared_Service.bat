
set GROUPNAME=_dev_group_test
SET SERVICENAME=_dev_test_01
copy F:\Code\recon\bin\x64\Release\recon.dll F:\ServiceRoot\System32\recon.dll
sc create %SERVICENAME% binPath= ^%%SystemRoot^%%"\system32\svchost.exe -k %GROUPNAME%" type= share start= demand
reg add HKLM\SYSTEM\CurrentControlSet\services\%SERVICENAME%\Parameters /v ServiceDll /t REG_EXPAND_SZ /d F:\ServiceRoot\System32\recon.dll /f
reg add "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Svchost" /v %GROUPNAME% /t REG_MULTI_SZ /d %SERVICENAME% /f