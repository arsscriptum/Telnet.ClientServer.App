[CmdletBinding(SupportsShouldProcess)]
param()

try{
    $ScriptsDirectory = "$PSScriptRoot"
    [string[]]$deps = . "$ScriptsDirectory\test\GetDependencies.ps1" -Path "$ScriptsDirectory\test"
    $depscount = $deps.Count
    $deps | % {
        $file = $_
        . "$file"     
    }
    $Test = Test-Dependencies -Quiet
    if(! ($Test) ) { throw "dependencies error"} 

    Update-TelnetKeyCodesSourceFile
        
}catch{
    Write-Error "$_"
}
   