#!/usr/bin/env php
<?php
define('ZERO', chr(9));
define('ONE', chr(32));
define('WIDTH', 8);
if (isset($argv[1])) {
	$source = $argv[1];
} else {
	$source = 'php://stdin';
}
$dat = str_replace("\n", '', file_get_contents($source));
$max = strlen($dat);
for ($i=0; $i<$max; $i+=WIDTH) {
	$chr = substr($dat, $i, WIDTH);
	echo chr(
		bindec(
			strtr(
				$chr,
				array(ZERO=>'0',ONE=>'1')
			)
		)
	);
}
