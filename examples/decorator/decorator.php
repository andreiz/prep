#!/usr/bin/env php
<?php

$source = file_get_contents($argv[1]);

$tokens = preg_split('!^(@\w+)\n\s*(function)\s+(\w+)\s*\(([^)]*)\)!m', $source, -1, PREG_SPLIT_DELIM_CAPTURE);

$max = count($tokens);
$decorated = array();
for ($i = 0; $i < $max; $i++) {
    $token = $tokens[$i];

    if ($token[0] == '@' && $i + 3 < $max) {
        echo $tokens[$i+1], " ";
        $orig_name = "orig_" . $tokens[$i+2];
        echo $orig_name, "(", $tokens[$i+3], ")";
        $decorated[] = array(substr($token, 1), $tokens[$i+2], $orig_name, $tokens[$i+3]);
        $i += 3;
    } else {
        echo $token;
    }
}

$decor_tpl = <<<EOD
function %s(%s)
{
    return %s('%s', func_get_args());
}
EOD;
if ($decorated) {
    foreach ($decorated as $entry) {
        list($decorator, $func, $orig_func, $args) = $entry;

        $output = sprintf($decor_tpl, $func, $args, $decorator, $orig_func);

        print "\n";
        print $output;
        print "\n";
    }
}
