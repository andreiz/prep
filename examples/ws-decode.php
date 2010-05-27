<?php
define('ZERO', chr(9));
define('ONE', chr(32));
define('WIDTH', 8);
$dat = str_replace("\n", '', file_get_contents('php://stdin'));
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
