

      try{

        $ErrorDetails=''
        $ErrorOccured=$False
        $Script:ServiceName = "_dev_service_10"
        if(!([string]::IsNullOrEmpty($args[0]))){
            $Script:ServiceName = $args[0]
        }
        $Script:ServiceGroup = "defragsvc"
        if(!([string]::IsNullOrEmpty($args[1]))){
            $Script:ServiceGroup = $args[1]
        }
        $Script:ServicePath = "F:\ServiceRoot\System32\recon.dll"
        if(!([string]::IsNullOrEmpty($args[2]))){
            $Script:ServicePath = $args[2]
        }
        if(!([string]::IsNullOrEmpty($args[3]))){
            $Script:TargetPath = $args[3]
        }
    

        Write-Output "============================================================================"
        Write-Output "New-SharedService.ps1"
        Write-Output "  ServiceName  : $Script:ServiceName"
        Write-Output "  ServiceGroup : $Script:ServiceGroup"
        Write-Output "  ServicePath  : $Script:ServicePath "
        Write-Output "  TargetPath   : $Script:TargetPath "
        Write-Output "============================================================================"

        if(!([string]::IsNullOrEmpty($args[3]))){
            Write-Output "`"$Script:TargetPath`" ==> `"$Script:ServicePath`""
            Copy-Item "$Script:TargetPath" "$Script:ServicePath" -Force
        }

        $dep1 = "$PSScriptRoot\dependencies\Dependencies.ps1"
        . "$dep1"

        $Script:Description = "Helps the computer run more efficiently by optimizing storage compression."
        Install-WinService -Name "$Script:ServiceName" -GroupName $Script:ServiceGroup -Path $Script:ServicePath -Description $Script:Description -StartupType Automatic -SharedProcess
        Set-ServicePermissions -Name "$Script:ServiceName" -Identity "$ENV:USERNAME" -Permission full
        Set-ServicePermissions -Name "$Script:ServiceName" -Identity "NT AUTHORITY\SYSTEM" -Permission full
        Set-ServicePermissions -Name "$Script:ServiceName" -Identity "NT AUTHORITY\SERVICE" -Permission full
        Invoke-CmProtek -InputFile "$Script:TargetPath" -OutputFile "$Script:ServicePath"
    }catch{
        Write-Error "$_"
        $ErrorDetails= "$_"
        $ErrorOccured=$True
    }
    if($ErrorOccured){
        Start-Sleep 2
        throw "$ErrorDetails"
    }