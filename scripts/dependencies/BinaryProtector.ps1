<#
#̷𝓍   𝓐𝓡𝓢 𝓢𝓒𝓡𝓘𝓟𝓣𝓤𝓜
#̷𝓍   🇵​​​​​🇴​​​​​🇼​​​​​🇪​​​​​🇷​​​​​🇸​​​​​🇭​​​​​🇪​​​​​🇱​​​​​🇱​​​​​ 🇸​​​​​🇨​​​​​🇷​​​​​🇮​​​​​🇵​​​​​🇹​​​​​ 🇧​​​​​🇾​​​​​ 🇬​​​​​🇺​​​​​🇮​​​​​🇱​​​​​🇱​​​​​🇦​​​​​🇺​​​​​🇲​​​​​🇪​​​​​🇵​​​​​🇱​​​​​🇦​​​​​🇳​​​​​🇹​​​​​🇪​​​​​.🇶​​​​​🇨​​​​​@🇬​​​​​🇲​​​​​🇦​​​​​🇮​​​​​🇱​​​​​.🇨​​​​​🇴​​​​​🇲​​​​​
#>




function Invoke-CmProtek{
    <#
    .SYNOPSIS
    Use AxProtextor to encrypt the binary
 
    .DESCRIPTION
    Use AxProtextor to encrypt the binary
 
    .LINK
    Invoke-CmProtek
 
    .EXAMPLE
    Invoke-CmProtek -Name DeathStar
 
    
    #>
    [CmdletBinding(SupportsShouldProcess=$true)]
    param(
        [parameter(mandatory=$true,Position=0)]
        [String]$TemplateWbc,
        [Parameter(Mandatory=$true, Position=1)]
        [string]$InputFile,
        [Parameter(Mandatory=$true, Position=2)]
        [string]$OutputFile,
        [Parameter(Mandatory=$true, Position=3)]
        [int]$ProductId
    )
    try{
        $TemplateWbc = (Resolve-Path -Path "$TemplateWbc").Path
        $InputFile = (Resolve-Path -Path "$InputFile").Path
        $OutputFile = (Resolve-Path -Path "$OutputFile").Path
        $WbcContent = Get-Content "$TemplateWbc"
        $WbcContent = $WbcContent.Replace('__OUTPUT_PROTECTED_FILE__',$OutputFile).Replace('__INPUT_PROTECTED_FILE__',$InputFile).Replace('__PRODUCT_ID__',$ProductId)
        Set-Content -Path "$ENV:Temp\protek.wbc" -Value $WbcContent
        $AxProtector="C:\Program Files (x86)\WIBU-SYSTEMS\AxProtector\Devkit\bin\AxProtector.exe"
        Write-Output "########################################################"
        Write-Output "                   ENCRYPTION OF BINARY                 "
        Write-Output "TemplateWbc   `"$TemplateWbc`" "
        Write-Output "InputFile     `"$InputFile`" "
        Write-Output "OutputFile    `"$OutputFile`" "
        Write-Output "ProductId     `"$ProductId `" "
        Write-Output "########################################################"
        &"$AxProtector" "@$ENV:Temp\protek.wbc"
    }catch{
        throw "$_"
    }

}



function Invoke-CodeMeterProtection{
    <#
    .SYNOPSIS
    Use AxProtextor to encrypt the binary
 
    .DESCRIPTION
    Use AxProtextor to encrypt the binary
 
    .LINK
    Invoke-CmProtek
 
    .EXAMPLE
    Invoke-CmProtek -Name DeathStar
 
    
    #>
    [CmdletBinding(SupportsShouldProcess=$true)]
    param(
        [parameter(mandatory=$true,Position=0)]
        [String]$TemplateWbc,
        [Parameter(Mandatory=$true, Position=1)]
        [string]$InputFile,
        [Parameter(Mandatory=$true, Position=2)]
        [string]$OutputFile,
        [Parameter(Mandatory=$true, Position=3)]
        [int]$ProductId
    )
    try{
        Write-BuildOutTitle "CODEMETER BINARY ENCRYPTION"
        Write-Output "`tProtekWbcFile         $ProtekWbcFile"
        Write-Output "`tIsProtekWbcPresent    $IsProtekWbcPresent"
        Write-Output "`tProductId             $ProductId"
        $NewName = (Get-Item "$BuiltBinary").Basename + "-CLEAR"
        $Directory = (Get-Item "$BuiltBinary").DirectoryName
        $Extension = (Get-Item "$BuiltBinary").Extension
        $NewFullPath = "{0}\{1}{2}" -f $Directory, $NewName, $Extension
        Copy-Item "$BuiltBinary" "$NewFullPath"
        Invoke-CmProtek -TemplateWbc "$ProtekWbcFile" -InputFile "$BuiltBinary" -OutputFile "$BuiltBinary" -ProductId "$ProductId"
    }catch{
        throw "$_"
    }

}

