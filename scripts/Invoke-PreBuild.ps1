
function Get-ScriptDirectory {
    Split-Path -Parent $PSCommandPath
}

      try{

        $ScriptDirectory = Get-ScriptDirectory
        $SolutionRoot = (Resolve-Path "$ScriptDirectory\..").Path
        $plexe = (Resolve-Path "$SolutionRoot\tools\pl\pl.exe").Path
        [string[]]$Out = &"$plexe" 'telnet_srv'
        $ServerPresent = !($Out[0].Contains('Error'))
        [string[]]$Out = &"$plexe" 'telnet_client'
        $ClientPresent = !($Out[0].Contains('Error'))
        $tk=(get-command taskkill).Source

        try{
            
            if($ClientPresent){
                Write-Output "Killing running client instance..."
                $Out = &"$tk" '/IM' 'telnet_client.exe' '/f'
            }
            if($ServerPresent){
                Write-Output "Killing running server instance..."
                $Out = &"$tk" '/IM' 'telnet_srv.exe' '/f'
            }
            
        }catch{}
    }catch{
        Write-Error "$_"
        $ErrorDetails= "$_"
        $ErrorOccured=$True
    }
    if($ErrorOccured){
        Start-Sleep 2
        throw "$ErrorDetails"
    }