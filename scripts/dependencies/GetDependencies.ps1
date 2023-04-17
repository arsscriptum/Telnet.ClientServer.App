   [CmdletBinding(SupportsShouldProcess)]
    param(
        [Parameter(Mandatory=$true,Position=0)]
        [string]$Path
    )
    try{
       
        $FilesToInclude = (gci -Path "$Path" -File -Filter "*.ps1") | Where Name -notmatch "Dependencies" | Select -ExpandProperty Fullname

        $FilesToInclude
    }catch{
        Write-Error "$_"
        $ReturnCode = -1
    }
