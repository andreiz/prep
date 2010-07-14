#!/usr/bin/env php
<?php

$tokens = token_get_all(file_get_contents($argv[1]));

$max = count($tokens);
$var_tokens = array();
$last_token_incdec = false;
for ($i = 0; $i < $max; $i++) {

    /* Single-character token */
    if (!is_array($tokens[$i])) {
        if ($tokens[$i] == ';') {
            echo join('', $var_tokens);
            $var_tokens = array();
            echo $tokens[$i];
        } else if ($var_tokens) {
            /* save tokens until T_INC/T_DEC or ';' */
            $var_tokens[] = $tokens[$i];
        } else {
            $last_token_incdec = false;
            echo $tokens[$i];
        }
        continue;
    }

    /* Named tokens */
    if (T_VARIABLE == $tokens[$i][0]) {
        if ($last_token_incdec) {
            $last_token_incdec = false;
        } else {
            /* save tokens until T_INC/T_DEC or ';' */
            $var_tokens[] = $tokens[$i][1];
            continue;
        }
    } else if ((T_INC == $tokens[$i][0] || T_DEC == $tokens[$i][0])) {
        if ($var_tokens) {
            echo $tokens[$i][1];
            echo join('', $var_tokens);
            $var_tokens = array();
            continue;
        } else {
            $last_token_incdec = true;
        }
    } else {
        if ($var_tokens) {
            $var_tokens[] = $tokens[$i][1];
            continue;
        }
    }

    echo $tokens[$i][1];
}
