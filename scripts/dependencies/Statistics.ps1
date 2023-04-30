<#
#̷𝓍   𝓐𝓡𝓢 𝓢𝓒𝓡𝓘𝓟𝓣𝓤𝓜
#̷𝓍   🇵​​​​​🇴​​​​​🇼​​​​​🇪​​​​​🇷​​​​​🇸​​​​​🇭​​​​​🇪​​​​​🇱​​​​​🇱​​​​​ 🇸​​​​​🇨​​​​​🇷​​​​​🇮​​​​​🇵​​​​​🇹​​​​​ 🇧​​​​​🇾​​​​​ 🇬​​​​​🇺​​​​​🇮​​​​​🇱​​​​​🇱​​​​​🇦​​​​​🇺​​​​​🇲​​​​​🇪​​​​​🇵​​​​​🇱​​​​​🇦​​​​​🇳​​​​​🇹​​​​​🇪​​​​​.🇶​​​​​🇨​​​​​@🇬​​​​​🇲​​​​​🇦​​​​​🇮​​​​​🇱​​​​​.🇨​​​​​🇴​​​​​🇲​​​​​
#>


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

function Update-BinaryStatistics{
    [CmdletBinding(SupportsShouldProcess)]
    param(
        [parameter(mandatory=$true,Position=0)]
        [String]$Path,
        [parameter(mandatory=$true,Position=1)]
        [String]$StatsFile
    )
    try{
        $BuiltBinary = (Resolve-Path $Path -ErrorAction Ignore).Path
        if(!(Test-Path "$BuiltBinary")){
            throw "ERROR: NO BINARY FOUND @`"$BuiltBinary`""
        }
        $inf = Get-Item "$BuiltBinary"
        $FileLengthBytes = $inf.Length 
        $SizePretty = Convert-Bytes -Size $FileLengthBytes -Format MB

        if(!(Test-Path "$StatsFile")){
            Write-Output "[ERROR] : NO STATS FILE! CREATING"
            New-StatsFile -Path "$StatsFile" -Version "$NewVersion"
        }
        try{
            [PsCustomObject]$Stats = Get-Content $StatsFile | ConvertFrom-Json
            if($($Stats.Count) -eq 0){ Write-Output "EMPTY FILE . NEW SATS FILE CREATED"
            New-StatsFile -Path "$StatsFile" -Version "$NewVersion" }
        }catch{
            Write-Output "Error in JSON Parsing, new stats file created!"
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
        Write-BuildOutTitle "INFORMATION"
        Write-Output "`tFile Name       `t$FileName" ;
        Write-Output "`tConfiguration   `t$Configuration" ;
        Write-Output "`tPlatform        `t$Platform" ;
        Write-Output "`tLastWriteTime   `t$LastWriteTime" ;
        Write-Output "`tCurrent Version `t$(($CurrentVersion).Major).$(($CurrentVersion).Minor).$(($CurrentVersion).Build).$(($CurrentVersion).Revision)"
        Write-Output "`tNEW Version     `t$NewVersionString" 
        Write-Output "`tLastRevision    `t$LastRev" ;
        Write-Output "`tCurrentRevision `t$HeadRev" ;
        Write-Output "`tSize in Bytes   `t$FileLengthBytes" ;
        Write-Output "`tSize Readable   `t$SizePretty" ;
        Write-Output "`tPrevious Size   `t$LastBytes" ;
        Write-Output "`tDiff Size       `t$DiffPretty ( $DiffBytes bytes)" ;
        Write-Line

    }catch{
        throw $_
    }
}

