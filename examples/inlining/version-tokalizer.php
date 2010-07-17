#!/usr/bin/env php
<?php

require "/Users/andrei/code/tokalizer/include/TokenSet.class.php";

$ts = TokenSet::fromFile($argv[1]);
foreach ($ts as $token) {
	if ($token instanceof ProceduralFunctionCallToken &&
		$token->getValue() == "phpversion") {
		echo '"'.phpversion().'"';
		$ts->next();
		$ts->next();
	} else {
		echo $token->reconstruct();
	}
}
