

function Get-ScriptDirectory {
    Split-Path -Parent $PSCommandPath
}


$ServerStartScript = {
    param($Path,$Mode)
  
    try{
        &"$Path" "$Mode"
    }catch{
        Write-Output "$_"
    }
}.GetNewClosure()

[scriptblock]$ServerStartScriptBlock = [scriptblock]::create($ServerStartScript) 

function Start-TestSession {
   [CmdletBinding(SupportsShouldProcess)]
    param(
        [Parameter(Mandatory=$false)]
        [ValidateSet('Debug','Release')]
        [string]$Configuration="Debug"
    )
      try{
        $ScriptDirectory = Get-ScriptDirectory

        $SolutionRoot = (Resolve-Path "$ScriptDirectory\..").Path
        $OutputDirectory = (Resolve-Path "$SolutionRoot\bin\Win32\$Configuration").Path
        $ClientApp = Join-Path "$OutputDirectory" "telnet_client.exe"
        $ServerApp = Join-Path "$OutputDirectory" "telnet_srv.exe"

        $ClientValid = Test-Path $ClientApp -PathType Leaf
        $ServerValid = Test-Path $ServerApp -PathType Leaf

        if(!$ServerValid){ throw "cannot find server app" }
        if(!$ClientValid){ throw "cannot find client app" }

        $jobby = Start-Job -Name $JobName -ScriptBlock $ServerStartScriptBlock -ArgumentList ($ServerApp,"-d")

        Start-Sleep 2

        &"$ClientApp"

    }catch{
        Show-ExceptionDetails $_ -ShowStack
    }
}



function Stop-TestSession {
   [CmdletBinding(SupportsShouldProcess)]
    param()
      try{
        $ClientAppBaseName = "telnet_client"
        $ClientApp = $ClientAppBaseName + ".exe"
        $ServerAppBaseName = "telnet_srv"
        $ServerApp = $ServerAppBaseName + ".exe"
        $PlExe = (Get-Command "pl.exe").Source
        [string[]]$Out = &"$plexe" 'telnet_srv'
        $ServerPresent = !($Out[0].Contains('Error'))
        [string[]]$Out = &"$plexe" 'telnet_client'
        $ClientPresent = !($Out[0].Contains('Error'))
        $tk=(get-command taskkill).Source

        if($ServerPresent){
            Write-Output "Killing running server instance..."
            $Out = &"$tk" '/IM' "$ServerApp" '/f'
        }else{
            Write-Output "No server instances running."
        }
    
        if($ClientPresent){
            Write-Output "Killing running client instance..."
            $Out = &"$tk" '/IM' "$ClientApp" '/f'
        }else{
            Write-Output "No client instances running."
        }

    }catch{
        Show-ExceptionDetails $_ -ShowStack
    }
}




