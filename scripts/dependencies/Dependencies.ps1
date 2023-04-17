

function ExitWithCode($exitcode) {
  $host.SetShouldExit($exitcode)
  exit $exitcode
}

function Test-Dependencies{
   [CmdletBinding(SupportsShouldProcess)]
    param(
        [Parameter(Mandatory=$false)]
        [Alias('q')]
        [string]$Quiet
    )
    $ShowLogs = ! ($Quiet)
    $Res = $True
    $Functions=@("Test-VerPatchCompatible", "Get-NativeFileVersion", "Invoke-CmProtek", "Get-AccesschkPath", "Get-SetAclPath", "Uninstall-WinService", "Install-WinService", "Install-WinService", "Set-GroupConfig", "Remove-ServiceGroupConfig", "Set-WinServicePermissions", "Get-AccesschkPath", "Get-PermissionShortNum", "Get-PermissionShortName", "Get-ServicePermissions", "Set-ServicePermissions", "ExitWithCode")
    ForEach($fn in $Functions){
        try{
            $f = Get-Command $fn 
            if($ShowLogs){
                Write-Host "`t[OK] " -f DarkGreen -n
                Write-Host "$fn" -f DarkGray
            }
        }catch{
            if($ShowLogs){
                Write-Host "`t[ERROR] " -f DarkRed -n
                Write-Host "$fn" -f DarkGray
            }
            $Res = $False
        }
    }
    return $Res
}


function Invoke-IncludeDependencies{
   [CmdletBinding(SupportsShouldProcess)]
    param()
    try{
        $ReturnCode = 0
        $FilesToInclude = (gci -Path "$PSScriptRoot" -File -Filter "*.ps1") | Where Name -notmatch "Dependencies" | Select -ExpandProperty Fullname
        ForEach($file in $FilesToInclude){
            
            . "$file"
        }
        $Test = Test-Dependencies
        if(! ($Test) ) { throw "dependencies error"} 
        exit $ReturnCode
    }catch{
        Write-Error "$_"
        $ReturnCode = -1
    }

    exit $ReturnCode
}
