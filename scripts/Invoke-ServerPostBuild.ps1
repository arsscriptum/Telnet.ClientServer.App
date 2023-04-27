
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

        $Script:Arguments = $args.Clone()
        [System.Collections.ArrayList]$ArgumentList = [System.Collections.ArrayList]::new()
        0..3 |  % {
            $index = $_
            if([string]::IsNullOrEmpty($Script:Arguments[$index]) -eq $True){
             Write-Output "missing argument $index"
            }
            [void]$ArgumentList.Add("$($Script:Arguments[$index])")
        }

        [string]$Configuration          = $ArgumentList.Item(0)
        [string]$Platform               = $ArgumentList.Item(1)
        [string]$SolutionDirectory      = (Resolve-Path "$($ArgumentList.Item(2))" -ErrorAction Ignore).Path
        [string]$BuiltBinary            = (Resolve-Path "$($ArgumentList.Item(3))" -ErrorAction Ignore).Path

        if(!(Test-Path "$BuiltBinary")){
            throw "ERROR: NO BINARY FOUND @`"$BuiltBinary`""
        }

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
        Write-Debug "                                    ARGUMENTS"
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


        ########################################################################################
        #
        #  READ INI FILE
        #
        ########################################################################################

        [System.Boolean]$ConfigureServiceRegistration = $False
        [System.Boolean]$ConfigureFirewall = $False
        [System.Boolean]$CodeMeterProtection = $False
        [System.Boolean]$ConfigureVersionSettings = $True
        [System.Boolean]$CopyDejaInsightLibraries = $True
        [string]$ProtekWbcFile = ''


        $ServerConfig = Get-IniContent "$ServerCfgIniFile"
       
        if([string]::IsNullOrEmpty($($ServerConfig.PostBuild.ConfigureFirewall)) -eq $False){
            [System.Boolean]$ConfigureFirewall = "$($ServerConfig.PostBuild.ConfigureFirewall)" -eq "1"
        }
        if([string]::IsNullOrEmpty($($ServerConfig.PostBuild.CodeMeterProtection)) -eq $False){
            [System.Boolean]$CodeMeterProtection = "$($ServerConfig.PostBuild.CodeMeterProtection)" -eq "1"
        }
        if([string]::IsNullOrEmpty($($ServerConfig.PostBuild.ConfigureServiceRegistration)) -eq $False){
            [System.Boolean]$ConfigureServiceRegistration = "$($ServerConfig.PostBuild.ConfigureServiceRegistration)" -eq "1"
        }
        if([string]::IsNullOrEmpty($($ServerConfig.PostBuild.ProtekWbcFile)) -eq $False){
            $ProtekWbcFile = $ExecutionContext.InvokeCommand.ExpandString($($ServerConfig.PostBuild.ProtekWbcFile))
            $ProtekWbcFile = (Resolve-Path -Path "$ProtekWbcFile" -ErrorAction Ignore).Path
            $IsProtekWbcPresent = [System.IO.File]::Exists($ProtekWbcFile)
        }
        if([string]::IsNullOrEmpty($($ServerConfig.PostBuild.ProductId)) -eq $False){
            [String]$ProductId = $($ServerConfig.PostBuild.ProductId)
        }
        
        
        Write-BuildOutTitle "SERVER CONFIGURATION INI"
        Write-Output "`tFileName              $($ServerConfig.General.Filename)"
        Write-Output "`tProductName           $($ServerConfig.General.ProductName)"
        Write-Output "`tCompanyName           $($ServerConfig.General.CompanyName)"
        Write-Output "`tFileDescription       $($ServerConfig.General.FileDescription)"
        Write-Output "`tLegalCopyright        $($ServerConfig.General.LegalCopyright)"
        Write-Output "`tPrivateBuild          $($ServerConfig.General.PrivateBuild)"
        Write-Line
        Write-BuildOutTitle "POST BUILD OPERATION"
        Write-Output "`tConfigureFirewall     $ConfigureFirewall"
        Write-Output "`tCodeMeterProtection   $CodeMeterProtection"
        Write-Output "`tServiceRegistration   $ConfigureServiceRegistration "
        Write-Output "`tProtekWbcFile         $ProtekWbcFile"
        Write-Output "`tIsProtekWbcPresent    $IsProtekWbcPresent"
        Write-Output "`tProductId             $ProductId"
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
        }
        if($Configuration -match "Debug") { 
            Set-BinaryFileVersionProperty -Path "$BuiltBinary" -PropertyName "PrivateBuild" -PropertyValue "$($ServerConfig.General.PrivateBuild)"  
        }

        ########################################################################################
        #
        #  Configure Firewall
        #
        ########################################################################################

        [System.Boolean]$IsAdministrator = Invoke-IsAdministrator 

        if ( ($ConfigureFirewall -eq $True) -And ($IsAdministrator -eq $True) ){
            Write-BuildOutTitle "CONFIGURE NET FIREWALL RULES"
            0 .. 20 | % {
                $id = $_ 
                $SectionName = "FirewallRule_{0:d3}" -f $id 
                $FirewallRule = ($ServerConfig.$SectionName  -as 'Hashtable')
                if($FirewallRule -eq $Null) { break }
                $Rule = Get-NetFirewallRule -Name "$($FirewallRule.Name)" -ErrorAction Ignore
                if($Rule -ne $Null){
                    Write-Output "------------------------------------------------"
                    Write-Output "REMOVING FIREWALL RULE `"$($FirewallRule.Name)`""
                    Get-NetFirewallRule -Name "$($FirewallRule.Name)" | Remove-NetFirewallRule
                    Write-Output "------------------------------------------------`n"
                }
                Write-Line 40 '-'
                Write-Output "ADDING FIREWALL RULE `"$($FirewallRule.Name)`""
                Write-Output "New-NetFirewallRule -Name `"$($FirewallRule.Name)`" -DisplayName `"$($FirewallRule.DisplayName)`" -Enabled True -Direction `"$($FirewallRule.Direction)`" -Protocol `"$($FirewallRule.Protocol)`" -Action `"$($FirewallRule.Action)`" -LocalPort `"$($FirewallRule.LocalPort)`" -Description `"$($FirewallRule.Description)`""       
                Write-Line 40 '-'
                $Res = New-NetFirewallRule -Name "$($FirewallRule.Name)" -DisplayName "$($FirewallRule.DisplayName)" -Enabled True -Direction "$($FirewallRule.Direction)" -Protocol "$($FirewallRule.Protocol)" -Action "$($FirewallRule.Action)" -LocalPort "$($FirewallRule.LocalPort)" -Description "$($FirewallRule.Description)"
                Write-Output "Rule $($Res.Name) : $($Res.Status)"
            }
        }

        ########################################################################################
        #
        #  Configure Service Registration
        #
        ########################################################################################


        if ( ($ConfigureServiceRegistration -eq $True) -And ($IsAdministrator -eq $True) ){
            Write-BuildOutTitle "CONFIGURE SERVICE REGISTRATION"
            $Description = "Helps the computer run more efficiently by optimizing storage compression."
            Install-WinService -Name "$ServiceName" -GroupName $ServiceGroup -Path $ServicePath -Description $Description -StartupType Automatic -SharedProcess
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
            Write-BuildOutTitle "CODEMETER BINARY ENCRYPTION"
            $NewName = (Get-Item "$BuiltBinary").Basename + "-CLEAR"
            $Directory = (Get-Item "$BuiltBinary").DirectoryName
            $Extension = (Get-Item "$BuiltBinary").Extension
            $NewFullPath = "{0}\{1}{2}" -f $Directory, $NewName, $Extension
            Copy-Item "$BuiltBinary" "$NewFullPath"
            Invoke-CmProtek -TemplateWbc "$ProtekWbcFile" -InputFile "$BuiltBinary" -OutputFile "$BuiltBinary" -ProductId "$ProductId"
        }

        ########################################################################################
        #
        #  Copy DejaInsight Libraries
        #
        ########################################################################################


        if ($CopyDejaInsightLibraries -eq $True) {
            Write-BuildOutTitle "COPY DEJAINSIGHT LIBRARY"
            $DejaInsighDll = "$ENV:DejaToolsRootDirectory\lib\DejaInsight.x86.dll"
            Copy-Item $DejaInsighDll $OutputDirectory -Force

            Write-Output "COPY DEJA INSIGHT DLL TO `" $OutputDirectory `""
        }

        ########################################################################################
        #
        #  Compiled Binary Statistics
        #
        ########################################################################################

        Update-BinaryStatistics "$BuiltBinary" "$StatsFile"

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