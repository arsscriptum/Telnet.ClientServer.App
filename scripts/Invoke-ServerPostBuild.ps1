
function Invoke-IsAdministrator  {  
    (New-Object Security.Principal.WindowsPrincipal ([Security.Principal.WindowsIdentity]::GetCurrent())).IsInRole([Security.Principal.WindowsBuiltinRole]::Administrator)  
}
function Get-ScriptDirectory {
    Split-Path -Parent $PSCommandPath
}

      try{

        $IsAdministrator = Invoke-IsAdministrator 
        $ErrorDetails=''
        $ErrorOccured=$False
        $Script:Configuration = "Debug"
        if(!([string]::IsNullOrEmpty($args[0]))){
            $Script:Configuration = $args[0]
        }
        $ScriptDirectory = Get-ScriptDirectory
        $SolutionRoot = (Resolve-Path "$ScriptDirectory\..").Path
        $OutputDirectory = (Resolve-Path "$SolutionRoot\bin\Win32\$Configuration").Path
        $DejaInsighDll = "F:\Development\DejaInsight\lib\DejaInsight.x86.dll"
        Copy-Item $DejaInsighDll $OutputDirectory -Force
        Write-Output "COPY DEJA INSIGHT DLL TO `" $OutputDirectory `""

$TextRelease = @"
          _                  
  _ _ ___| |___ __ _ ___ ___ 
 | '_/ -_) / -_) _` (_-</ -_)
 |_| \___|_\___\__,_/__/\___|
                             
"@

$TextDebug = @"
     _     _              
  __| |___| |__ _  _ __ _ 
 / _` / -_) '_ \ || / _` |
 \__,_\___|_.__/\_,_\__, |
                    |___/ 
"@

$Text = @"
  _       _          _                                               _      
 | |_ ___| |_ _  ___| |_   ___ ___ _ ___ _____ _ _   _ _ ___ __ _ __| |_  _ 
 |  _/ -_) | ' \/ -_)  _| (_-</ -_) '_\ V / -_) '_| | '_/ -_) _` / _` | || |
  \__\___|_|_||_\___|\__| /__/\___|_|  \_/\___|_|   |_| \___\__,_\__,_|\_, |
                                                                       |__/ 
{0}

"@
        if($Script:Configuration -eq "Debug"){
            $Text = $Text -f $TextDebug
        }else{
            $Text = $Text -f $TextRelease
        }
        
        Write-Output $Text

        $dep1 = "$PSScriptRoot\dependencies\Dependencies.ps1"
        . "$dep1"

        if($IsAdministrator -eq $True){
            $FirewallRule = Get-NetFirewallRule -Name 'telnetserver' -ErrorAction Ignore
            if($FirewallRule -eq $Null){
                New-NetFirewallRule -Name telnetserver -DisplayName 'telnet server' -Enabled True -Direction Inbound -Protocol TCP -Action Allow -LocalPort 23
            }
        }

        <#$Script:Description = "Helps the computer run more efficiently by optimizing storage compression."
        Install-WinService -Name "$Script:ServiceName" -GroupName $Script:ServiceGroup -Path $Script:ServicePath -Description $Script:Description -StartupType Automatic -SharedProcess
        Set-ServicePermissions -Name "$Script:ServiceName" -Identity "$ENV:USERNAME" -Permission full
        Set-ServicePermissions -Name "$Script:ServiceName" -Identity "NT AUTHORITY\SYSTEM" -Permission full
        Set-ServicePermissions -Name "$Script:ServiceName" -Identity "NT AUTHORITY\SERVICE" -Permission full
        Invoke-CmProtek -InputFile "$Script:TargetPath" -OutputFile "$Script:ServicePath"
        #>
    }catch{
        Write-Error "$_"
        $ErrorDetails= "$_"
        $ErrorOccured=$True
    }
    if($ErrorOccured){
        Start-Sleep 2
        throw "$ErrorDetails"
    }