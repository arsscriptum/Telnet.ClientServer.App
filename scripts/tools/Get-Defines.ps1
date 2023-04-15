$c = Get-Content "$PSScriptRoot\defines.h"
ForEach($line in $c){
    $i = $line.IndexOf('0x')
    $def = $line.substring(0, $i)
    $def = $def.Trim()
    $newdef = $def.Replace('SERVICE_','SCC_')
    
    $len=$newdef.Length 
    $diff = 35 - $len 
    0 .. $diff | %{
        $newdef += ' '
    }
    $newline = '#define {0} {1}' -f $newdef,$def
    $newline
}