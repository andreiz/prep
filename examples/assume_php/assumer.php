#!/usr/bin/env php
<?php
// see http://blogs.sitepoint.com/2010/10/15/closing-php-tags-debate/
// I don't think this is actually a *good* idea, but it's still possible
// run with: php -d'extension=prep.so' -d'prep.command=/path/to/prep/examples/assume_php/assumer.php' input.php

$tokens = token_get_all(file_get_contents($argv[1]));
$replace = "<?php\n";
$max = count($tokens);
for ($i = 0; $i < $max; $i++) {
	if (!is_array($tokens[$i])) {
		$replace .= $tokens[$i];
		continue;
	} elseif (
		T_OPEN_TAG == $tokens[$i][0]
		&& '<?' == $tokens[$i][1]
		&& T_STRING == $tokens[$i+1][0]
		&& 'html' == strtolower($tokens[$i+1][1])
		&& T_WHITESPACE == $tokens[$i+2][0]
	) {
		// got "<?html "
		$i += 1;
		$replace .= "?>";
		while (++$i < $max) {
			if (!is_array($tokens[$i])) {
				$replace .= $tokens[$i];
				continue;
			} elseif (T_CLOSE_TAG == $tokens[$i][0]) {
				$replace .= "<?php";
				break;
			} else {
				$replace .= $tokens[$i][1];
			}
		}
	} else {
		$replace .= $tokens[$i][1];
	}
}
echo $replace;

