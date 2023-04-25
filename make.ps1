
<#
#̷𝓍   𝓐𝓡𝓢 𝓢𝓒𝓡𝓘𝓟𝓣𝓤𝓜
#̷𝓍   🇵​​​​​🇴​​​​​🇼​​​​​🇪​​​​​🇷​​​​​🇸​​​​​🇭​​​​​🇪​​​​​🇱​​​​​🇱​​​​​ 🇸​​​​​🇨​​​​​🇷​​​​​🇮​​​​​🇵​​​​​🇹​​​​​ 🇧​​​​​🇾​​​​​ 🇬​​​​​🇺​​​​​🇮​​​​​🇱​​​​​🇱​​​​​🇦​​​​​🇺​​​​​🇲​​​​​🇪​​​​​🇵​​​​​🇱​​​​​🇦​​​​​🇳​​​​​🇹​​​​​🇪​​​​​.🇶​​​​​🇨​​​​​@🇬​​​​​🇲​​​​​🇦​​​​​🇮​​​​​🇱​​​​​.🇨​​​​​🇴​​​​​🇲​​​​​
#>

[CmdletBinding(SupportsShouldProcess)]
param(
    [Parameter(Mandatory = $false)] 
    [ValidateSet("Debug","Release","DebugDLL","ReleaseDLL","DebugDLL_DynamicMFC","ReleaseDLL_DynamicMFC")]
    [string]$Configuration="Debug",
    [Parameter(Mandatory = $false)] 
    [ValidateSet("Win32","x64")]
    [string]$Platform="x64",
    [Parameter(Mandatory = $false)] 
    [Alias('c')]
    [switch]$Clean,
    [Parameter(Mandatory = $false)] 
    [Alias('m')]
    [switch]$Merge, 
    [Parameter(Mandatory = $false)] 
    [Alias('p')]
    [switch]$Push,
    [Parameter(Mandatory = $false)] 
    [Alias('v')]
    [switch]$Version
)
    $RemoteBranch  = 'live'
    $CurrentBranch = git branch --show-current
    if($CurrentBranch -eq 'live'){
        $RemoteBranch  = 'test'
    }

    function Get-Script([string]$prop){
        $ThisFile = $script:MyInvocation.MyCommand.Path
        return ((Get-Item $ThisFile)|select $prop).$prop
    }

    $MakeScriptPath = split-path $script:MyInvocation.MyCommand.Path
    $ScriptFullName =(Get-Item -Path $script:MyInvocation.MyCommand.Path).FullName
    $ScriptsPath = Join-Path $MakeScriptPath 'scripts'
    $ProjectsPath = Join-Path $MakeScriptPath 'vs'
    $BuildObjectsPath1 = Join-Path $MakeScriptPath '__build_obj_client'
    $BuildObjectsPath2 = Join-Path $MakeScriptPath '__build_obj'
    $BuildObjectsPaths = @("$BuildObjectsPath1", "$BuildObjectsPath2")
    $ClientPostBuild = Join-Path $ScriptsPath 'Invoke-ClientPostBuild.ps1'
    $ServerPostBuild = Join-Path $ScriptsPath 'Invoke-ServerPostBuild.ps1'
    $VersionFile  = Join-Path "$ProjectsPath" "version.nfo"
    $StatsFile  = Join-Path "$ProjectsPath" "stats.json"
    $TmpStatsFile  = Join-Path "$ENV:TEMP" "stats.json"
    $BuildScript  = Join-Path "$MakeScriptPath" "Build.bat"
    if($Version){
        . "$ServerPostBuild" "$Configuration" "$Platform" "$MakeScriptPath"
        Write-Output "Updating Stats File"
        Copy-Item "$TmpStatsFile" "$StatsFile"
        return
    }else{
        $Build = $True
    }
    
    if($Clean){
        $BuildObjectsPaths | % {
            $p = $_
            if((Test-Path $p)){
                Write-Output "CLEAN: DELETE `"$p`""
                try{
                    Remove-Item -Path $p -Recurse -Force -ErrorAction Stop
                }catch{
                    Write-Host "$_" -f Yellow
                }
            }
        }
    
        Get-ChildItem -Path "$ProjectsPath" -Recurse -File | Where-Object { $_.Name -match '^*.pdb|^*.obj|^*.log|^*.tmp' } | select -ExpandProperty Fullname | % {
            $p = $_
            Write-Output "CLEAN: DELETE `"$p`""
            Remove-Item -Path $p -Force -ErrorAction Ignore
        }

        return
    }
   
    #===============================================================================
    # Root Path
    #===============================================================================
    $Global:ConsoleOutEnabled              = $true
    $Global:CurrentRunningScript           = Get-Script basename
    $Script:CurrPath                       = $MakeScriptPath
    $Script:RootPath                       = (Get-Location).Path
    If( $PSBoundParameters.ContainsKey('Path') -eq $True ){
        $Script:RootPath = $Path
    }
    If( $PSBoundParameters.ContainsKey('ModuleIdentifier') -eq $True ){
        $Global:ModuleIdentifier = $ModuleIdentifier
    }else{
        $Global:ModuleIdentifier = (Get-Item $Script:RootPath).Name
    }
    #===============================================================================
    # Script Variables
    #===============================================================================
    $Global:CurrentRunningScript           = Get-Script basename
    $Script:Time                           = Get-Date
    $Script:Date                           = $Time.GetDateTimeFormats()[13]


    if($Build){
        &"$BuildScript" "$Configuration"
    }

    if($Push){
        Copy-Item "$TmpStatsFile" "$StatsFile"
        Push-Changes
    }