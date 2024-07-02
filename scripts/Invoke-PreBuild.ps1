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


function Stop-RunningProcess  {  
    [CmdletBinding(SupportsShouldProcess)]
    Param(
        [parameter(mandatory=$true, Position=0)]
        [string]$Name
    )

    $TaskkillExe=(get-command 'taskkill.exe').Source
    $TargetExe = "$Name" + '.exe'
    $ExecRunning = ( (get-process "$Name" -ErrorAction Ignore) -ne $Null)
    if($ExecRunning){
        Write-Host "Killing running $TargetExe instance"
        $Out = &"$TaskkillExe" '/IM' "$TargetExe" '/f'
    }
}


Write-ScriptInfo $ArgumentList

$ArgumentCount = $ArgumentList.Count 
if($ArgumentCount -gt 0){
    ForEach($a in $ArgumentList){
        Stop-RunningProcess "$a"
    }
}else{
    Write-Host "No process name specified!"
}



