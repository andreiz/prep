<?php

// [= and =] denote an array literal

$test = array();
$test[0] = 'x';

$simple = [= 'one' => 1, 'two' => 2, 'three' => 3 =];
var_dump($simple);

$complex = [=
	'fruit' => [= 'apple', 'orange', 'banana' =],
	'animals' => [= 'cow', 'pig', 'sheep' =],
=];
var_dump($complex);
