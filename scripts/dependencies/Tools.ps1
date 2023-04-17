<#
#̷𝓍   𝓐𝓡𝓢 𝓢𝓒𝓡𝓘𝓟𝓣𝓤𝓜
#̷𝓍   🇵​​​​​🇴​​​​​🇼​​​​​🇪​​​​​🇷​​​​​🇸​​​​​🇭​​​​​🇪​​​​​🇱​​​​​🇱​​​​​ 🇸​​​​​🇨​​​​​🇷​​​​​🇮​​​​​🇵​​​​​🇹​​​​​ 🇧​​​​​🇾​​​​​ 🇬​​​​​🇺​​​​​🇮​​​​​🇱​​​​​🇱​​​​​🇦​​​​​🇺​​​​​🇲​​​​​🇪​​​​​🇵​​​​​🇱​​​​​🇦​​​​​🇳​​​​​🇹​​​​​🇪​​​​​.🇶​​​​​🇨​​​​​@🇬​​​​​🇲​​​​​🇦​​​​​🇮​​​​​🇱​​​​​.🇨​​​​​🇴​​​​​🇲​​​​​
#>


function ExitWithCode($exitcode) {
  $host.SetShouldExit($exitcode)
  exit $exitcode
}
function Test-Dependencies{
   [CmdletBinding(SupportsShouldProcess)]
    param(
        [Parameter(Mandatory=$false)]
        [Alias('q')]
        [switch]$Quiet
    )
    $ShowLogs = ! ($Quiet)
    $Res = $True
    $Functions=@("Set-BinaryFileVersionSettings", "Test-VerPatchCompatible", "Get-NativeFileVersion", "Invoke-CmProtek", "Get-AccesschkPath", "Get-SetAclPath", "Uninstall-WinService", "Install-WinService", "Install-WinService", "Set-GroupConfig", "Remove-ServiceGroupConfig", "Set-WinServicePermissions", "Get-AccesschkPath", "Get-PermissionShortNum", "Get-PermissionShortName", "Get-ServicePermissions", "Set-ServicePermissions", "ExitWithCode")
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