<#
#̷𝓍   𝓐𝓡𝓢 𝓢𝓒𝓡𝓘𝓟𝓣𝓤𝓜
#̷𝓍   🇵​​​​​🇴​​​​​🇼​​​​​🇪​​​​​🇷​​​​​🇸​​​​​🇭​​​​​🇪​​​​​🇱​​​​​🇱​​​​​ 🇸​​​​​🇨​​​​​🇷​​​​​🇮​​​​​🇵​​​​​🇹​​​​​ 🇧​​​​​🇾​​​​​ 🇬​​​​​🇺​​​​​🇮​​​​​🇱​​​​​🇱​​​​​🇦​​​​​🇺​​​​​🇲​​​​​🇪​​​​​🇵​​​​​🇱​​​​​🇦​​​​​🇳​​​​​🇹​​​​​🇪​​​​​.🇶​​​​​🇨​​​​​@🇬​​​​​🇲​​​​​🇦​​​​​🇮​​​​​🇱​​​​​.🇨​​​​​🇴​​​​​🇲​​​​​
#>

function Remove-ReconFirewallRules {
    [CmdletBinding(SupportsShouldProcess)]
    param(
        [parameter(mandatory=$false)]
        [switch]$Force
    )

    try{
        $ToBeDeleted = Get-NetFirewallRule | Where Name -match "recon_"
        $ToBeDeletedCount = $ToBeDeleted.Count 
        if($ToBeDeletedCount -gt 3){
            throw "Retrieved $ToBeDeletedCount Rules To Be Deleted, which ifgroupnas an INSAME amount. Do it manually, to avoid breaking."
        }

        $ToBeDeleted  | % {
            $RuleName  = $_.Name 
            if(Get-Confirmation "Delete Firewall Rule `"$RuleName`"" $Force) {
                Write-Output "DELETING Firewall Rule `"$RuleName`""
                $Out = $_ | Remove-NetFirewallRule | Out-Null
            }
        }
    }catch{
        throw $_
    }
}


function Update-FirewallConfiguration{
    [CmdletBinding(SupportsShouldProcess)]
    param(
        [parameter(mandatory=$true,Position=0)]
        [String]$Config
    )
    try{
        $ServerConfig = Get-IniContent "$ServerCfgIniFile"
        Write-BuildOutTitle "CONFIGURE NET FIREWALL RULES"
        Remove-ReconFirewallRules -Force
        0 .. 20 | % {
            $id = $_ 
            $SectionName = "FirewallRule_{0:d3}" -f $id 
            $FirewallRule = ($ServerConfig.$SectionName  -as 'Hashtable')
            if($FirewallRule -eq $Null) { break }
            $Rule = Get-NetFirewallRule -Name "$($FirewallRule.Name)" -ErrorAction Ignore
            if($Rule -ne $Null){
                Write-Debug "------------------------------------------------"
                Write-Debug "REMOVING FIREWALL RULE `"$($FirewallRule.Name)`""
                Get-NetFirewallRule -Name "$($FirewallRule.Name)" | Remove-NetFirewallRule
                Write-Debug "------------------------------------------------`n"
            }
            Write-Line 40 '-'
            Write-Output "ADDING FIREWALL RULE `"$($FirewallRule.Name)`""
            Write-Debug "New-NetFirewallRule -Name `"$($FirewallRule.Name)`" -DisplayName `"$($FirewallRule.DisplayName)`" -Enabled True -Direction `"$($FirewallRule.Direction)`" -Protocol `"$($FirewallRule.Protocol)`" -Action `"$($FirewallRule.Action)`" -LocalPort `"$($FirewallRule.LocalPort)`" -Description `"$($FirewallRule.Description)`""       
            Write-Line 40 '-'
            $Res = New-NetFirewallRule -Name "$($FirewallRule.Name)" -DisplayName "$($FirewallRule.DisplayName)" -Enabled True -Direction "$($FirewallRule.Direction)" -Protocol "$($FirewallRule.Protocol)" -Action "$($FirewallRule.Action)" -LocalPort "$($FirewallRule.LocalPort)" -Description "$($FirewallRule.Description)" | Out-Null
            Write-Debug "Rule $($Res.Name) : $($Res.Status)"
        }
    }catch{
        throw $_
    }
}

