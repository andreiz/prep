<?php

function logger($func, $args)
{
	echo "entering $func()\n";
	$return = call_user_func_array($func, $args);
	echo "exiting $func()\n";
	return $return;
}

@logger
function square($a)
{
	print "calculating square($a)\n";
	return $a * $a;
}

@logger
function cube($a)
{
	print "calculating cube($a)\n";
	return $a * $a * $a;
}

print square(2)."\n";

print cube(3)."\n";
