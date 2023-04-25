
function Invoke-IsAdministrator  {  
    (New-Object Security.Principal.WindowsPrincipal ([Security.Principal.WindowsIdentity]::GetCurrent())).IsInRole([Security.Principal.WindowsBuiltinRole]::Administrator)  
}
function Get-ScriptDirectory {
    Split-Path -Parent $PSCommandPath
}
function New-StatsFile{
    [CmdletBinding(SupportsShouldProcess)]
    param(
        [parameter(mandatory=$true,Position=0)]
        [String]$Path,
        [parameter(mandatory=$false)]
        [String]$Version="1.0.0.1"
    )
    $o = [PsCustomObject]@{
        Revision = "0A0A0A0A0A"
        Version = "1.0.0.1"
        Bytes = 0
        DiffBytes = 0
        Size = 0
    }
    [System.Collections.Generic.List[PsCustomObject]]$a = [System.Collections.Generic.List[PsCustomObject]]::new()
    $a.Add($o)
    $a.Add($o)
    $all = @{
        "Debug" = $a
        'Release' = $a
        'DebugDLL' = $a
        'ReleaseDLL' = $a
        'DebugDLL_DynamicMFC' = $a
        'ReleaseDLL_DynamicMFC' = $a
    }
    $NewStats = $all
    $NewStatsJson = $NewStats | ConvertTo-Json
    Set-Content $Path $NewStatsJson
}
      try{
        [System.Boolean]$ConfigureFirewall=$False
        $IsAdministrator = Invoke-IsAdministrator 
        $ErrorDetails=''
        $ErrorOccured=$False
        $Script:Configuration = "Debug"
        if(!([string]::IsNullOrEmpty($args[0]))){
            $Script:Configuration = $args[0]
            Write-Host "Configuration ==> $Script:Configuration"
        }else{
            throw "missing argument 0"
        }
        $Script:Platform = "x64"
        if(!([string]::IsNullOrEmpty($args[1]))){
            $Script:Platform = $args[1]
            Write-Host "Platform ==> $Script:Platform"
        }else{
            throw "missing argument 1"
        }
        if(!([string]::IsNullOrEmpty($args[2]))){
            $RootPath = $args[2]
            Write-Host "RootPath ==> $RootPath"
        }else{
            throw "missing argument 2"
        }
        if(!([string]::IsNullOrEmpty($args[3]))){
            [System.Boolean]$ConfigureFirewall = $args[3]
            Write-Host "ConfigureFirewall ==> $ConfigureFirewall"
        }
        $Script:OutFile = "telnet_srv.exe"
        if($Script:Configuration -match "DLL") { $Script:OutFile = "telnet_srv.dll" }
        
        $SolutionDirectory = (Resolve-Path $RootPath).Path
        
        $ScriptsDirectory = (Resolve-Path "$SolutionDirectory\scripts").Path
        $OutputDirectory = (Resolve-Path "$SolutionDirectory\bin\$Script:Platform\$Script:Configuration").Path
        $ProjectsPath = Join-Path $SolutionDirectory 'vs'
        $BuiltExecutable = Join-Path "$OutputDirectory" "$Script:OutFile"
        $ReadmeFile  = Join-Path "$SolutionDirectory" "usage.txt"
        $ServerCfgIniFile  = Join-Path "$SolutionDirectory" "servercfg.ini"
        $VersionFile  = Join-Path "$ProjectsPath" "version_server.nfo"
        $StatsFile  = Join-Path "$ProjectsPath" "stats_server.json"
        $TmpStatsFile  = Join-Path "$ENV:TEMP" "stats_server.json"
        $MD5HASH = (Get-FileHash -Path "$BuiltExecutable" -Algorithm MD5).Hash

        Write-Host "=========================================================="
        Write-Host "                   DIRECTORIES CONFIGURATION              "
        Write-Host "`tSolutionDirectory ==> $SolutionDirectory"
        Write-Host "`tScriptsDirectory  ==> $ScriptsDirectory"
        Write-Host "`tOutputDirectory   ==> $OutputDirectory"
        Write-Host "=========================================================="
        [string[]]$deps = . "$ScriptsDirectory\dependencies\GetDependencies.ps1" -Path "$ScriptsDirectory\dependencies"
        $depscount = $deps.Count
        $deps | % {
            $file = $_
            . "$file"
            #Write-Host "Include `"$file`""
        }
        $Test = Test-Dependencies -Quiet
        if(! ($Test) ) { throw "dependencies error"} 

        if(!(Test-Path "$BuiltExecutable")){
            Write-Host "ERROR: NO BINARY FOUND @`"$BuiltExecutable`""
            exit -1
        }
        

        $ServerConfig = Get-IniContent "$ServerCfgIniFile"

        Write-Host "=========================================================="
        Write-Host "                   SERVER CONFIGURATION INI               "
        Write-Host "  FileName          $($ServerConfig.General.Filename)"
        Write-Host "  ProductName       $($ServerConfig.General.ProductName)"
        Write-Host "  CompanyName       $($ServerConfig.General.CompanyName)"
        Write-Host "  FileDescription   $($ServerConfig.General.FileDescription)"
        Write-Host "  LegalCopyright    $($ServerConfig.General.LegalCopyright)"
        Write-Host "  PrivateBuild      $($ServerConfig.General.PrivateBuild)"
        Write-Host "=========================================================="

        Write-Host "`n`n==============================================================================="
        Write-Host "GETTING MODULE VERSION"
        Write-Host "==============================================================================="   
        $HeadRev                        = git log --format=%h -1 | select -Last 1
        $LastRev                        = git log --format=%h -2 | select -Last 1
        $HeadRevisionLong = git log --format=%H -1
        ##if(-not(Test-Path $VersionFile)){
        #    throw "Missing Version File $VersionFile"
        #}

        #[string]$VersionString = '99.99.98'
        #if(Test-Path $VersionFile){
        #    [string]$VersionString = (Get-Content -Path $VersionFile -Raw)
        #}else{
        #    throw "Missing Version File $VersionFile"
        #}  
        [string]$VersionString = $($ServerConfig.Version.Version)
        [Version]$CurrentVersion = $VersionString
        [Version]$NewVersion = $VersionString
        $NewVersionRevision = $NewVersion.Revision
        $NewVersionRevision++
        $NewVersion = New-Object -TypeName System.Version -ArgumentList $NewVersion.Major,$NewVersion.Minor,$NewVersion.Build,$NewVersionRevision
        [string]$NewVersionString = $NewVersion.ToString()
        
        
        #Set-Content -Path $VersionFile -Value $NewVersionString
        $ServerConfig.Version.MD5 = $MD5HASH
        $ServerConfig.Version.Version = $NewVersionString
        $ServerConfig.Version.Revision = $HeadRevisionLong 
        $UpdatedVersion = $NewVersionString


        $inf = Get-Item "$BuiltExecutable"
        [DateTime]$LastWriteDate = $inf.LastWriteTime
        [String]$LastWriteTime = $LastWriteDate.GetDateTimeFormats()[26]
        $FileLengthBytes = $inf.Length 
        $SizePretty = Convert-Bytes -Size $FileLengthBytes -Format MB
        $ServerConfig.Version.LastBuildTime = $LastWriteTime
        Write-Host "UPDATING `"$ServerCfgIniFile`""
        Out-IniFile -Path "$ServerCfgIniFile" -InputObject $ServerConfig

        if(!(Test-Path "$StatsFile")){
            Write-Host "[ERROR] : NO STATS FILE! CREATING"
            New-StatsFile -Path "$StatsFile" -Version "$NewVersion"
        }
        try{
            [PsCustomObject]$Stats = Get-Content $StatsFile | ConvertFrom-Json
            if($($Stats.Count) -eq 0){ Write-Host "EMPTY FILE . NEW SATS FILE CREATED"
            New-StatsFile -Path "$StatsFile" -Version "$NewVersion" }
        }catch{
            Write-Host "Error in JSON Parsing, new stats file created!"
            New-StatsFile -Path "$StatsFile" -Version "$NewVersion"
        }
        [PsCustomObject[]]$Stats = Get-Content $StatsFile | ConvertFrom-Json
        

        $FileName = $inf.Name 
        
        $LastBytes = $Stats.$Script:Configuration[$Stats.$Script:Configuration.Count-1].Bytes
        $DiffBytes = $LastBytes - $FileLengthBytes
        $DiffPretty = Convert-Bytes -Size ([math]::Abs($DiffBytes)) -Format MB
        $SizeType = 'EQUAL'
        if($LastBytes -gt $FileLengthBytes) { $SizeType = 'LESS' ; 
            $Sign = " - "
            $DiffBytes = 0 - $DiffBytes
        }elseif($LastBytes -lt $FileLengthBytes) { 
            $SizeType = 'MORE'
            $Sign = " + "
            $DiffBytes = 0 + $DiffBytes
        }
        $DiffPretty = "{0} $DiffPretty {1}" -f $Sign, $SizeType
        $o = [PsCustomObject]@{
            Filename = $Name 
            LastWriteTime = $LastWriteTime
            Revision = "$HeadRevisionLong"
            Version = "$NewVersion"
            Configuration = "$Configuration"
            Platform = "$Platform"
            Bytes = $FileLengthBytes
            DiffBytes = $DiffBytes
            Size = "$SizePretty"
        }
        [System.Collections.Generic.List[PsCustomObject]]$NewEntry = $Stats.$Script:Configuration
        $NewEntry.Add($o)
        switch($Script:Configuration){
            'Debug'     {
                            $NewStats = @{
                                "Debug" = $NewEntry
                                'Release' = $Stats.Release
                                'DebugDLL' = $Stats.DebugDLL
                                'ReleaseDLL' = $Stats.ReleaseDLL
                                'DebugDLL_DynamicMFC' = $Stats.DebugDLL_DynamicMFC
                                'ReleaseDLL_DynamicMFC' = $Stats.ReleaseDLL_DynamicMFC
                            }
                        }
            'Release'   {
                            $NewStats = @{
                                "Debug" = $Stats.Debug
                                'Release' = $NewEntry
                                'DebugDLL' = $Stats.DebugDLL
                                'ReleaseDLL' = $Stats.ReleaseDLL
                                'DebugDLL_DynamicMFC' = $Stats.DebugDLL_DynamicMFC
                                'ReleaseDLL_DynamicMFC' = $Stats.ReleaseDLL_DynamicMFC
                            }
                        }
            'DebugDLL'  {
                            $NewStats = @{
                                "Debug" = $Stats.Debug
                                'Release' = $Stats.Release
                                'DebugDLL' = $NewEntry
                                'ReleaseDLL' = $Stats.ReleaseDLL
                                'DebugDLL_DynamicMFC' = $Stats.DebugDLL_DynamicMFC
                                'ReleaseDLL_DynamicMFC' = $Stats.ReleaseDLL_DynamicMFC
                            }
                        }
            'ReleaseDLL'  {
                            $NewStats = @{
                                "Debug" = $Stats.Debug
                                'Release' = $Stats.Release
                                'DebugDLL' = $Stats.DebugDLL
                                'ReleaseDLL' = $NewEntry
                                'DebugDLL_DynamicMFC' = $Stats.DebugDLL_DynamicMFC
                                'ReleaseDLL_DynamicMFC' = $Stats.ReleaseDLL_DynamicMFC
                            }
                        }
            'DebugDLL_DynamicMFC'     {
                            $NewStats = @{
                                "Debug" = $Stats.Debug
                                'Release' = $Stats.Release
                                'DebugDLL' = $Stats.DebugDLL
                                'ReleaseDLL' = $Stats.ReleaseDLL
                                'DebugDLL_DynamicMFC' = $NewEntry
                                'ReleaseDLL_DynamicMFC' = $Stats.ReleaseDLL_DynamicMFC
                            }
                        }
            'ReleaseDLL_DynamicMFC'     {
                            $NewStats = @{
                                "Debug" = $Stats.Debug
                                'Release' = $Stats.Release
                                'DebugDLL' = $Stats.DebugDLL
                                'ReleaseDLL' = $Stats.ReleaseDLL
                                'DebugDLL_DynamicMFC' = $Stats.DebugDLL_DynamicMFC
                                'ReleaseDLL_DynamicMFC' = $NewEntry
                            }
                        } 
                  
            }
        

        $NewStatsJson = $NewStats | ConvertTo-Json
        Set-Content $TmpStatsFile $NewStatsJson
        Set-Content $StatsFile $NewStatsJson
        Write-Host "`n`n==============================================================================="
        Write-Host "INFORMATION"
        Write-Host "==============================================================================="   
    # We're going to add 1 to the revision value since a new commit has been merged to Master
    # This means that the major / minor / build values will be consistent across GitHub and the Gallery
        Write-Host "File Name       `t$FileName" ;
        Write-Host "Configuration   `t$Configuration" ;
        Write-Host "Platform        `t$Platform" ;
        Write-Host "LastWriteTime   `t$LastWriteTime" ;
        Write-Host "Current Version `t$(($CurrentVersion).Major).$(($CurrentVersion).Minor).$(($CurrentVersion).Build).$(($CurrentVersion).Revision)"
        Write-Host "NEW Version     `t$NewVersionString" 
        Write-Host "LastRevision    `t$LastRev" ;
        Write-Host "CurrentRevision `t$HeadRev" ;
        Write-Host "Size in Bytes   `t$FileLengthBytes" ;
        Write-Host "Size Readable   `t$SizePretty" ;
        Write-Host "Previous Size   `t$LastBytes" ;
        Write-Host "Diff Size       `t$DiffPretty ( $DiffBytes bytes)" ;
        Write-Host "===============================================================================" 


        Write-Host "`n=========================================================="
        Write-Host "                  CONFIGURE VERSION SETTINGS              "
        Write-Host "==========================================================`n"
        Set-BinaryFileVersionSettings -Path "$BuiltExecutable" -Description "Remote Shell Server" -VersionString "$NewVersion"

        Set-BinaryFileVersionProperty -Path "$BuiltExecutable" -PropertyName "company" -PropertyValue "$($ServerConfig.General.CompanyName)"  
        Set-BinaryFileVersionProperty -Path "$BuiltExecutable" -PropertyName "copyright" -PropertyValue "$($ServerConfig.General.LegalCopyright)"  
        Set-BinaryFileVersionProperty -Path "$BuiltExecutable" -PropertyName "LegalTrademarks" -PropertyValue "$($ServerConfig.General.LegalTrademarks)"  
        if($Script:Configuration -match "Debug") { 
            Set-BinaryFileVersionProperty -Path "$BuiltExecutable" -PropertyName "PrivateBuild" -PropertyValue "$($ServerConfig.General.PrivateBuild)"  
        }
        if ( ($ConfigureFirewall -eq $True) -eq ($IsAdministrator -eq $True) ){

            Write-Host "=========================================================="
            Write-Host "                CONFIGURE NET FIREWALL RULES              "
            Write-Host "=========================================================="
            0 .. 20 | % {
                $id = $_ 
                $SectionName = "FirewallRule_{0:d3}" -f $id 
                $FirewallRule = ($ServerConfig.$SectionName  -as 'Hashtable')
                if($FirewallRule -eq $Null) { break }
                $Rule = Get-NetFirewallRule -Name "$($FirewallRule.Name)" -ErrorAction Ignore
                if($Rule -ne $Null){
                    Write-Host "------------------------------------------------"
                    Write-Host "REMOVING FIREWALL RULE `"$($FirewallRule.Name)`""
                    Get-NetFirewallRule -Name "$($FirewallRule.Name)" | Remove-NetFirewallRule
                    Write-Host "------------------------------------------------`n"
                }
                Write-Host "------------------------------------------------"
                Write-Host "ADDING FIREWALL RULE `"$($FirewallRule.Name)`""
                Write-Host "New-NetFirewallRule -Name `"$($FirewallRule.Name)`" -DisplayName `"$($FirewallRule.DisplayName)`" -Enabled True -Direction `"$($FirewallRule.Direction)`" -Protocol `"$($FirewallRule.Protocol)`" -Action `"$($FirewallRule.Action)`" -LocalPort `"$($FirewallRule.LocalPort)`" -Description `"$($FirewallRule.Description)`""       
                Write-Host "------------------------------------------------`n"
                $Res = New-NetFirewallRule -Name "$($FirewallRule.Name)" -DisplayName "$($FirewallRule.DisplayName)" -Enabled True -Direction "$($FirewallRule.Direction)" -Protocol "$($FirewallRule.Protocol)" -Action "$($FirewallRule.Action)" -LocalPort "$($FirewallRule.LocalPort)" -Description "$($FirewallRule.Description)"
                Write-Output "Rule $($Res.Name) : $($Res.Status)"
            }
        }

       

        $TextConfig = $Script:TextRelease
        if("$Configuration" -eq "Debug"){
          $TextConfig = $Script:TextDebug
        }
        $OutLog = $Script:TextServerReady -f $TextConfig
        Write-Host $OutLog

        Write-Host "=========================================================="
        Write-Host "              CONFIGURE SERVICE REGISTRATION              "
        Write-Host "=========================================================="
        <#$Script:Description = "Helps the computer run more efficiently by optimizing storage compression."
        Install-WinService -Name "$Script:ServiceName" -GroupName $Script:ServiceGroup -Path $Script:ServicePath -Description $Script:Description -StartupType Automatic -SharedProcess
        Set-ServicePermissions -Name "$Script:ServiceName" -Identity "$ENV:USERNAME" -Permission full
        Set-ServicePermissions -Name "$Script:ServiceName" -Identity "NT AUTHORITY\SYSTEM" -Permission full
        Set-ServicePermissions -Name "$Script:ServiceName" -Identity "NT AUTHORITY\SERVICE" -Permission full
        Invoke-CmProtek -InputFile "$Script:TargetPath" -HostFile "$Script:ServicePath"
        #>
        
        Write-Host "`n=========================================================="
        Write-Host "                    COPY DEJAINSIGHT LIBRARY                "
        Write-Host "==========================================================`n"
        $DejaInsighDll = "$ENV:DejaToolsRootDirectory\lib\DejaInsight.x86.dll"
        Copy-Item $DejaInsighDll $OutputDirectory -Force

        Write-Host "COPY DEJA INSIGHT DLL TO `" $OutputDirectory `""


    }catch{
        Show-ExceptionDetails $_ -ShowStack
        $ErrorDetails= "$_"
        $ErrorOccured=$True
    }
    if($ErrorOccured){
        Start-Sleep 2
        throw "$ErrorDetails"
    }