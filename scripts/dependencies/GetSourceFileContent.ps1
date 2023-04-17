
function Get-SourceFileContent {
   [CmdletBinding(SupportsShouldProcess)]
    param(
        [Parameter(Mandatory=$True, Position=0)]
        [string]$Path
    )
      try{
        [System.Collections.ArrayList]$TrimmedContent = [System.Collections.ArrayList]::new()
        $SourceFileContent = Get-Content $Path
        $RecordingLines = $true
        ForEach($line in $SourceFileContent){
            $FunctionStart = $line.Contains('void KeyCodesHashTable::InitializeKeyCodes(){')
            $FunctionEnd = $line.Contains('// InitializeKeyCodes END')

            if($FunctionStart) { 
                $RecordingLines = $false 
                [void]$TrimmedContent.Add("$line`n")
                [void]$TrimmedContent.Add("__SOURCE_CODE_InitializeKeyCodes__`n")
                [void]$TrimmedContent.Add("} // InitializeKeyCodes END`n")
            }
            if($FunctionEnd) { 
                $RecordingLines = $true 
                continue
            }
            if($RecordingLines) {
                [void]$TrimmedContent.Add($line)
            }
        }

        Set-Content -Path "$ENV:Temp\KeyCodesHashTable.Tmp" -Value $TrimmedContent
        $ReturnedContent = Get-Content "$ENV:Temp\KeyCodesHashTable.Tmp" -Raw
        $ReturnedContent
    }catch{
        Show-ExceptionDetails $_ -ShowStack
       
    }
}

