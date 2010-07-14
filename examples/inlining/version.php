#!/usr/bin/env php
<?php

$tokens = token_get_all(file_get_contents($argv[1]));

$max = count($tokens);
for ($i = 0; $i < $max; $i++) {

    /* Single-character token */
    if (!is_array($tokens[$i])) {
        echo $tokens[$i];
        continue;
    }

    /* Named tokens */
    if (T_STRING == $tokens[$i][0] &&
        $tokens[$i][1] == 'phpversion' &&
        $i+2 < $max && '(' == $tokens[$i+1] && ')' == $tokens[$i+2]) {
        echo '"'.phpversion().'"';
        $i += 2;
    } else {
        echo $tokens[$i][1];
    }
}
