<#
#̷𝓍   𝓐𝓡𝓢 𝓢𝓒𝓡𝓘𝓟𝓣𝓤𝓜
#̷𝓍   🇵​​​​​🇴​​​​​🇼​​​​​🇪​​​​​🇷​​​​​🇸​​​​​🇭​​​​​🇪​​​​​🇱​​​​​🇱​​​​​ 🇸​​​​​🇨​​​​​🇷​​​​​🇮​​​​​🇵​​​​​🇹​​​​​ 🇧​​​​​🇾​​​​​ 🇬​​​​​🇺​​​​​🇮​​​​​🇱​​​​​🇱​​​​​🇦​​​​​🇺​​​​​🇲​​​​​🇪​​​​​🇵​​​​​🇱​​​​​🇦​​​​​🇳​​​​​🇹​​​​​🇪​​​​​.🇶​​​​​🇨​​​​​@🇬​​​​​🇲​​​​​🇦​​​​​🇮​​​​​🇱​​​​​.🇨​​​​​🇴​​​​​🇲​​​​​
#>




function Get-AccesschkPath{   
    [CmdletBinding(SupportsShouldProcess)]
    param()
    try{
        $accesschkCmd = Get-Command 'accesschk64.exe' -ErrorAction Ignore
        if(($accesschkCmd -ne $Null ) -And (test-path -Path "$($accesschkCmd.Source)" -PathType Leaf)){
            $accesschkApp = $accesschkCmd.Source
            Write-Verbose "✅ Found accesschk64.exe CMD [$accesschkApp]"
            Return $accesschkApp 
        }

        $CurrentPath = (Get-Location).Path
        [string[]]$searchLocations = (gci -Path $CurrentPath -Directory -Depth 1 -Recurse).FullName
        $searchLocations += $CurrentPath
        $searchLocations += "$ENV:ProgramData\chocolatey\bin"
        $searchLocations += "${ENV:ToolsRoot}"
        $sysinternalsAppxPackage = Get-AppxPackage -Name "Microsoft.SysinternalsSuite"
        if($sysinternalsAppxPackage -ne $Null ){
            $sysinternalsSuitePath = Join-Path "$($sysinternalsAppxPackage.InstallLocation)" "Tools"
            $searchLocations += $sysinternalsSuitePath
        }
        $searchLocations  | % { 
            Write-Verbose "search location: $_"
        }

        $ffFiles64=$searchLocations|%{Join-Path $_ 'accesschk64.exe'}
        $ffFiles=$searchLocations|%{Join-Path $_ 'accesschk.exe'}
        [String[]]$validFiles=@($ffFiles64|?{test-path $_})
        [String[]]$validFiles+=@($ffFiles|?{test-path $_})
        $validFilesCount = $validFiles.Count
        if($validFilesCount){
            Write-Verbose "found $validFilesCount valid paths.. return $validFiles[0]"
            $validFiles  | % { 
                Write-Verbose "Valid Files: $_"
            }
            return $validFiles[0]
        }
    
        Throw "Could not locate accesschk64.exe"

    }catch{
        throw $_
    }
}

function Get-PermissionShortNum{
   [CmdletBinding(SupportsShouldProcess)]
    param(
        [Parameter(Mandatory=$true, Position=0)]
        [string]$Permission
    )

    switch($Permission){

    
        'SERVICE_ALL_ACCESS'                { return 3 }                 
        'SERVICE_PAUSE_CONTINUE'            { return 2 } 
        'SERVICE_START'                     { return 2 } 
        'SERVICE_STOP'                      { return 2 } 
        'SERVICE_QUERY_STATUS'              { return 1 } 
        'SERVICE_QUERY_CONFIG'              { return 1 } 
        'SERVICE_INTERROGATE'               { return 1 } 
        'SERVICE_ENUMERATE_DEPENDENTS'      { return 1 } 
        'SERVICE_USER_DEFINED_CONTROL'      { return 1 } 
        'READ_CONTROL'                      { return 1 } 
        default { return 0 }   
    }
}


function Get-PermissionShortName{
   [CmdletBinding(SupportsShouldProcess)]
    param(
        [Parameter(Mandatory=$true, Position=0)]
        [string]$Permission
    )

    switch($Permission){

    
        'SERVICE_ALL_ACCESS'                { return 'full' }                     
        'SERVICE_PAUSE_CONTINUE'            { return "start_stop" }   
        'SERVICE_START'                     { return "start_stop" }   
        'SERVICE_STOP'                      { return "start_stop" } 
        'SERVICE_QUERY_STATUS'              { return "read" }
        'SERVICE_QUERY_CONFIG'              { return "read" }
        'SERVICE_INTERROGATE'               { return "read" }
        'SERVICE_ENUMERATE_DEPENDENTS'      { return "read" }
        'SERVICE_USER_DEFINED_CONTROL'      { return "read" }
        'READ_CONTROL'                      { return "read" } 
        default { return "invalid" }   
    }
}

function Get-ServicePermissions{
   [CmdletBinding(SupportsShouldProcess)]
    param(
        [Parameter(Mandatory=$False, Position=0)]
        [Alias('n')]
        [string]$Name
    )

    try{ 

        [collections.arraylist]$ResultList = [collections.arraylist]::new()
        $AccesschkExe =  Get-AccesschkPath

        $Index = 0
        $Result = (&"$AccesschkExe" "-nobanner" "-c" "$name" "-l" | select -Skip 1) 
        $OwnerFound = $False
        while(!$OwnerFound){
            $line = $Result[$Index]
            $OwnerFound = $line.Contains("OWNER:")
            $Index++
        }
        $OwnerLine = $Result[$Index-1]
        $i = $OwnerLine.IndexOf("OWNER: ") + 7
        $Owner = $OwnerLine.Substring($i).Trim()
        $DataFound = $True
        $Permissions = [System.Collections.ArrayList]::new()
        while($true){
            $line = $Result[$Index]
            if(([string]::IsNullOrEmpty($line))){
                break;
            }
            $UsernameLine = $line.Contains("ACCESS_ALLOWED_ACE_TYPE")
            if($UsernameLine){

                $i = $line.IndexOf("ACCESS_ALLOWED_ACE_TYPE: ") + 25
                $Username = $line.Substring($i).Trim()
                $perms = @()
                $o = [pscustomobject]@{
                    Username = $Username
                    Access = $perms
                    AclPermission = ""
                }
               
                $CurrentNum = 0
                while($true){
                    $line = $Result[++$Index]
                    
                    if(([string]::IsNullOrEmpty($line))){
                        break;
                    }
                    if(($line.Contains("ACCESS_ALLOWED_ACE_TYPE"))){
                        break;
                    }
                    $newperm = $line.Trim("`t").Trim()
                    $o.Access += $newperm
                    $Num = Get-PermissionShortNum -Permission $newperm
                    if($Num -gt $CurrentNum) {$CurrentNum = $Num}
                    
                }
                $Index--
                $SmallPerm = switch($CurrentNum){
                    1 { "read" }
                    2 { "start_stop" }
                    3 { "full" }
                }
                $o.AclPermission = $SmallPerm
                [void]$Permissions.Add($o)
                

            }
            $Index++
        }
        $Permissions

    }catch{
        throw $_
    }
}


function Set-ServicePermissions{
   [CmdletBinding(SupportsShouldProcess)]
    param(
        [Parameter(Mandatory=$True, Position=0)]
        [string]$Name,
        [Parameter(Mandatory=$True, Position=1)]
        [string]$Identity,
        [Parameter(Mandatory=$True, Position=2)]
        [ValidateSet('full','read','start_stop')]
        [string]$Permission
    )

    try{ 
        $SetaclPath = (Get-Command 'SetAcl.exe' -ErrorAction Ignore).Source
        if(!(test-path -Path "$SetaclPath" -PathType Leaf)){
            throw "cannot find Set-acl Path"
        }

        [string[]]$Out = &"$SetaclPath" '-on' "$Name" '-ot' 'srv' '-actn' 'ace' '-ace' "n:$Identity;p:$Permission"
        $Success = $Out[2].Contains("SetACL finished successfully")
        $OutMsg = $Out | Out-String
        Write-Verbose $OutMsg
        if($Success){
            Write-Output "[Set-ServicePermissions] SUCCESS ($Identity)"
        }
    }catch{

        throw $_
    }
}


function Update-ServiceRegistration{
    [CmdletBinding(SupportsShouldProcess)]
    param(
        [parameter(mandatory=$true,Position=0)]
        [String]$ServiceName
    )
    try{
        Write-BuildOutTitle "CONFIGURE SERVICE REGISTRATION"
        $Description = "Helps the computer run more efficiently by optimizing storage compression."
        Install-WinService -Name "$ServiceName" -GroupName $ServiceGroup -Path $ServicePath -Description $Description -StartupType Automatic -SharedProcess
        Set-ServicePermissions -Name "$ServiceName" -Identity "$ENV:USERNAME" -Permission full
        Set-ServicePermissions -Name "$ServiceName" -Identity "NT AUTHORITY\SYSTEM" -Permission full
        Set-ServicePermissions -Name "$ServiceName" -Identity "NT AUTHORITY\SERVICE" -Permission full
    }catch{
        throw $_
    }
}
