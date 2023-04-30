<#
#̷𝓍   𝓐𝓡𝓢 𝓢𝓒𝓡𝓘𝓟𝓣𝓤𝓜
#̷𝓍   🇵​​​​​🇴​​​​​🇼​​​​​🇪​​​​​🇷​​​​​🇸​​​​​🇭​​​​​🇪​​​​​🇱​​​​​🇱​​​​​ 🇸​​​​​🇨​​​​​🇷​​​​​🇮​​​​​🇵​​​​​🇹​​​​​ 🇧​​​​​🇾​​​​​ 🇬​​​​​🇺​​​​​🇮​​​​​🇱​​​​​🇱​​​​​🇦​​​​​🇺​​​​​🇲​​​​​🇪​​​​​🇵​​​​​🇱​​​​​🇦​​​​​🇳​​​​​🇹​​​​​🇪​​​​​.🇶​​​​​🇨​​​​​@🇬​​​​​🇲​​​​​🇦​​​​​🇮​​​​​🇱​​​​​.🇨​​​​​🇴​​​​​🇲​​​​​
#>


function Invoke-IsAdministrator  {  
    (New-Object Security.Principal.WindowsPrincipal ([Security.Principal.WindowsIdentity]::GetCurrent())).IsInRole([Security.Principal.WindowsBuiltinRole]::Administrator)  
}
function Get-ScriptDirectory {
    Split-Path -Parent $PSCommandPath
}

      try{

        [string]$ErrorDetails=''
        [System.Boolean]$ErrorOccured=$False

        ########################################################################################
        #
        #  Read Arguments 
        #
        ########################################################################################
        $ExpectedNumberArguments = 3
        $Script:Arguments = $args.Clone()
        [System.Collections.ArrayList]$ArgumentList = [System.Collections.ArrayList]::new()
        0..$ExpectedNumberArguments |  % {
            $index = $_
            if([string]::IsNullOrEmpty($Script:Arguments[$index]) -eq $True){
             Write-Output "missing argument $index"
            }
            [void]$ArgumentList.Add("$($Script:Arguments[$index])")
        }

        [string]$Configuration          = $ArgumentList.Item(0)
        [string]$Platform               = $ArgumentList.Item(1)
        [string]$SolutionDirectory      = $ArgumentList.Item(2)
        [string]$BuiltBinary            = $ArgumentList.Item(3)

        [string]$SolutionDirectory      = (Resolve-Path "$SolutionDirectory").Path
        [string]$BuiltBinary            = (Resolve-Path "$BuiltBinary").Path

        [string]$OutputDirectory        = (Resolve-Path "$SolutionDirectory\bin\$Platform\$Configuration").Path
        [string]$ScriptsDirectory       = Join-Path $SolutionDirectory 'scripts'
        [string]$ProjectsPath           = Join-Path $SolutionDirectory 'vs'
        [string]$ReadmeFile             = Join-Path "$SolutionDirectory" "usage.txt"
        [string]$ServerCfgIniFile       = Join-Path "$SolutionDirectory" "servercfg.ini"
        [string]$VersionFile            = Join-Path "$ProjectsPath" "version_server.nfo"
        [string]$StatsFile              = Join-Path "$ProjectsPath" "stats_server.json"
        [string]$TmpStatsFile           = Join-Path "$ENV:TEMP" "stats_server.json"
        [string]$MD5HASH                = (Get-FileHash -Path "$BuiltBinary" -Algorithm MD5).Hash

        Write-Debug "########################################################################################"
        Write-Debug "                                    DEBUG ARGUMENTS                                     "
        Write-Debug "`tPlatform            ==> $Platform"
        Write-Debug "`tConfiguration       ==> $Configuration"
        Write-Debug "`tSolutionDirectory   ==> $SolutionDirectory"
        Write-Debug "`tScriptsDirectory    ==> $ScriptsDirectory"
        Write-Debug "`tOutputDirectory     ==> $OutputDirectory"
        Write-Debug "`tBuiltBinary         ==> $BuiltBinary"
        Write-Debug "`tReadmeFile          ==> $ReadmeFile"
        Write-Debug "`tServerCfgIniFile    ==> $ServerCfgIniFile"
        Write-Debug "`tVersionFile         ==> $VersionFile"
        Write-Debug "`tStatsFile           ==> $StatsFile"
        Write-Debug "########################################################################################"

        if(!(Test-Path "$BuiltBinary")){
            throw "ERROR: NO BINARY FOUND @`"$BuiltBinary`""
        }

        ########################################################################################
        #
        #  Get Dependencies
        #
        ########################################################################################

        [string[]]$deps = . "$ScriptsDirectory\dependencies\GetDependencies.ps1" -Path "$ScriptsDirectory\dependencies"
        $depscount = $deps.Count
        $deps | % {
            $file = $_
            . "$file"
            Write-Debug "Include `"$file`""
        }
        $Test = Test-Dependencies -Quiet
        if(! ($Test) ) { throw "dependencies error"} 

        [System.Boolean]$IsAdministrator = Invoke-IsAdministrator 

        ########################################################################################
        #
        #  READ INI FILE
        #
        ########################################################################################
        [System.Boolean]$ConfigureServiceRegistration = $False
        [System.Boolean]$ConfigureFirewall = $False
        [System.Boolean]$CodeMeterProtection = $False
        [System.Boolean]$ConfigureVersionSettings = $False
        [System.Boolean]$CopyDejaInsightLibraries = $False
        [string]$ProtekWbcFile = ''


        $ServerConfig = Get-IniContent "$ServerCfgIniFile"
        if([string]::IsNullOrEmpty($($ServerConfig.PostBuild.ConfigureVersionSettings)) -eq $False){
            [System.Boolean]$ConfigureVersionSettings = "$($ServerConfig.PostBuild.ConfigureVersionSettings)" -eq "1"
        }    
        if([string]::IsNullOrEmpty($($ServerConfig.PostBuild.ConfigureFirewall)) -eq $False){
            [System.Boolean]$ConfigureFirewall = "$($ServerConfig.PostBuild.ConfigureFirewall)" -eq "1"
        }
        if([string]::IsNullOrEmpty($($ServerConfig.PostBuild.CodeMeterProtection)) -eq $False){
            [System.Boolean]$CodeMeterProtection = "$($ServerConfig.PostBuild.CodeMeterProtection)" -eq "1"
        }
        if([string]::IsNullOrEmpty($($ServerConfig.PostBuild.ConfigureServiceRegistration)) -eq $False){
            [System.Boolean]$ConfigureServiceRegistration = "$($ServerConfig.PostBuild.ConfigureServiceRegistration)" -eq "1"
        }
        if([string]::IsNullOrEmpty($($ServerConfig.PostBuild.CopyDejaInsightLibraries)) -eq $False){
            [System.Boolean]$CopyDejaInsightLibraries = "$($ServerConfig.PostBuild.CopyDejaInsightLibraries)" -eq "1"
        }    
        if([string]::IsNullOrEmpty($($ServerConfig.PostBuild.ProtekWbcFile)) -eq $False){
            $ProtekWbcFile = $ExecutionContext.InvokeCommand.ExpandString($($ServerConfig.PostBuild.ProtekWbcFile))
            $ProtekWbcFile = (Resolve-Path -Path "$ProtekWbcFile" -ErrorAction Ignore).Path
            $IsProtekWbcPresent = [System.IO.File]::Exists($ProtekWbcFile)
        }
        if([string]::IsNullOrEmpty($($ServerConfig.PostBuild.ProductId)) -eq $False){
            [String]$ProductId = $($ServerConfig.PostBuild.ProductId)
        }
        
        Write-Line
        Write-BuildOutTitle "POST BUILD OPERATION"
        Write-Output "`tCopyDejaInsightLibs   $CopyDejaInsightLibraries"
        Write-Output "`tVersionSettings       $ConfigureVersionSettings"
        Write-Output "`tConfigureFirewall     $ConfigureFirewall"
        Write-Output "`tCodeMeterProtection   $CodeMeterProtection"
        Write-Output "`tServiceRegistration   $ConfigureServiceRegistration "
        Write-Line


        ########################################################################################
        #
        #  UPDATE VERSION STRING
        #
        ########################################################################################

        $HeadRev  = git log --format=%h -1 | select -Last 1
        $LastRev  = git log --format=%h -2 | select -Last 1
        $HeadRevisionLong = git log --format=%H -1

        [string]$VersionString = $($ServerConfig.Version.Version)
        [Version]$CurrentVersion = $VersionString
        [Version]$NewVersion = $VersionString
        $NewVersionRevision = $NewVersion.Revision
        $NewVersionRevision++
        $NewVersion = New-Object -TypeName System.Version -ArgumentList $NewVersion.Major,$NewVersion.Minor,$NewVersion.Build,$NewVersionRevision
        [string]$NewVersionString = $NewVersion.ToString()
        
        $ServerConfig.Version.MD5 = $MD5HASH
        $ServerConfig.Version.Version = $NewVersionString
        $ServerConfig.Version.Revision = $HeadRevisionLong 
        $UpdatedVersion = $NewVersionString

        $inf = Get-Item "$BuiltBinary"
        [DateTime]$LastWriteDate = $inf.LastWriteTime
        [String]$LastWriteTime = $LastWriteDate.GetDateTimeFormats()[26]
        $ServerConfig.Version.LastBuildTime = $LastWriteTime
      
        Out-IniFile -Path "$ServerCfgIniFile" -InputObject $ServerConfig

        ########################################################################################
        #
        #  CONFIGURE VERSION STRINGS IN BINARY
        #
        ########################################################################################

        if ($ConfigureVersionSettings -eq $True) {
            Write-BuildOutTitle "CONFIGURE VERSION SETTINGS"
            Set-BinaryFileVersionSettings -Path "$BuiltBinary" -Description "Remote Shell Server" -VersionString "$NewVersion"
            Set-BinaryFileVersionProperty -Path "$BuiltBinary" -PropertyName "company" -PropertyValue "$($ServerConfig.General.CompanyName)"  
            Set-BinaryFileVersionProperty -Path "$BuiltBinary" -PropertyName "copyright" -PropertyValue "$($ServerConfig.General.LegalCopyright)"  
            Set-BinaryFileVersionProperty -Path "$BuiltBinary" -PropertyName "LegalTrademarks" -PropertyValue "$($ServerConfig.General.LegalTrademarks)"  

            if($Configuration -match "Debug") { 
                Set-BinaryFileVersionProperty -Path "$BuiltBinary" -PropertyName "PrivateBuild" -PropertyValue "$($ServerConfig.General.PrivateBuild)"  
            }
            Write-Output "`tFileName              $($ServerConfig.General.Filename)"
            Write-Output "`tProductName           $($ServerConfig.General.ProductName)"
            Write-Output "`tCompanyName           $($ServerConfig.General.CompanyName)"
            Write-Output "`tFileDescription       $($ServerConfig.General.FileDescription)"
            Write-Output "`tLegalCopyright        $($ServerConfig.General.LegalCopyright)"
            Write-Output "`tPrivateBuild          $($ServerConfig.General.PrivateBuild)"
        }


        [System.Boolean]$IsAdministrator = Invoke-IsAdministrator 


        ########################################################################################
        #
        #  Configure Service Registration
        #
        ########################################################################################

        if ( ($ConfigureServiceRegistration -eq $True) -And ($IsAdministrator -eq $True) ){
            [string]$ServiceName = ""
            [string]$GroupName = ""
            [string]$ServiceDescription = ""
            [string]$StartupType = ""
            if([string]::IsNullOrEmpty($($ServerConfig.Service.ServiceName)) -eq $False){
                [string]$ServiceName = $($ServerConfig.Service.ServiceName)
            }    
            if([string]::IsNullOrEmpty($($ServerConfig.Service.GroupName)) -eq $False){
                [string]$GroupName = $($ServerConfig.Service.GroupName)
            }   
            if([string]::IsNullOrEmpty($($ServerConfig.Service.ServiceDescription)) -eq $False){
                [string]$ServiceDescription = $($ServerConfig.Service.ServiceDescription)
            }   
            if([string]::IsNullOrEmpty($($ServerConfig.Service.StartupType)) -eq $False){
                [string]$StartupType = $($ServerConfig.Service.StartupType)
            } 

            $IsDll = (((Get-Item -Path "$BuiltBinary").Extension) -eq '.dll')

            $Description = 
           
            Write-BuildOutTitle "CONFIGURE SERVICE REGISTRATION"
 

            if($IsDll){
                Write-Output "`tInstall-WinServiceExtended `"$ServiceName`" SharedProcess"
                Install-WinServiceExtended -Name "$ServiceName" -GroupName "$GroupName" -Path "$BuiltBinary" -Description "$ServiceDescription" -StartupType "$StartupType" -SharedProcess -ResetGroup
            }else{
                Write-Output "Install-WinServiceExtended -Name `"$ServiceName`" -Path `"$BuiltBinary`" -Description `"$ServiceDescription`" -StartupType `"$StartupType`" -OwnProcess"
                Install-WinServiceExtended -Name "$ServiceName" -Path "$BuiltBinary" -Description "$ServiceDescription" -StartupType "$StartupType" -OwnProcess -ResetGroup
            }
                
            Set-ServicePermissions -Name "$ServiceName" -Identity "$ENV:USERNAME" -Permission full
            Set-ServicePermissions -Name "$ServiceName" -Identity "NT AUTHORITY\SYSTEM" -Permission full
            Set-ServicePermissions -Name "$ServiceName" -Identity "NT AUTHORITY\SERVICE" -Permission full
            
        }

        ########################################################################################
        #
        #  CodeMeter Protection
        #
        ########################################################################################

        if ( ($IsProtekWbcPresent -eq $True) -And ($IsAdministrator -eq $True) -And ($CodeMeterProtection -eq $True) ){
            Invoke-CodeMeterProtection -TemplateWbc "$ProtekWbcFile" -InputFile "$BuiltBinary" -OutputFile "$BuiltBinary" -ProductId "$ProductId"
        }

        ########################################################################################
        #
        #  Copy DejaInsight Libraries
        #
        ########################################################################################


        if ($CopyDejaInsightLibraries -eq $True) {
            Write-BuildOutTitle "COPY DEJAINSIGHT LIBRARY ($Platform)"
            if($Platform -eq 'x64'){
                $DejaInsighDll = "$ENV:DejaToolsRootDirectory\lib\DejaInsight.x64.dll"
                Copy-Item $DejaInsighDll $OutputDirectory -Force
                Write-Output "[DEJA $Platform Dll] `"$DejaInsighDll`" ==> `"$OutputDirectory`""
            }else{
                $DejaInsighDll = "$ENV:DejaToolsRootDirectory\lib\DejaInsight.x86.dll"
                Copy-Item $DejaInsighDll $OutputDirectory -Force
                Write-Output "[DEJA $Platform Dll] `"$DejaInsighDll`" ==> `"$OutputDirectory`""
            }
        }

        ########################################################################################
        #
        #  Compiled Binary Statistics
        #
        ########################################################################################

        Update-BinaryStatistics "$BuiltBinary" "$StatsFile"


        ########################################################################################
        #
        #  Configure Firewall
        #
        ########################################################################################

        if ( ($ConfigureFirewall -eq $True) -And ($IsAdministrator -eq $True) ){
            Update-FirewallConfiguration "$ServerCfgIniFile"
        }

    }catch{
        #Enable-ExceptionDetailsTextBox
        ##Disable-ExceptionDetailsTextBox
        #Show-ExceptionDetails $_ -ShowStack
        [System.Management.Automation.ErrorRecord]$Record=$_
        $Line = [string]::new('=',120)
        $Spaces = [string]::new(' ',49)
        $ErrorOccured=$True
        Write-Output "`n`n`n`n$Line`n$Spaces POST BUILD SCRIPT ERROR`n$Line"
        $formatstring = "{0} {1}"
        $fields = $Record.FullyQualifiedErrorId,$Record.Exception.ToString()
        $ExceptMsg=($formatstring -f $fields)
        $Stack=$Record.ScriptStackTrace
        Write-Output "$ExceptMsg"
        Write-Output "$Stack" 
        
        exit -9
    }