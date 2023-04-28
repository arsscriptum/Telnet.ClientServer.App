<#
#̷𝓍   𝓐𝓡𝓢 𝓢𝓒𝓡𝓘𝓟𝓣𝓤𝓜
#̷𝓍   🇵​​​​​🇴​​​​​🇼​​​​​🇪​​​​​🇷​​​​​🇸​​​​​🇭​​​​​🇪​​​​​🇱​​​​​🇱​​​​​ 🇸​​​​​🇨​​​​​🇷​​​​​🇮​​​​​🇵​​​​​🇹​​​​​ 🇧​​​​​🇾​​​​​ 🇬​​​​​🇺​​​​​🇮​​​​​🇱​​​​​🇱​​​​​🇦​​​​​🇺​​​​​🇲​​​​​🇪​​​​​🇵​​​​​🇱​​​​​🇦​​​​​🇳​​​​​🇹​​​​​🇪​​​​​.🇶​​​​​🇨​​​​​@🇬​​​​​🇲​​​​​🇦​​​​​🇮​​​​​🇱​​​​​.🇨​​​​​🇴​​​​​🇲​​​​​
#>


function Invoke-IsAdministrator  {  
    (New-Object Security.Principal.WindowsPrincipal ([Security.Principal.WindowsIdentity]::GetCurrent())).IsInRole([Security.Principal.WindowsBuiltinRole]::Administrator)  
}
function Get-ScriptDirectory {
    Split-Path -Parent $PSCommandPath
}

      try{

        [string]$ErrorDetails=''
        [System.Boolean]$ErrorOccured=$False

        ########################################################################################
        #
        #  Read Arguments 
        #
        ########################################################################################
        $ExpectedNumberArguments = 2
        $Script:Arguments = $args.Clone()
        [System.Collections.ArrayList]$ArgumentList = [System.Collections.ArrayList]::new()
        0..$ExpectedNumberArguments |  % {
            $index = $_
            if([string]::IsNullOrEmpty($Script:Arguments[$index]) -eq $True){
             Write-Output "missing argument $index"
            }
            [void]$ArgumentList.Add("$($Script:Arguments[$index])")
        }

        [Bool]$DebugMode                = ([string]::IsNullOrEmpty($Script:Arguments[3]) -eq $False)

        [string]$TargetName             = $ArgumentList.Item(0)
        [string]$SolutionDirectory      = $ArgumentList.Item(1)
        [string]$PlBinary               = $ArgumentList.Item(2)

        [string]$SolutionDirectory      = (Resolve-Path "$SolutionDirectory").Path
        [string]$PlBinary               = (Resolve-Path "$PlBinary").Path


        Write-Debug "########################################################################################"
        Write-Debug "                                    DEBUG ARGUMENTS                                     "
        Write-Debug "`tTargetName          ==> $TargetName"
        Write-Debug "`tPlBinary            ==> $PlBinary"
        Write-Debug "`tSolutionDirectory   ==> $SolutionDirectory"
        Write-Debug "########################################################################################"

        [string[]]$Out = &"$PlBinary" "$TargetName"
        Write-Debug "$Out"
        $ExecRunning = !($Out[0].Contains('Error'))
      
        
        $TaskkillExe=(get-command 'taskkill.exe').Source
        $TargetExe = "$TargetName" + '.exe'
        if($ExecRunning){
            Write-Output "====================================================="
            Write-Output "Killing running $TargetExe instance"
            Write-Output "====================================================="
            $Out = &"$TaskkillExe" '/IM' "$TargetExe" '/f'
        }else{
            Write-Debug "NO Instance Running for `"$TargetName`""
        }
    }catch{
        Write-Error "$_"
    }
