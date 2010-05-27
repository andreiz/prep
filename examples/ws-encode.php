<?php
define('ZERO', chr(9));
define('ONE', chr(32));
define('WIDTH', 8);
$dat = file_get_contents('php://stdin');
$max = strlen($dat);
$buf = '';
for ($i=0; $i<$max; $i++) {
	$buf .= str_pad(
		strtr(
			decbin(
				ord(
					$dat[$i]
				)
			),
			array('0'=>ZERO, '1'=>ONE)
		),
		WIDTH,
		ZERO,
		STR_PAD_LEFT
	);
}
echo chunk_split($buf, 76, "\n");
