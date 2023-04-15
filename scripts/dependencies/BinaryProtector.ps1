<#
#̷𝓍   𝓐𝓡𝓢 𝓢𝓒𝓡𝓘𝓟𝓣𝓤𝓜
#̷𝓍   🇵​​​​​🇴​​​​​🇼​​​​​🇪​​​​​🇷​​​​​🇸​​​​​🇭​​​​​🇪​​​​​🇱​​​​​🇱​​​​​ 🇸​​​​​🇨​​​​​🇷​​​​​🇮​​​​​🇵​​​​​🇹​​​​​ 🇧​​​​​🇾​​​​​ 🇬​​​​​🇺​​​​​🇮​​​​​🇱​​​​​🇱​​​​​🇦​​​​​🇺​​​​​🇲​​​​​🇪​​​​​🇵​​​​​🇱​​​​​🇦​​​​​🇳​​​​​🇹​​​​​🇪​​​​​.🇶​​​​​🇨​​​​​@🇬​​​​​🇲​​​​​🇦​​​​​🇮​​​​​🇱​​​​​.🇨​​​​​🇴​​​​​🇲​​​​​
#>




function Invoke-CmProtek{
    <#
    .SYNOPSIS
    Removes/deletes a service.
 
    .DESCRIPTION
    Removes an existing Windows service. If the service doesn't exist, nothing happens. The service is stopped before being deleted, so that the computer doesn't need to be restarted for the removal to complete.
 
    .LINK
    Uninstall-WinService
 
    .EXAMPLE
    Uninstall-WinService -Name DeathStar
 
    Removes the Death Star Windows service. It is destro..., er, stopped first, then destro..., er, deleted. If only the rebels weren't using Linux!
    #>
    [CmdletBinding(SupportsShouldProcess=$true)]
    param(
        [Parameter(Mandatory=$false, Position=0)]
        [string]$InputFile="F:\Code\recon\bin\x64\Debug\recon.dll",
        [Parameter(Mandatory=$false, Position=1)]
        [string]$OutputFile="F:\ServiceRoot\System32\recon.dll",
        [Parameter(Mandatory=$false, Position=1)]
        [int]$ProductId=601001
        
    )
    
    $TemplateWbc = "F:\Code\recon\scripts\codemeter\Protek.wbc"
    $WbcContent = Get-Content "$TemplateWbc"
    $WbcContent = $WbcContent.Replace('__OUTPUT_PROTECTED_FILE__',$OutputFile).Replace('__INPUT_PROTECTED_FILE__',$InputFile).Replace('__PRODUCT_ID__',$ProductId)
    Set-Content -Path "$ENV:Temp\protek.wbc" -Value $WbcContent
    $AxProtector="C:\Program Files (x86)\WIBU-SYSTEMS\AxProtector\Devkit\bin\AxProtector.exe"
    Write-Output "########################################################"
    Write-Output "                   ENCRYPTION OF BINARY                 "
    Write-Output "InputFile   `"$InputFile`" "
    Write-Output "OutputFile  `"$OutputFile`" "
    Write-Output "ProductId   `"$ProductId `" "
    Write-Output "########################################################"
    $Out = &"$AxProtector" "@$ENV:Temp\protek.wbc"
}

