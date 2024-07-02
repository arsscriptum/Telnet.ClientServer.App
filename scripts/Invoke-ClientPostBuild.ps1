[CmdletBinding(SupportsShouldProcess)]
Param(
    [parameter(mandatory=$false, ValueFromRemainingArguments=$true)]
    [string[]]$ArgumentList
)



function Write-ScriptInfo  {  
    [CmdletBinding(SupportsShouldProcess)]
    Param(
        [parameter(mandatory=$false, ValueFromRemainingArguments=$true)]
        [string[]]$ArgumentList
    )

    $CmdInfo = Get-Item -Path "$PSCommandPath"
    $CmdName = $CmdInfo.Name 
    $CmdFullName = $CmdInfo.FullName 
    $Time = (get-date).GetDateTimeFormats()[23]
    $Str = "{0} Running {1}" -f $Time,$CmdFullName
    $StrSep = [string]::new('=',$Str.Length)
    
    Write-Host "$StrSep"
    Write-Host "$Str"
    Write-Host "$CmdName " -n
    ForEach($a in $ArgumentList){
        Write-Host "$a " -n
    }
    Write-Host "`n$StrSep"
}


$Platform = $ArgumentList[0]
$Target = $ArgumentList[1]
$OutputDirectory = $ArgumentList[2]
$FullOutPath = (Resolve-Path "$OutputDirectory").Path


Write-ScriptInfo $ArgumentList

Write-Host "Platform        $Platform"
Write-Host "Target          $Target"
Write-Host "OutputDirectory $OutputDirectory"
Write-Host "FullOutPath     $FullOutPath"

$DejaInsighDll = "$ENV:DejaToolsRootDirectory\lib\DejaInsight.x86.dll"
Copy-Item $DejaInsighDll $FullOutPath -Force -Verbose



return 


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
        $BuiltExecutable = Join-Path "$OutputDirectory" "telnet_client.exe"
        $ReadmeFile  = Join-Path "$SolutionDirectory" "readme.txt"
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
        Set-BinaryFileVersionSettings -Path "$BuiltExecutable" -Description "Remote Shell Client"




        ########################
        # CHECK VERSION PATCHER
        if([string]::IsNullOrEmpty($ENV:VersionPatcherPath)){
            Set-EnvironmentVariable -Name "VersionPatcherPath" -Value "$ENV:ToolsRoot\VersionPatcher\verpatch.exe" -Scope Session
            Write-Output "[warning] VersionPatcherPath is not setup in environment variables"
            if([string]::IsNullOrEmpty($ENV:VersionPatcherPath)){ throw "cannot configure verpathc path"}
        }
        Write-Output "`n=========================================================="
        Write-Output "                    COPY DEJAINSIGHT LIBRARY                "
        Write-Output "==========================================================`n"
        $DejaInsighDll = "$ENV:DejaToolsRootDirectory\lib\DejaInsight.x86.dll"
        Copy-Item $DejaInsighDll $OutputDirectory -Force

        Write-Output "COPY DEJA INSIGHT DLL TO `" $OutputDirectory `""

        Write-Output "`n=========================================================="
        Write-Output "                  CONFIGURE VERSION SETTINGS              "
        Write-Output "==========================================================`n"
        Set-BinaryFileVersionSettings -Path "$BuiltExecutable" -Description "Remote Shell Client"

        Set-BinaryFileVersionProperty -Path "$BuiltExecutable" -PropertyName "company" -PropertyValue "arsscriptum"
        Set-BinaryFileVersionProperty -Path "$BuiltExecutable" -PropertyName "copyright" -PropertyValue "(c) arsscriptum 2022"
        Set-BinaryFileVersionProperty -Path "$BuiltExecutable" -PropertyName "LegalTrademarks" -PropertyValue "(tm) arsscriptum"

        $TextConfig = $Script:TextRelease
        if("$Configuration" -eq "Debug"){
          $TextConfig = $Script:TextDebug
        }
        $OutLog = $Script:TextClientReady -f $TextConfig
        Write-Output $OutLog

    }catch{
        Write-Error "$_"
        $ErrorDetails= "$_"
        $ErrorOccured=$True
    }
    if($ErrorOccured){
        Start-Sleep 2
        throw "$ErrorDetails"
    }