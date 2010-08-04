#!/usr/bin/env php
<?php
$in = file_get_contents($argv[1]);

$tokens = token_get_all($in);

$out = '';
$its = count($tokens);
for ($i=0; $i<$its; $i++) {
	if ('[' == $tokens[$i] && '=' == $tokens[$i+1]) {
		$out .= 'array(';
		++$i;
	} elseif ('=' == $tokens[$i] && ']' == $tokens[$i+1]) {
		$out .= ')';
		++$i;
	} else {
		if (isset($tokens[$i][1])) {
			$out .= $tokens[$i][1];
		} else {
			$out .= $tokens[$i];
		}
	}
}

echo $out;
