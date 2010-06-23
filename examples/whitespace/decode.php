#!/usr/bin/env php
<?php
/**
 * Tricky script to turn what looks like whitespace into actual PHP.
 *
 * There's an example in hello.php (hello.phps is the decoded version).
 * This file is a decoder (that works as a prep script).
 * encode.php is the encoder.
 * Once you see the trick, it's simple, but still fun. (-:
 *
 * php -d'extension=prep.so' -d'prep.command=/path/to/prep/examples/whitespace/decode.php' hello.php
 */

define('ZERO', chr(9));
define('ONE', chr(32));
define('WIDTH', 8);
$dat = str_replace("\n", '', file_get_contents(isset($argv[1]) ? $argv[1] : 'php://stdin'));
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
