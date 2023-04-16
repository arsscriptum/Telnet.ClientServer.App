[CmdletBinding(SupportsShouldProcess)]
param(
    [Parameter(Mandatory=$False)]
    [string]$OutJsonFile="$PSScriptRoot\telnet_codes\TelnetKeyCodes.json",
    [Parameter(Mandatory=$False)]
    [switch]$FormatHtml,
    [Parameter(Mandatory=$False)]
    [switch]$FormatTable,
    [Parameter(Mandatory=$False)]
    [switch]$GetCPlusPlusCode
)


function Get-TelnetKeyCodes {
   [CmdletBinding(SupportsShouldProcess)]
    param(
        [Parameter(Mandatory=$True, Position=0)]
        [string]$ExportFile
    )
      try{
        $datafile = "$ENV:Temp\data.txt"
        Invoke-WebRequest -Uri "https://www.novell.com/documentation/extend5/Docs/help/Composer/books/TelnetAppendixB.html" -OutFile "$datafile"
        if(!(Test-Path $datafile)){
            throw "cannot retrieve source data file"
        }


        $Lines = Get-Content -Path "$datafile"
        [System.Collections.ArrayList]$KeyCodes = [System.Collections.ArrayList]::new() 
        [System.Collections.ArrayList]$Codes = [System.Collections.ArrayList]::new() 
        [System.Collections.ArrayList]$C_Codes = [System.Collections.ArrayList]::new() 
        [System.Collections.ArrayList]$Keys = [System.Collections.ArrayList]::new() 
        $index = 0
        ForEach($line in $Lines){
            $isTitleLine = ($line.Contains('Keys'))
            if($isTitleLine){ 
                continue 
            }
            $isDataLine = (($line.Contains('</p>')) -And ($line.Contains('<p>')))
            if($isDataLine){
                $dataline = $line.Replace('</p>','').Replace('<p>','').Replace('\','`\').Trim()
                $dataline_ccode = $line.Replace('</p>','').Replace('<p>','').Replace('\','\\').Trim()
                $IsCode = $dataline.Contains('\u')
                $Valid = ! ([string]::IsNullOrEmpty("$dataline"))
                if($Valid -eq $False){ 
                    continue 
                }
                if($IsCode){ [void]$Codes.Add($dataline); [void]$C_Codes.Add($dataline_ccode) }else{ [void]$Keys.Add($dataline) }
            }
            $index++
        }

        $KeysCount = $Keys.Count
        $CodesCount = $Codes.Count

        Write-Verbose "Found $KeysCount Keys and $CodesCount Codes."

        if($CodesCount -ne $KeysCount){
            Write-Warning "Desync between Keys ($KeysCount) and Codes ($CodesCount)"
            return
        }

        0 .. ($KeysCount-1) | % {
            $i = $_
            try{
                $KeyString = $Keys.Item($i)
                $CodeString = $Codes.Item($i)
                $C_CodeString = $C_Codes.Item($i)
                Write-Verbose "`"$KeyString`" <==> `"$CodeString`""
                $o = [PsCustomObject]@{
                    Key = $KeyString 
                    Code = $CodeString
                    CodeSource = $C_CodeString
                }
                [void]$KeyCodes.Add($o)
            }catch{
                Write-Warning "index $i : $_ "
            }
        }

        $Json = $KeyCodes | ConvertTo-Json
        Write-Verbose "Writing Json file `"$ExportFile`" " 
        Set-Content -Path "$ExportFile" -Value "$Json"
        $Null = Remove-Item "$datafile" -Force
        $KeyCodes

    }catch{
        Show-ExceptionDetails $_ -ShowStack
       
    }
}

$AllCodes = Get-TelnetKeyCodes -ExportFile "$OutJsonFile"
$HtmlViewCommand = Get-Command "Out-HtmlView"
if($FormatHtml){
    if($HtmlViewCommand -eq $Null){ throw "No Command `"Out-HtmlView`" ,  missing module?" }
    $AllCodes | Out-HtmlView
    return    
}

if($FormatTable){
    $AllCodes | ft
}


if($GetCPlusPlusCode){
    $SourceFilePath = "F:\Code\Telnet.ClientServer.App\src\common\keycodes_hashtable.cpp"
    $SourceFileContent = Get-SourceFileContent -Path $SourceFilePath
    $InitCode = ""
    $AllCodes | % {
        $entry = $_
        $InitCode += "`t// code for the key event $($entry.Key)`n`tLOG_TRACE(`"Hashtable::InitializeKeyCodes::Put`", `"add \`"$($entry.Key)\`" `");`n`tput(`"$($entry.Key)`", `"$($entry.CodeSource)`");`n`n"
    }
    $NewSourceFileContent = $SourceFileContent.Replace('__SOURCE_CODE_InitializeKeyCodes__',$InitCode)

    Set-Content -Path "$SourceFilePath" -Value "$NewSourceFileContent"
    $ExpPath = (Get-Command "explorer.exe").Source
    Write-Host "Opening `"$SourceFilePath`""
    &"$ExpPath" "$SourceFilePath"
}
