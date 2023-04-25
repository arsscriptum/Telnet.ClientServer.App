
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
    $all = [PsCustomObject]@{
        'Debug' = $a
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
        $ConfigureFirewall=$False
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
        $Script:OutFile = "telnet_srv.exe"
        if($Script:Configuration -match "DLL") { $Script:OutFile = "telnet_srv.dll" }
        
        $SolutionDirectory = (Resolve-Path $RootPath).Path
        
        $ScriptsDirectory = (Resolve-Path "$SolutionDirectory\scripts").Path
        $OutputDirectory = (Resolve-Path "$SolutionDirectory\bin\$Script:Platform\$Script:Configuration").Path
        $ProjectsPath = Join-Path $SolutionDirectory 'vs'
        $BuiltExecutable = Join-Path "$OutputDirectory" "$Script:OutFile"
        $ReadmeFile  = Join-Path "$SolutionDirectory" "usage.txt"
        $VersionFile  = Join-Path "$ProjectsPath" "version_server.nfo"
        $StatsFile  = Join-Path "$ProjectsPath" "stats_server.json"
        $TmpStatsFile  = Join-Path "$ENV:TEMP" "stats_server.json"

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
        
        Write-Host "`n=========================================================="
        Write-Host "                    COPY DEJAINSIGHT LIBRARY                "
        Write-Host "==========================================================`n"
        $DejaInsighDll = "$ENV:DejaToolsRootDirectory\lib\DejaInsight.x86.dll"
        Copy-Item $DejaInsighDll $OutputDirectory -Force

        Write-Host "COPY DEJA INSIGHT DLL TO `" $OutputDirectory `""

        Write-Host "`n=========================================================="
        Write-Host "                  CONFIGURE VERSION SETTINGS              "
        Write-Host "==========================================================`n"
        Set-BinaryFileVersionSettings -Path "$BuiltExecutable" -Description "Remote Shell Server"

        Set-BinaryFileVersionProperty -Path "$BuiltExecutable" -PropertyName "company" -PropertyValue "arsscriptum"
        Set-BinaryFileVersionProperty -Path "$BuiltExecutable" -PropertyName "copyright" -PropertyValue "(c) arsscriptum 2022"
        Set-BinaryFileVersionProperty -Path "$BuiltExecutable" -PropertyName "LegalTrademarks" -PropertyValue "(tm) arsscriptum"
        if($ConfigureFirewall){
            Write-Host "=========================================================="
            Write-Host "                CONFIGURE NET FIREWALL RULES              "
             Write-Host "=========================================================="
            if($IsAdministrator -eq $True){
                $FirewallRule = Get-NetFirewallRule -Name 'remote_shell_server' -ErrorAction Ignore
                if($FirewallRule -eq $Null){
                    Write-Host "Add New Rule"
                    New-NetFirewallRule -Name "remote_shell_server" -DisplayName 'remote_shell_server' -Enabled True -Direction Inbound -Protocol TCP -Action Allow -LocalPort 34010-34050
                }
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
        


        Write-Host "`n`n==============================================================================="
        Write-Host "GETTING MODULE VERSION"
        Write-Host "==============================================================================="   
        $HeadRev                        = git log --format=%h -1 | select -Last 1
        $LastRev                        = git log --format=%h -2 | select -Last 1

        if(-not(Test-Path $VersionFile)){
            throw "Missing Version File $VersionFile"
        }

        [string]$VersionString = '99.99.98'
        if(Test-Path $VersionFile){
            [string]$VersionString = (Get-Content -Path $VersionFile -Raw)
        }else{
            throw "Missing Version File $VersionFile"
        }

        [Version]$CurrentVersion = $VersionString
        [Version]$NewVersion = $VersionString
        $NewVersionRevision = $NewVersion.Revision
        $NewVersionRevision++
        $NewVersion = New-Object -TypeName System.Version -ArgumentList $NewVersion.Major,$NewVersion.Minor,$NewVersion.Build,$NewVersionRevision
        [string]$NewVersionString = $NewVersion.ToString()
        
        
        Set-Content -Path $VersionFile -Value $NewVersionString
        $UpdatedVersion = $NewVersionString


        $inf = Get-Item "$BuiltExecutable"
        $FileLengthBytes = $inf.Length 
        $SizePretty = Convert-Bytes -Size $FileLengthBytes -Format MB


        if(!(Test-Path "$StatsFile")){
            Write-Host "[ERROR] : NO STATS FILE! CREATING"
            New-StatsFile -Path "$StatsFile" -Version "$NewVersion"
        }
        try{
            [PsCustomObject[]]$Stats = Get-Content $StatsFile | ConvertFrom-Json
            if($($Stats.Count) -eq 0){ Write-Host "EMPTY FILE . NEW SATS FILE CREATED"
            New-StatsFile -Path "$StatsFile" -Version "$NewVersion" }
        }catch{
            Write-Host "Error in JSON Parsing, new stats file created!"
            New-StatsFile -Path "$StatsFile" -Version "$NewVersion"
        }
        [PsCustomObject[]]$Stats = Get-Content $StatsFile | ConvertFrom-Json
        

        $FileName = $inf.Name 
        [DateTime]$LastWriteDate = $inf.LastWriteTime
        [String]$LastWriteTime = $LastWriteDate.GetDateTimeFormats()[26]
        $LastBytes = $Stats[$Stats.Count-1].Bytes
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
            Revision = "$HeadRev"
            Version = "$NewVersion"
            Configuration = "$Configuration"
            Platform = "$Platform"
            Bytes = $FileLengthBytes
            DiffBytes = $DiffBytes
            Size = "$SizePretty"
        }

        $subobj = $Script:Configuration
        ($Stats.$subobj -as [System.Collections.Generic.List[PsCustomObject]]).Add($o)
    
        $StatsJson = $Stats | ConvertTo-Json
        Set-Content $TmpStatsFile $StatsJson
      
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



    }catch{
        Show-ExceptionDetails $_ -ShowStack
        $ErrorDetails= "$_"
        $ErrorOccured=$True
    }
    if($ErrorOccured){
        Start-Sleep 2
        throw "$ErrorDetails"
    }