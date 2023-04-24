
function Invoke-IsAdministrator  {  
    (New-Object Security.Principal.WindowsPrincipal ([Security.Principal.WindowsIdentity]::GetCurrent())).IsInRole([Security.Principal.WindowsBuiltinRole]::Administrator)  
}
function Get-ScriptDirectory {
    Split-Path -Parent $PSCommandPath
}
      try{
        $ConfigureFirewall=$False
        $IsAdministrator = Invoke-IsAdministrator 
        $ErrorDetails=''
        $ErrorOccured=$False
        $Script:Configuration = "Debug"
        if(!([string]::IsNullOrEmpty($args[0]))){
            $Script:Configuration = $args[0]
            Write-Output "Configuration ==> $Script:Configuration"
        }else{
            throw "missing argument 0"
        }
        if(!([string]::IsNullOrEmpty($args[1]))){
            $RootPath = $args[1]
            Write-Output "RootPath ==> $RootPath"
        }else{
            throw "missing argument 1"
        }
        $SolutionDirectory = (Resolve-Path $RootPath).Path
        
        $ScriptsDirectory = (Resolve-Path "$SolutionDirectory\scripts").Path
        $OutputDirectory = (Resolve-Path "$SolutionDirectory\bin\Win32\$Configuration").Path
        $BuiltExecutable = Join-Path "$OutputDirectory" "telnet_srv.exe"
        $ReadmeFile  = Join-Path "$SolutionDirectory" "usage.txt"
        Write-Output "=========================================================="
        Write-Output "                   DIRECTORIES CONFIGURATION              "
        Write-Output "`tSolutionDirectory ==> $SolutionDirectory"
        Write-Output "`tScriptsDirectory  ==> $ScriptsDirectory"
        Write-Output "`tOutputDirectory   ==> $OutputDirectory"
        Write-Output "=========================================================="
        [string[]]$deps = . "$ScriptsDirectory\dependencies\GetDependencies.ps1" -Path "$ScriptsDirectory\dependencies"
        $depscount = $deps.Count
        $deps | % {
            $file = $_
            . "$file"
            #Write-Output "Include `"$file`""
        }
        $Test = Test-Dependencies -Quiet
        if(! ($Test) ) { throw "dependencies error"} 

        
        Write-Output "`n=========================================================="
        Write-Output "                    COPY DEJAINSIGHT LIBRARY                "
        Write-Output "==========================================================`n"
        $DejaInsighDll = "$ENV:DejaToolsRootDirectory\lib\DejaInsight.x86.dll"
        Copy-Item $DejaInsighDll $OutputDirectory -Force

        Write-Output "COPY DEJA INSIGHT DLL TO `" $OutputDirectory `""

        Write-Output "`n=========================================================="
        Write-Output "                  CONFIGURE VERSION SETTINGS              "
        Write-Output "==========================================================`n"
        Set-BinaryFileVersionSettings -Path "$BuiltExecutable" -Description "Remote Shell Server"

        Set-BinaryFileVersionProperty -Path "$BuiltExecutable" -PropertyName "company" -PropertyValue "arsscriptum"
        Set-BinaryFileVersionProperty -Path "$BuiltExecutable" -PropertyName "copyright" -PropertyValue "(c) arsscriptum 2022"
        Set-BinaryFileVersionProperty -Path "$BuiltExecutable" -PropertyName "LegalTrademarks" -PropertyValue "(tm) arsscriptum"
        if($ConfigureFirewall){
            Write-Output "=========================================================="
            Write-Output "                CONFIGURE NET FIREWALL RULES              "
             Write-Output "=========================================================="
            if($IsAdministrator -eq $True){
                $FirewallRule = Get-NetFirewallRule -Name 'remote_shell_server' -ErrorAction Ignore
                if($FirewallRule -eq $Null){
                    Write-Output "Add New Rule"
                    New-NetFirewallRule -Name "remote_shell_server" -DisplayName 'remote_shell_server' -Enabled True -Direction Inbound -Protocol TCP -Action Allow -LocalPort 34010-34050
                }
            }
        }

       

        $TextConfig = $Script:TextRelease
        if("$Configuration" -eq "Debug"){
          $TextConfig = $Script:TextDebug
        }
        $OutLog = $Script:TextServerReady -f $TextConfig
        Write-Output $OutLog

        Write-Output "=========================================================="
        Write-Output "              CONFIGURE SERVICE REGISTRATION              "
        Write-Output "=========================================================="
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