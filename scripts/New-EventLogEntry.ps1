<#
#̷𝓍   𝓐𝓡𝓢 𝓢𝓒𝓡𝓘𝓟𝓣𝓤𝓜 
#̷𝓍 
#̷𝓍   <guillaumeplante.qc@gmail.com>
#̷𝓍   https://arsscriptum.github.io/  Http
#>



function New-EventLogEntry{
    [CmdletBinding(SupportsShouldProcess)]
    param(  
        [Parameter(Mandatory=$true, Position=0)]
        [String]$Message,
        [Parameter(Mandatory=$false)]
        [String]$Source="Development",
        [Parameter(Mandatory=$false)]
        [ValidateSet('Application','Security','System','Setup')]
        [String]$Category="Application",
        [Parameter(Mandatory=$false)]
        [String]$Type="Information",
        [Parameter(Mandatory=$false)]
        [int]$EventId=1999
    )

    try{
        if (-not ([System.Diagnostics.EventLog]::SourceExists($Source))) {
            [System.Diagnostics.EventLog]::CreateEventSource($Source, $Category)
        }
        [System.Diagnostics.EventLog]::WriteEntry($Source, $Message, $Type, $EventId)   
    }catch{
        Write-Error "$_"
    }
}




function New-EventLogEntryAsync{
    [CmdletBinding(SupportsShouldProcess)]
    param(  
        [Parameter(Mandatory=$true, Position=0)]
        [String]$Message,
        [Parameter(Mandatory=$false)]
        [int]$TimeoutSeconds=45,
        [Parameter(Mandatory=$false)]
        [switch]$Wait
    )

    $NewEventLogScript = {
          param($Message,$Source)
      
        try{
            [String]$Source="Development"
            [String]$Type="Information"
            [int]$EventId=1999
            [String]$Category="Application"

            $DateStr=(Get-Date).GetDateTimeFormats()[23]
            Write-Output "[START] $DateStr"
            if (-not ([System.Diagnostics.EventLog]::SourceExists($Source))) {
                [System.Diagnostics.EventLog]::CreateEventSource($Source, $Category)
            }
            [System.Diagnostics.EventLog]::WriteEntry($Source, $Message, $Type, $EventId)   
        }catch{
            Write-Error "$_"
        }finally{
            $DateStr=(Get-Date).GetDateTimeFormats()[23]
            Write-Output "[DONE] $DateStr"
    }}.GetNewClosure()

    [scriptblock]$NewEventLogScriptBlock = [scriptblock]::create($NewEventLogScript) 

    try{
        $Result = $False
        $TimeoutTime = (Get-Date).AddSeconds($TimeoutSeconds)
        $DateStr=(Get-Date).GetDateTimeFormats()[23]
        Write-Host "[New-EventLogEntryAsync] New Log Entry @ $DateStr"
        
        [string]$JobName="NewEventLogScriptBlock-" + (Get-Random -Maximum 9999).ToString("d4")
        $jobby = Start-Job -Name $JobName -ScriptBlock $NewEventLogScriptBlock -ArgumentList ($Message,$Source)
        if($Wait){
            Write-Host "[New-EventLogEntryAsync] Waiting for job completion..."
            while($True){
                $JobState = (Get-Job -Name $JobName).State
                $Output = Receive-Job -Name $JobName

                if($JobState -eq 'Completed'){
                    $DateStr=(Get-Date).GetDateTimeFormats()[23]
                    Write-Host "[New-EventLogEntryAsync] Completed @ $DateStr" -f DarkGreen
                    Get-Job $JobName -ea Ignore | Remove-Job -ea Ignore
                    $Result = $True
                    break;
                }
                [timespan]$ts = $TimeoutTime-[datetime]::Now
                if([math]::Sign($ts.TotalSeconds) -lt 0){
                    $DateStr=(Get-Date).GetDateTimeFormats()[23]
                    Write-Host "[New-EventLogEntryAsync] Timeout @ $DateStr" -f DarkYellow
                    Get-Job $JobName -ea Ignore | Remove-Job -ea Ignore
                    break;
                }
                Start-Sleep 1
            }
        } 
        return $Result 
    }catch{
        Write-Error "$_"
    }
}