
    [CmdletBinding(SupportsShouldProcess)]
    param()

    $Code = @"

"@

    try{
        $ReturnCode = 0

        $dep1 = "$PSScriptRoot\ServiceInstaller.ps1"
        $dep2 = "$PSScriptRoot\ServicePermissions.ps1"
        $dep3 = "$PSScriptRoot\BinaryProtector.ps1"
        $dep4 = "$PSScriptRoot\Tools.ps1"
        . "$dep1"
        . "$dep2"
        . "$dep3"
        . "$dep4"
        
     
#>
        exit $ReturnCode
    }catch{
        Write-Error "$_"
        $ReturnCode = -1
    }

    exit $ReturnCode