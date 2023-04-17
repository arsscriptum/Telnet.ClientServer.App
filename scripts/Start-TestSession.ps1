

function Get-ScriptDirectory {
    Split-Path -Parent $PSCommandPath
}

function Start-TestSession {
   [CmdletBinding(SupportsShouldProcess)]
    param(
        [Parameter(Mandatory=$false)]
        [ValidateSet('Debug','Release')]
        [string]$Configuration
    )
      try{
        $ScriptDirectory = Get-ScriptDirectory
        $SolutionRoot = (Resolve-Path "$ScriptDirectory\..").Path
        $OutputDirectory = (Resolve-Path "$SolutionRoot\bin\Win32\$Configuration").Path
        $ClientApp = Join-Path "$OutputDirectory" "telnet_client.exe"
        $ServerApp = Join-Path "$OutputDirectory" "telnet_srv.exe"

        $ClientValid = Test-Path $ClientApp -PathType Leaf
        $ServerValid = Test-Path $ServerApp -PathType Leaf
    }catch{
        Show-ExceptionDetails $_ -ShowStack
       
    }
}




      try{


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


        if($Script:Configuration -eq "Debug"){
            $Text = $Text -f $TextDebug
        }else{
            $Text = $Text -f $TextRelease
        }
        
        Write-Output $Text

        $dep1 = "$PSScriptRoot\dependencies\Dependencies.ps1"
        . "$dep1"

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