[CmdletBinding(SupportsShouldProcess)]
param()

try{
    $ScriptsDirectory = "$PSScriptRoot"
    [string[]]$deps = . "$ScriptsDirectory\dependencies\GetDependencies.ps1" -Path "$ScriptsDirectory\dependencies"
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
   