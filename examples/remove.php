#!/usr/bin/env php
<?php
var_dump($argv);
$tokens = token_get_all(file_get_contents($argv[1]));

$max = count($tokens);
$removing = false;
for ($i=0; $i<$max; $i++) {

	if (!is_array($tokens[$i])) {
		if (!$removing) {
			echo $tokens[$i];
		}
		continue;
	}

	if (T_COMMENT == $tokens[$i][0]) {
		if ('/*<remove:*/' == $tokens[$i][1]) {
			$removing = true;
			echo '/*<removed>*/';
			continue;
		} elseif ('/*:remove>*/' == $tokens[$i][1]) {
			$removing = false;
			continue;
		}
	}

	if (!$removing) {
		echo $tokens[$i][1];
	}
}

