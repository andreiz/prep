#!/usr/bin/env php
<?php

$source = file_get_contents($argv[1]);

$tokens = preg_split('!^\s*(@\w+)\n\s*(function)\s+(\w+)\s*\(([^)]*)\)!m', $source, -1, PREG_SPLIT_DELIM_CAPTURE);

$max = count($tokens);
$decorated = array();
for ($i = 0; $i < $max; $i++) {
    $token = $tokens[$i];

    if ($token[0] == '@' && $i + 3 < $max) {
        $function_stmt = $tokens[$i+1];
        $function_name = $tokens[$i+2];
        $function_args = $tokens[$i+3];

        $function_impl = "orig_" . $function_name;
        $decorator = substr($token, 1);

        echo <<<EOD
function $function_name($function_args)
{
    return $decorator('$function_impl', func_get_args());
}

EOD;

        echo $function_stmt, " ";
        echo $function_impl, "(", $function_args, ")";
        $i += 3;
    } else {
        echo $token;
    }
}
