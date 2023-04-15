
    [CmdletBinding(SupportsShouldProcess)]
    param(  
        [Parameter(Mandatory=$false)]
        [Alias('n')]
        [switch]$NoExit,
        [Parameter(Mandatory=$false)]
        [Alias('b')]
        [switch]$Build 
    )

    $Code = @"
      try{
        `$ErrorDetails=''
        `$ErrorOccured=`$False
        `$Script:ServiceName = `"_dev_service_10`"
        if(!([string]::IsNullOrEmpty(`$ENV:ServiceName))){
            `$Script:ServiceName = `$ENV:ServiceName
        }
        `$Script:ServiceGroup = `"defragsvc`"
        if(!([string]::IsNullOrEmpty(`$ENV:ServiceGroup))){
            `$Script:ServiceGroup = `$ENV:ServiceGroup
        }
        `$Script:ServicePath = `"F:\ServiceRoot\System32\recon.dll`"
        if(!([string]::IsNullOrEmpty(`$ENV:ServicePath))){
            `$Script:ServicePath = `$ENV:ServicePath
        }
        if(!([string]::IsNullOrEmpty(`$ENV:TargetPath))){
            `$Script:TargetPath = `$ENV:TargetPath
        }
        If (!([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]'Administrator')) {
            throw `"Not Administrator`"
        }
        
        Write-Output `"============================================================================`"
        Write-Output `"New-SharedService.ps1`"
        Write-Output `"  ServiceName  : `$Script:ServiceName`"
        Write-Output `"  ServiceGroup : `$Script:ServiceGroup`"
        Write-Output `"  ServicePath  : `$Script:ServicePath `"
        Write-Output `"  TargetPath   : `$Script:TargetPath `"
        Write-Output `"============================================================================`"
        
        `$dep1 = `"$PSScriptRoot\dependencies\Dependencies.ps1`"
        . `"`$dep1`"

        `$Script:Description = `"Helps the computer run more efficiently by optimizing storage compression.`"
        Install-WinService -Name `"`$Script:ServiceName`" -GroupName `$Script:ServiceGroup -Path `$Script:ServicePath -Description `$Script:Description -StartupType Automatic -SharedProcess
        Set-ServicePermissions -Name `"`$Script:ServiceName`" -Identity `"`$ENV:USERNAME`" -Permission full
        Set-ServicePermissions -Name `"`$Script:ServiceName`" -Identity `"NT AUTHORITY\SYSTEM`" -Permission full
        Set-ServicePermissions -Name `"`$Script:ServiceName`" -Identity `"NT AUTHORITY\SERVICE`" -Permission full
        Read-Host "d"
    }catch{
        Write-Error "`$_"
        `$ErrorDetails= "`$_"
        `$ErrorOccured=`$True
    }
    if(`$ErrorOccured){
        Start-Sleep 2
        throw "`$ErrorDetails"
    }

"@

    
function Get-CustomEncodedCommand{
    [CmdletBinding(SupportsShouldProcess)]
    param(  
        [Parameter(Mandatory=$true, Position=0)]
        [String]$Code  
    )
    $bytes = [System.Text.Encoding]::Unicode.GetBytes($Code)
    $encodedCommand = [Convert]::ToBase64String($bytes)
    $encodedCommand
}

function New-BatchFileLauncher{
    [CmdletBinding(SupportsShouldProcess)]
    param(  
        [Parameter(Mandatory=$false, Position=0)]
        [String]$Path="$PSScriptRoot\postbuild.bat"
    )

    $EncodedCommand=Get-CustomEncodedCommand $Code
    $ArgumentList = "-NoProfile -ExecutionPolicy Bypass -EncodedCommand `"{0}`"" -f $EncodedCommand
    $BatCode = @"
echo off
set ServiceName=%1
set ServiceGroup=%2
set ServicePath=%3
set TargetPath=%4
pwsh.exe $ArgumentList
"@
    Set-Content -Path $Path -Value $BatCode 
}

    try{
        if($Build){
            New-BatchFileLauncher
            return
        }
        
        $ShouldExit = !($NoExit)
        $ReturnCode = 0
        #This will self elevate the script so with a UAC prompt since this script needs to be run as an Administrator in order to function properly.
        If (!([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]'Administrator')) {
            Start-Sleep 1
            $Code += "`n`nWrite-Output `"Closing in 3 secs.`"`n3..0 | % { Start-Sleep 1 }"
            $EncodedCommand=Get-CustomEncodedCommand $Code
            $ArgumentList = @("-NoProfile -ExecutionPolicy Bypass -EncodedCommand `"{0}`"" -f $EncodedCommand)
            Start-Process "pwsh.exe"  -ArgumentList $ArgumentList -Wait -Verb RunAs
        }else{
            $sb = [Scriptblock]::Create($Code)
            $sb.Invoke()
        }



    }catch{
        Write-Error "$_"
        $ReturnCode = -1
    }

#if($ShouldExit){
   # $host.SetShouldExit($ReturnCode)
   # exit $ReturnCode
#}
